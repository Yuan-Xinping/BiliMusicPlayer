#include "PlaybackService.h"
#include "AudioPlayer.h"
#include "PlaylistManager.h"
#include "PlaybackHistory.h"
#include "PlaybackQueue.h"
#include "../data/SongRepository.h"
#include "../common/AppConfig.h"

#include <QTimer>
#include <QDebug>
#include <QHash>
#include <QFileInfo>
#include <QtGlobal>

namespace {
    inline PlaybackMode modeFromInt(int v) {
        switch (v) {
        case 1: return PlaybackMode::Shuffle;
        case 2: return PlaybackMode::RepeatOne;
        case 3: return PlaybackMode::RepeatAll;
        case 0:
        default: return PlaybackMode::Normal;
        }
    }
    inline int intFromMode(PlaybackMode m) {
        switch (m) {
        case PlaybackMode::Shuffle:   return 1;
        case PlaybackMode::RepeatOne: return 2;
        case PlaybackMode::RepeatAll: return 3;
        case PlaybackMode::Normal:
        default: return 0;
        }
    }
}

class PlaybackService::Impl : public QObject {
    Q_OBJECT

public:
    explicit Impl(PlaybackService* parent)
        : QObject(parent)
        , q(parent)
        , audioPlayer(new AudioPlayer(this))
        , playlistManager(new PlaylistManager(this))
        , playbackHistory(new PlaybackHistory(this))
        , playbackQueue(new PlaybackQueue(this))
        , songRepository(new SongRepository(this))
        , positionTimer(new QTimer(this))
        , currentState(PlaybackState::Stopped)
        , loopA(-1)
        , loopB(-1)
        , saveDebounce(new QTimer(this))
        , restoringSession(false)
        , pendingSave(false)
        , lastSavedPositionMs(-1)
        , lastUserVolume(70)
    {
        setupConnections();
        setupTimer();
        setupSmartFeatures();

        // 启动时恢复音量与播放模式
        AppConfig& cfg = AppConfig::instance();
        audioPlayer->setVolume(cfg.getPlayerVolume());
        playlistManager->setPlaybackMode(modeFromInt(cfg.getPlayerPlaybackMode()));
        lastUserVolume = cfg.getPlayerVolume();

        // 防抖保存定时器
        saveDebounce->setInterval(700);
        saveDebounce->setSingleShot(true);
        connect(saveDebounce, &QTimer::timeout, this, [this]() {
            if (!pendingSave || restoringSession) return;
            pendingSave = false;
            AppConfig::instance().save();
            });

        // 会话恢复改为事件循环启动后执行，降低构造期时序风险
        if (cfg.getResumeOnStartup()) {
            QTimer::singleShot(0, this, &Impl::restoreSession);
        }
    }

    // 合并保存（防抖）
    void scheduleConfigSave() {
        if (restoringSession) return;
        pendingSave = true;
        if (!saveDebounce->isActive()) saveDebounce->start();
    }

    // 播放前校验本地文件是否存在；若静音则恢复到上次用户音量
    bool safeStartSong(const Song& song) {
        const QString path = song.getLocalFilePath();
        if (path.isEmpty() || !QFileInfo::exists(path)) {
            emit q->error(QString("音频文件不存在，已跳过：%1").arg(song.getTitle()));
            qWarning() << "PlaybackService: 文件不存在，跳过:" << path;
            return false;
        }
        if (audioPlayer->volume() == 0 && lastUserVolume > 0) {
            qDebug() << "[Service] safeStartSong: restore vol to" << lastUserVolume;
            audioPlayer->setVolume(lastUserVolume);
        }
        qDebug() << "[Service] safeStartSong: play path=" << path;
        audioPlayer->play(path);
        return true;
    }

private slots:
    void handlePlaybackStateChanged(PlaybackState state) {
        if (currentState != state) {
            currentState = state;
            emit q->playbackStateChanged(state);

            if (state == PlaybackState::Playing) positionTimer->start();
            else positionTimer->stop();

            qDebug() << "[Service] stateChanged ->" << (int)state
                << "vol=" << audioPlayer->volume()
                << "posMs=" << audioPlayer->position();
        }
    }

    void handleCurrentIndexChanged(int index) {
        emit q->currentSongIndexChanged(index);

        Song cur = playlistManager->getCurrentSong();
        if (!cur.getId().isEmpty()) {
            emit q->currentSongChanged(cur);
            if (!restoringSession) {
                AppConfig& cfg = AppConfig::instance();
                cfg.setLastSongId(cur.getId());
                scheduleConfigSave();
            }
            qDebug() << "PlaybackService: 当前歌曲变化:" << cur.getTitle();
        }
    }

    void handlePositionChanged(qint64 position) {
        emit q->positionChanged(position);

        if (restoringSession) return;

        // 至少每5秒记录一次位置，减少写盘
        if (lastSavedPositionMs < 0 || qAbs(position - lastSavedPositionMs) >= 5000) {
            qDebug() << "[Service] positionTick 5s"
                << "state=" << (int)currentState
                << "vol=" << audioPlayer->volume()
                << "posMs=" << position;

            lastSavedPositionMs = position;
            AppConfig& cfg = AppConfig::instance();
            cfg.setLastPositionMs(position);

            // 同步当前歌曲ID，保障恢复场景一致
            Song cur = playlistManager->getCurrentSong();
            if (!cur.getId().isEmpty()) cfg.setLastSongId(cur.getId());

            scheduleConfigSave();
        }
    }

    void handleDurationChanged(qint64 duration) {
        emit q->durationChanged(duration);
    }

    void handleVolumeChanged(int volume) {
        emit q->volumeChanged(volume);
        if (volume > 0) lastUserVolume = volume;
        qDebug() << "[Service] volumeChanged ->" << volume
            << "(restoring=" << restoringSession << ")"
            << "state=" << (int)currentState;

        if (restoringSession) return;
        AppConfig& cfg = AppConfig::instance();
        cfg.setPlayerVolume(volume);
        scheduleConfigSave();
    }

    void handlePlaybackModeChanged(PlaybackMode mode) {
        emit q->playbackModeChanged(mode);
        if (restoringSession) return;

        AppConfig& cfg = AppConfig::instance();
        cfg.setPlayerPlaybackMode(intFromMode(mode));
        scheduleConfigSave();
    }

    void handlePlaylistChanged(const QList<Song>& playlist) {
        emit q->playlistChanged(playlist);
        if (restoringSession) return;

        QStringList plIds; for (const Song& s : playlist) plIds << s.getId();
        AppConfig& cfg = AppConfig::instance();
        cfg.setLastPlaylistIds(plIds);
        scheduleConfigSave();
    }

    void handleAudioError(const QString& error) {
        emit q->error(error);
        qWarning() << "PlaybackService: 音频播放错误:" << error;
        // 自动跳过损坏/不存在的文件
        q->playNext();
    }

    void handleSongFinished() {
        qDebug() << "PlaybackService: 歌曲播放完成";

        qint64 duration = audioPlayer->duration();
        playbackHistory->updateCurrentRecord(duration, true);

        // 1) 播放队列优先
        const QList<Song> queue = playbackQueue ? playbackQueue->getQueue() : QList<Song>{};
        if (!queue.isEmpty()) {
            Song head = queue.first();
            playbackQueue->dequeue();
            q->playSong(head);
            return;
        }

        // 2) 根据播放模式
        PlaybackMode mode = playlistManager->getPlaybackMode();

        if (mode == PlaybackMode::RepeatOne) {
            Song cur = playlistManager->getCurrentSong();
            if (!cur.getId().isEmpty()) {
                q->playSong(cur);
                return;
            }
        }

        if (mode == PlaybackMode::Shuffle) {
            Song s = playlistManager->getSmartNextSong();
            if (!s.getId().isEmpty()) {
                q->playSong(s);
                return;
            }
        }

        // Normal / RepeatAll：顺序推进
        const QList<Song> pl = playlistManager->getPlaylist();
        const int size = pl.size();
        if (size <= 0) {
            qDebug() << "PlaybackService: 播放列表为空";
            return;
        }
        const int idx = playlistManager->getCurrentIndex();
        int nextIndex = idx + 1;

        if (nextIndex >= size) {
            if (mode == PlaybackMode::RepeatAll) {
                nextIndex = 0;
            }
            else {
                qDebug() << "PlaybackService: 顺序播放到末尾，停止";
                q->stop();
                return;
            }
        }

        q->playSong(pl[nextIndex]);
    }

    void handleHistoryRecordAdded(const PlaybackRecord& record) {
        emit q->playbackRecordAdded(record);
        qDebug() << "PlaybackService: 播放记录已添加:" << record.song.getTitle();
    }

    void handleQueueChanged(const QList<Song>& queue) {
        emit q->playbackQueueChanged(queue);
        if (restoringSession) return;

        QStringList qIds; for (const Song& s : queue) qIds << s.getId();
        AppConfig& cfg = AppConfig::instance();
        cfg.setLastQueueIds(qIds);
        scheduleConfigSave();
    }

    void handleSmartPlaylistGenerated(const QList<Song>& playlist) {
        emit q->smartPlaylistGenerated(playlist);
    }

    void updatePosition() {
        if (currentState == PlaybackState::Playing) {
            qint64 position = audioPlayer->position();

            // A-B 循环：超过 B 时回跳 A
            if (loopA >= 0 && loopB > loopA && position >= loopB) {
                audioPlayer->setPosition(loopA);
                return;
            }

            emit q->positionChanged(position);
        }
    }

private:
    void setupConnections() {
        // AudioPlayer 信号连接
        connect(audioPlayer, &AudioPlayer::stateChanged,
            this, &Impl::handlePlaybackStateChanged);
        connect(audioPlayer, &AudioPlayer::positionChanged,
            this, &Impl::handlePositionChanged);
        connect(audioPlayer, &AudioPlayer::durationChanged,
            this, &Impl::handleDurationChanged);
        connect(audioPlayer, &AudioPlayer::volumeChanged,
            this, &Impl::handleVolumeChanged);
        connect(audioPlayer, &AudioPlayer::error,
            this, &Impl::handleAudioError);
        connect(audioPlayer, &AudioPlayer::finished,
            this, &Impl::handleSongFinished);

        // PlaylistManager 信号连接
        connect(playlistManager, &PlaylistManager::playlistChanged,
            this, &Impl::handlePlaylistChanged);
        connect(playlistManager, &PlaylistManager::currentIndexChanged,
            this, &Impl::handleCurrentIndexChanged);
        connect(playlistManager, &PlaylistManager::playbackModeChanged,
            this, &Impl::handlePlaybackModeChanged);
        connect(playlistManager, &PlaylistManager::smartPlaylistGenerated,
            this, &Impl::handleSmartPlaylistGenerated);

        // PlaybackHistory 信号连接
        connect(playbackHistory, &PlaybackHistory::recordAdded,
            this, &Impl::handleHistoryRecordAdded);
        connect(playbackHistory, &PlaybackHistory::historyChanged,
            q, &PlaybackService::playbackHistoryChanged);

        // PlaybackQueue 信号连接
        connect(playbackQueue, &PlaybackQueue::queueChanged,
            this, &Impl::handleQueueChanged);
    }

    void setupTimer() {
        positionTimer->setInterval(100);
        connect(positionTimer, &QTimer::timeout,
            this, &Impl::updatePosition);
    }

    void setupSmartFeatures() {
        playlistManager->setPlaybackQueue(playbackQueue);
        playlistManager->setPlaybackHistory(playbackHistory);

        qDebug() << "PlaybackService: 智能播放功能已启用";
    }

    // 会话恢复：从 AppConfig 恢复 列表/队列/当前歌曲/进度
    void restoreSession() {
        AppConfig& cfg = AppConfig::instance();
        restoringSession = true;

        // 建立 id->Song 快速映射
        QList<Song> allSongs = songRepository->findAll();
        QHash<QString, Song> byId;
        byId.reserve(allSongs.size());
        for (const Song& s : allSongs) {
            if (!s.getId().isEmpty()) byId.insert(s.getId(), s);
        }

        // 恢复播放列表
        QList<Song> restoredPlaylist;
        const QStringList plIds = cfg.getLastPlaylistIds();
        for (const QString& id : plIds) {
            auto it = byId.constFind(id);
            if (it != byId.constEnd()) restoredPlaylist << it.value();
        }

        if (!restoredPlaylist.isEmpty()) {
            playlistManager->setPlaylist(restoredPlaylist);

            // 恢复队列
            const QStringList qIds = cfg.getLastQueueIds();
            for (const QString& id : qIds) {
                auto it = byId.constFind(id);
                if (it != byId.constEnd()) playbackQueue->enqueue(it.value());
            }

            // 当前歌曲索引
            int idx = 0;
            const QString lastId = cfg.getLastSongId();
            if (!lastId.isEmpty()) {
                for (int i = 0; i < restoredPlaylist.size(); ++i) {
                    if (restoredPlaylist[i].getId() == lastId) { idx = i; break; }
                }
            }
            playlistManager->setCurrentIndex(idx);

            // 自动播放并恢复进度
            const Song cur = restoredPlaylist[idx];
            if (safeStartSong(cur)) {
                const qint64 resumePos = cfg.getLastPositionMs();
                if (resumePos > 0) {
                    QTimer::singleShot(200, audioPlayer, [this, resumePos]() {
                        audioPlayer->setPosition(resumePos);
                        });
                }
            }
            else {
                // 若当前歌无效，尝试下一首
                q->playNext();
            }
        }

        restoringSession = false;
    }

public:
    PlaybackService* q;
    AudioPlayer* audioPlayer;
    PlaylistManager* playlistManager;
    PlaybackHistory* playbackHistory;
    PlaybackQueue* playbackQueue;
    SongRepository* songRepository;
    QTimer* positionTimer;

    PlaybackState currentState;

    // A-B 循环点
    qint64 loopA;
    qint64 loopB;

    // 配置持久化合并
    QTimer* saveDebounce;
    bool restoringSession;
    bool pendingSave;
    qint64 lastSavedPositionMs;
    int lastUserVolume;
};

// PlaybackService 主类实现
PlaybackService::PlaybackService(QObject* parent)
    : QObject(parent)
    , d(new Impl(this))
{
}

PlaybackService& PlaybackService::instance() {
    static PlaybackService instance;
    return instance;
}

// 基础播放控制
void PlaybackService::playSong(const Song& song) {
    if (song.getId().isEmpty()) {
        qWarning() << "PlaybackService::playSong: 歌曲 ID 为空";
        return;
    }

    qDebug() << "PlaybackService: 播放歌曲:" << song.getTitle();

    d->playbackHistory->addRecord(song);

    int songIndex = d->playlistManager->findSongIndex(song);
    if (songIndex == -1) {
        d->playlistManager->addSong(song);
        songIndex = d->playlistManager->getPlaylistSize() - 1;
    }

    d->playlistManager->setCurrentIndex(songIndex);

    // 播放前校验文件，静音兜底
    if (!d->safeStartSong(song)) {
        // 跳到下一首（将触发自动错误跳过路径）
        playNext();
    }
}

void PlaybackService::playPlaylist(const QList<Song>& playlist, int startIndex) {
    if (playlist.isEmpty()) {
        qWarning() << "PlaybackService::playPlaylist: 播放列表为空";
        return;
    }

    if (startIndex < 0 || startIndex >= playlist.size()) {
        startIndex = 0;
    }

    qDebug() << "PlaybackService: 播放播放列表，起始歌曲:"
        << playlist[startIndex].getTitle();

    d->playlistManager->setPlaylist(playlist);
    d->playlistManager->setCurrentIndex(startIndex);

    playSong(playlist[startIndex]);
}

void PlaybackService::togglePlayPause() {
    qDebug() << "[Service] togglePlayPause() enter."
        << "state=" << (int)d->currentState
        << "vol=" << d->audioPlayer->volume()
        << "posMs=" << d->audioPlayer->position();

    switch (d->currentState) {
    case PlaybackState::Playing:
        qDebug() << "[Service] request: pause()";
        d->audioPlayer->pause();
        break;
    case PlaybackState::Paused:
        if (d->audioPlayer->volume() == 0 && d->lastUserVolume > 0) {
            qDebug() << "[Service] paused->resume with vol restore to" << d->lastUserVolume;
            d->audioPlayer->setVolume(d->lastUserVolume);
        }
        else {
            qDebug() << "[Service] paused->resume keep vol=" << d->audioPlayer->volume();
        }
        d->audioPlayer->resume();
        break;
    case PlaybackState::Stopped:
        if (d->playlistManager->hasCurrentSong()) {
            qDebug() << "[Service] stopped->play currentSong";
            playSong(d->playlistManager->getCurrentSong());
        }
        else {
            qDebug() << "[Service] stopped AND no currentSong";
        }
        break;
    }

    QTimer::singleShot(80, d, [this]() {
        qDebug() << "[Service] togglePlayPause() post."
            << "state=" << (int)d->currentState
            << "vol=" << d->audioPlayer->volume()
            << "posMs=" << d->audioPlayer->position();
        });
}

void PlaybackService::stop() {
    d->audioPlayer->stop();
}

void PlaybackService::playNext() {
    // 1) 播放队列优先
    const QList<Song> queue = d->playbackQueue->getQueue();
    if (!queue.isEmpty()) {
        Song head = queue.first();
        d->playbackQueue->dequeue();
        playSong(head);
        return;
    }

    // 2) 模式分支
    PlaybackMode mode = d->playlistManager->getPlaybackMode();

    if (mode == PlaybackMode::Shuffle) {
        Song s = d->playlistManager->getSmartNextSong();
        if (!s.getId().isEmpty()) playSong(s);
        return;
    }

    // 手动“下一首”：RepeatOne 也按顺序推进
    const QList<Song> pl = d->playlistManager->getPlaylist();
    const int size = pl.size();
    if (size <= 0) return;

    int idx = d->playlistManager->getCurrentIndex();
    int nextIndex = idx + 1;

    if (nextIndex >= size) {
        // 手动下一首：Normal/RepeatAll 都从头开始
        nextIndex = 0;
    }

    playSong(pl[nextIndex]);
}

void PlaybackService::playPrevious() {
    // 2 秒规则：>2s 回到本曲开头
    if (d->audioPlayer->position() > 2000) {
        d->audioPlayer->setPosition(0);
        return;
    }

    PlaybackMode mode = d->playlistManager->getPlaybackMode();

    if (mode == PlaybackMode::Shuffle) {
        Song s = d->playlistManager->getSmartPreviousSong();
        if (!s.getId().isEmpty()) playSong(s);
        return;
    }

    const QList<Song> pl = d->playlistManager->getPlaylist();
    const int size = pl.size();
    if (size <= 0) return;

    int idx = d->playlistManager->getCurrentIndex();
    int prevIndex = idx - 1;

    if (prevIndex < 0) {
        // 手动上一首：Normal/RepeatAll 跳到最后
        prevIndex = size - 1;
    }

    playSong(pl[prevIndex]);
}

void PlaybackService::seek(qint64 position) {
    d->audioPlayer->setPosition(position);
}

// 状态查询
PlaybackState PlaybackService::getPlaybackState() const {
    return d->currentState;
}

bool PlaybackService::isPlaying() const {
    return d->currentState == PlaybackState::Playing;
}

bool PlaybackService::isPaused() const {
    return d->currentState == PlaybackState::Paused;
}

Song PlaybackService::getCurrentSong() const {
    return d->playlistManager->getCurrentSong();
}

qint64 PlaybackService::getCurrentPosition() const {
    return d->audioPlayer->position();
}

qint64 PlaybackService::getDuration() const {
    return d->audioPlayer->duration();
}

PlaybackMode PlaybackService::getPlaybackMode() const {
    return d->playlistManager->getPlaybackMode();
}

void PlaybackService::setPlaybackMode(PlaybackMode mode) {
    d->playlistManager->setPlaybackMode(mode);
    // 持久化（防抖）
    AppConfig& cfg = AppConfig::instance();
    cfg.setPlayerPlaybackMode(intFromMode(mode));
    d->scheduleConfigSave();
}

int PlaybackService::getVolume() const {
    return d->audioPlayer->volume();
}

void PlaybackService::setVolume(int volume) {
    d->audioPlayer->setVolume(volume);
    if (volume > 0) d->lastUserVolume = volume;
    // 持久化（防抖）
    AppConfig& cfg = AppConfig::instance();
    cfg.setPlayerVolume(volume);
    d->scheduleConfigSave();
}

QList<Song> PlaybackService::getCurrentPlaylist() const {
    return d->playlistManager->getPlaylist();
}

int PlaybackService::getCurrentSongIndex() const {
    return d->playlistManager->getCurrentIndex();
}

void PlaybackService::addToQueue(const Song& song) {
    d->playbackQueue->enqueue(song);
}

void PlaybackService::addNextToQueue(const Song& song) {
    d->playbackQueue->enqueueNext(song);
}

QList<Song> PlaybackService::getPlaybackQueue() const {
    return d->playbackQueue->getQueue();
}

void PlaybackService::clearPlaybackQueue() {
    d->playbackQueue->clear();
}

void PlaybackService::removeFromQueue(int index) {
    d->playbackQueue->removeSong(index);
}

void PlaybackService::moveInQueue(int from, int to) {
    d->playbackQueue->moveSong(from, to);
}

QList<PlaybackRecord> PlaybackService::getPlaybackHistory(int count) const {
    return d->playbackHistory->getRecentRecords(count);
}

Song PlaybackService::getMostPlayedSong() const {
    return d->playbackHistory->getMostPlayedSong();
}

QList<Song> PlaybackService::getFrequentlyPlayedSongs(int count) const {
    return d->playbackHistory->getFrequentlyPlayedSongs(count);
}

qint64 PlaybackService::getTotalListeningTime() const {
    return d->playbackHistory->getTotalListeningTime();
}

int PlaybackService::getPlayCount(const Song& song) const {
    return d->playbackHistory->getTotalPlayCount(song);
}

qint64 PlaybackService::getTotalPlayDuration(const Song& song) const {
    return d->playbackHistory->getTotalPlayDuration(song);
}

void PlaybackService::clearPlaybackHistory() {
    d->playbackHistory->clearHistory();
    emit playbackHistoryChanged();
}

QList<Song> PlaybackService::generateSmartPlaylist(int maxSongs) const {
    return d->playlistManager->generateSmartPlaylist(maxSongs);
}

void PlaybackService::createSmartPlaylist(int maxSongs) {
    QList<Song> smartPlaylist = d->playlistManager->createAndNotifySmartPlaylist(maxSongs);
    if (!smartPlaylist.isEmpty()) {
        playPlaylist(smartPlaylist);
    }
}

void PlaybackService::resetShuffleHistory() {
    d->playlistManager->resetShuffleHistory();
}

void PlaybackService::playSmartNext() {
    Song nextSong = d->playlistManager->getSmartNextSong();
    if (!nextSong.getId().isEmpty()) {
        playSong(nextSong);
    }
}

void PlaybackService::playSmartPrevious() {
    Song prevSong = d->playlistManager->getSmartPreviousSong();
    if (!prevSong.getId().isEmpty()) {
        playSong(prevSong);
    }
}

// A-B 循环
void PlaybackService::setLoopA(qint64 ms) {
    d->loopA = ms < 0 ? -1 : ms;
    emit loopABChanged(d->loopA, d->loopB);
}
void PlaybackService::setLoopB(qint64 ms) {
    d->loopB = ms < 0 ? -1 : ms;
    emit loopABChanged(d->loopA, d->loopB);
}
void PlaybackService::clearLoopAB() {
    d->loopA = -1;
    d->loopB = -1;
    emit loopABChanged(d->loopA, d->loopB);
}

#include "PlaybackService.moc"
