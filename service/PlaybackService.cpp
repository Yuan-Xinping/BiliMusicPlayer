// service/PlaybackService.cpp
#include "PlaybackService.h"
#include "AudioPlayer.h"
#include "PlaylistManager.h"
#include "PlaybackHistory.h"
#include "PlaybackQueue.h"
#include "../data/SongRepository.h"
#include <QTimer>
#include <QDebug>

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
        , wasPlaying(false)
    {
        setupConnections();
        setupTimer();
        setupSmartFeatures();
        qDebug() << "PlaybackService::Impl initialized with smart features";
    }

private slots:
    void handlePlaybackStateChanged(PlaybackState state) {
        if (currentState != state) {
            currentState = state;
            emit q->playbackStateChanged(state);

            if (state == PlaybackState::Playing) {
                positionTimer->start();
            }
            else {
                positionTimer->stop();
            }

            qDebug() << "PlaybackService: 播放状态变化:" << (int)state;
        }
    }

    void handleCurrentSongChanged(const Song& song) {
        emit q->currentSongChanged(song);
        qDebug() << "PlaybackService: 当前歌曲变化:" << song.getTitle();
    }

    void handlePositionChanged(qint64 position) {
        emit q->positionChanged(position);
    }

    void handleDurationChanged(qint64 duration) {
        emit q->durationChanged(duration);
    }

    void handleVolumeChanged(int volume) {
        emit q->volumeChanged(volume);
    }

    void handlePlaybackModeChanged(PlaybackMode mode) {
        emit q->playbackModeChanged(mode);
    }

    void handlePlaylistChanged(const QList<Song>& playlist) {
        emit q->playlistChanged(playlist);
    }

    void handleCurrentIndexChanged(int index) {
        emit q->currentSongIndexChanged(index);
    }

    void handleAudioError(const QString& error) {
        emit q->error(error);
        qWarning() << "PlaybackService: 音频播放错误:" << error;
    }

    void handleSongFinished() {
        qDebug() << "PlaybackService: 歌曲播放完成";

        qint64 duration = audioPlayer->duration();
        playbackHistory->updateCurrentRecord(duration, true);

        Song nextSong = playlistManager->getSmartNextSong();

        if (!nextSong.getId().isEmpty()) {
            if (playbackQueue && playbackQueue->contains(nextSong)) {
                playbackQueue->dequeue();
            }

            q->playSong(nextSong);
        }
        else {
            qDebug() << "PlaybackService: 没有更多歌曲可播放";
        }
    }

    void handleHistoryRecordAdded(const PlaybackRecord& record) {
        emit q->playbackRecordAdded(record);
        qDebug() << "PlaybackService: 播放记录已添加:" << record.song.getTitle();
    }

    void handleQueueChanged(const QList<Song>& queue) {
        emit q->playbackQueueChanged(queue);
    }

    void handleSmartPlaylistGenerated(const QList<Song>& playlist) {
        emit q->smartPlaylistGenerated(playlist);
    }

    void updatePosition() {
        if (currentState == PlaybackState::Playing) {
            qint64 position = audioPlayer->position();
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

public:
    PlaybackService* q;
    AudioPlayer* audioPlayer;
    PlaylistManager* playlistManager;
    PlaybackHistory* playbackHistory;
    PlaybackQueue* playbackQueue;
    SongRepository* songRepository;
    QTimer* positionTimer;

    PlaybackState currentState;
    bool wasPlaying;
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
    d->audioPlayer->play(song.getLocalFilePath());
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
    switch (d->currentState) {
    case PlaybackState::Playing:
        d->audioPlayer->pause();
        break;
    case PlaybackState::Paused:
        d->audioPlayer->resume();
        break;
    case PlaybackState::Stopped:
        if (d->playlistManager->hasCurrentSong()) {
            playSong(d->playlistManager->getCurrentSong());
        }
        break;
    }
}

void PlaybackService::stop() {
    d->audioPlayer->stop();
}

void PlaybackService::playNext() {
    Song nextSong = d->playlistManager->getSmartNextSong();
    if (!nextSong.getId().isEmpty()) {
        if (d->playbackQueue->contains(nextSong)) {
            d->playbackQueue->dequeue();
        }

        playSong(nextSong);
    }
}

void PlaybackService::playPrevious() {
    Song prevSong = d->playlistManager->getSmartPreviousSong();
    if (!prevSong.getId().isEmpty()) {
        playSong(prevSong);
    }
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
}

int PlaybackService::getVolume() const {
    return d->audioPlayer->volume();
}

void PlaybackService::setVolume(int volume) {
    d->audioPlayer->setVolume(volume);
}

QList<Song> PlaybackService::getCurrentPlaylist() const {
    return d->playlistManager->getPlaylist();
}

int PlaybackService::getCurrentSongIndex() const {
    return d->playlistManager->getCurrentIndex();
}

// Phase 4.2: 播放队列管理
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

// Phase 4.2: 播放历史管理
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

// Phase 4.2: 智能播放功能
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

#include "PlaybackService.moc"
