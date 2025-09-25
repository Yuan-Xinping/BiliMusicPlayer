// service/PlaylistManager.cpp
#include "PlaylistManager.h"
#include "PlaybackQueue.h"
#include "PlaybackHistory.h"
#include <QDebug>

PlaylistManager::PlaylistManager(QObject* parent)
    : QObject(parent)
    , m_currentIndex(-1)
    , m_playbackMode(PlaybackMode::Normal)
    , m_randomGenerator(QRandomGenerator::securelySeeded())
    , m_playbackQueue(nullptr)
    , m_playbackHistory(nullptr)
{
    qDebug() << "PlaylistManager initialized with smart features";
}

// 🆕 智能播放功能
void PlaylistManager::setPlaybackQueue(PlaybackQueue* queue) {
    m_playbackQueue = queue;
    qDebug() << "PlaylistManager: 设置播放队列";
}

void PlaylistManager::setPlaybackHistory(PlaybackHistory* history) {
    m_playbackHistory = history;
    qDebug() << "PlaylistManager: 设置播放历史";
}

Song PlaylistManager::getSmartNextSong() const {
    // 1. 优先检查播放队列
    if (m_playbackQueue && !m_playbackQueue->isEmpty()) {
        Song queueSong = m_playbackQueue->peek();
        qDebug() << "PlaylistManager: 从播放队列获取下一首:" << queueSong.getTitle();
        return queueSong;
    }

    // 2. 使用常规的播放列表逻辑
    int nextIndex = getNextIndex();
    if (nextIndex >= 0 && nextIndex < m_playlist.size()) {
        return m_playlist[nextIndex];
    }

    // 3. 如果常规列表结束，尝试生成智能播放列表
    if (m_playbackMode == PlaybackMode::Normal && m_playbackHistory) {
        QList<Song> smartList = generateSmartPlaylist(10);
        if (!smartList.isEmpty()) {
            qDebug() << "PlaylistManager: 生成智能播放列表，返回第一首";
            return smartList.first();
        }
    }

    return Song();
}

Song PlaylistManager::getSmartPreviousSong() const {
    // 智能上一首主要基于历史记录
    if (m_playbackHistory) {
        QList<PlaybackRecord> recentRecords = m_playbackHistory->getRecentRecords(10);
        if (recentRecords.size() >= 2) {
            // 返回倒数第二个播放记录（倒数第一个是当前歌曲）
            Song prevSong = recentRecords[recentRecords.size() - 2].song;
            qDebug() << "PlaylistManager: 从历史记录获取上一首:" << prevSong.getTitle();
            return prevSong;
        }
    }

    // 回退到常规上一首逻辑
    int prevIndex = getPreviousIndex();
    if (prevIndex >= 0 && prevIndex < m_playlist.size()) {
        return m_playlist[prevIndex];
    }

    return Song();
}

void PlaylistManager::resetShuffleHistory() {
    m_recentShuffleIndices.clear();
    qDebug() << "PlaylistManager: 重置随机播放历史";
}

int PlaylistManager::getSmartShuffleIndex() const {
    if (m_playlist.size() <= MAX_SHUFFLE_MEMORY) {
        // 如果播放列表很短，使用普通随机
        return getRandomIndex(m_currentIndex);
    }

    // 智能随机：避开最近播放的歌曲
    QSet<int> availableIndices;
    for (int i = 0; i < m_playlist.size(); ++i) {
        if (i != m_currentIndex && !m_recentShuffleIndices.contains(i)) {
            availableIndices.insert(i);
        }
    }

    if (availableIndices.isEmpty()) {
        // 如果没有可用的索引，清空历史记录重新开始
        m_recentShuffleIndices.clear();
        return getRandomIndex(m_currentIndex);
    }

    // 从可用索引中随机选择
    QList<int> availableList = availableIndices.values();
    int randomChoice = m_randomGenerator.bounded(availableList.size());
    int selectedIndex = availableList[randomChoice];

    // 记录这次选择
    m_recentShuffleIndices.insert(selectedIndex);

    // 限制历史记录大小
    while (m_recentShuffleIndices.size() > MAX_SHUFFLE_MEMORY) {
        // 移除一个随机的旧记录
        auto it = m_recentShuffleIndices.begin();
        m_recentShuffleIndices.erase(it);
    }

    qDebug() << "PlaylistManager: 智能随机选择索引:" << selectedIndex
        << "历史大小:" << m_recentShuffleIndices.size();

    return selectedIndex;
}

// 🔧 修复：const 版本 - 不发射信号
QList<Song> PlaylistManager::generateSmartPlaylist(int maxSongs) const {
    QList<Song> smartPlaylist;

    if (!m_playbackHistory) {
        qDebug() << "PlaylistManager: 没有播放历史，无法生成智能播放列表";
        return smartPlaylist;
    }

    // 1. 获取经常播放的歌曲
    QList<Song> frequentSongs = m_playbackHistory->getFrequentlyPlayedSongs(maxSongs / 2);
    for (const Song& song : frequentSongs) {
        if (!smartPlaylist.contains(song)) {
            smartPlaylist.append(song);
        }
    }

    // 2. 从当前播放列表中随机添加一些歌曲
    if (smartPlaylist.size() < maxSongs && !m_playlist.isEmpty()) {
        int remainingSlots = maxSongs - smartPlaylist.size();
        QSet<int> usedIndices;

        for (int i = 0; i < remainingSlots && usedIndices.size() < m_playlist.size(); ++i) {
            int randomIndex;
            do {
                randomIndex = m_randomGenerator.bounded(m_playlist.size());
            } while (usedIndices.contains(randomIndex));

            usedIndices.insert(randomIndex);
            Song randomSong = m_playlist[randomIndex];

            if (!smartPlaylist.contains(randomSong)) {
                smartPlaylist.append(randomSong);
            }
        }
    }

    qDebug() << "PlaylistManager: 生成智能播放列表，包含" << smartPlaylist.size() << "首歌曲";

    return smartPlaylist;
}

// 🆕 新增：非 const 版本 - 发射信号
QList<Song> PlaylistManager::createAndNotifySmartPlaylist(int maxSongs) {
    QList<Song> smartPlaylist = generateSmartPlaylist(maxSongs);  // 调用 const 版本

    if (!smartPlaylist.isEmpty()) {
        emit smartPlaylistGenerated(smartPlaylist);  // ✅ 在非 const 函数中发射信号
        qDebug() << "PlaylistManager: 智能播放列表已生成并通知";
    }

    return smartPlaylist;
}

// 现有方法保持不变
void PlaylistManager::setPlaylist(const QList<Song>& playlist) {
    m_playlist = playlist;
    m_currentIndex = playlist.isEmpty() ? -1 : 0;

    emit playlistChanged(m_playlist);
    emit currentIndexChanged(m_currentIndex);

    qDebug() << "PlaylistManager: 设置播放列表，歌曲数量:" << playlist.size();
}

void PlaylistManager::addSong(const Song& song) {
    m_playlist.append(song);

    if (m_playlist.size() == 1 && m_currentIndex == -1) {
        m_currentIndex = 0;
        emit currentIndexChanged(m_currentIndex);
    }

    emit playlistChanged(m_playlist);
    qDebug() << "PlaylistManager: 添加歌曲:" << song.getTitle();
}

void PlaylistManager::removeSong(int index) {
    if (index < 0 || index >= m_playlist.size()) {
        qWarning() << "PlaylistManager: 无效的歌曲索引:" << index;
        return;
    }

    QString title = m_playlist[index].getTitle();
    m_playlist.removeAt(index);

    if (m_currentIndex == index) {
        if (m_playlist.isEmpty()) {
            m_currentIndex = -1;
        }
        else if (m_currentIndex >= m_playlist.size()) {
            m_currentIndex = m_playlist.size() - 1;
        }
        emit currentIndexChanged(m_currentIndex);
    }
    else if (m_currentIndex > index) {
        m_currentIndex--;
        emit currentIndexChanged(m_currentIndex);
    }

    emit playlistChanged(m_playlist);
    qDebug() << "PlaylistManager: 移除歌曲:" << title;
}

void PlaylistManager::clearPlaylist() {
    m_playlist.clear();
    m_currentIndex = -1;

    emit playlistChanged(m_playlist);
    emit currentIndexChanged(m_currentIndex);

    qDebug() << "PlaylistManager: 清空播放列表";
}

QList<Song> PlaylistManager::getPlaylist() const {
    return m_playlist;
}

int PlaylistManager::getPlaylistSize() const {
    return m_playlist.size();
}

bool PlaylistManager::isEmpty() const {
    return m_playlist.isEmpty();
}

void PlaylistManager::setCurrentIndex(int index) {
    if (index < -1 || index >= m_playlist.size()) {
        qWarning() << "PlaylistManager: 无效的当前索引:" << index;
        return;
    }

    if (m_currentIndex != index) {
        m_currentIndex = index;
        emit currentIndexChanged(m_currentIndex);
        qDebug() << "PlaylistManager: 设置当前歌曲索引:" << index;
    }
}

int PlaylistManager::getCurrentIndex() const {
    return m_currentIndex;
}

Song PlaylistManager::getCurrentSong() const {
    if (hasCurrentSong()) {
        return m_playlist[m_currentIndex];
    }
    return Song();
}

bool PlaylistManager::hasCurrentSong() const {
    return m_currentIndex >= 0 && m_currentIndex < m_playlist.size();
}

void PlaylistManager::setPlaybackMode(PlaybackMode mode) {
    if (m_playbackMode != mode) {
        m_playbackMode = mode;
        emit playbackModeChanged(mode);
        qDebug() << "PlaylistManager: 设置播放模式:" << (int)mode;
    }
}

PlaybackMode PlaylistManager::getPlaybackMode() const {
    return m_playbackMode;
}

// 🔄 更新：使用智能随机
int PlaylistManager::getNextIndex() const {
    if (isEmpty() || m_currentIndex < 0) {
        return -1;
    }

    switch (m_playbackMode) {
    case PlaybackMode::RepeatOne:
        return m_currentIndex;

    case PlaybackMode::RepeatAll:
        return (m_currentIndex + 1) % m_playlist.size();

    case PlaybackMode::Shuffle:
        return getSmartShuffleIndex();  // ✅ 使用智能随机

    case PlaybackMode::Normal:
    default:
        return (m_currentIndex + 1 < m_playlist.size()) ?
            (m_currentIndex + 1) : -1;
    }
}

int PlaylistManager::getPreviousIndex() const {
    if (isEmpty() || m_currentIndex < 0) {
        return -1;
    }

    switch (m_playbackMode) {
    case PlaybackMode::RepeatOne:
        return m_currentIndex;

    case PlaybackMode::RepeatAll:
        return (m_currentIndex - 1 + m_playlist.size()) % m_playlist.size();

    case PlaybackMode::Shuffle:
        return getRandomIndex(m_currentIndex);

    case PlaybackMode::Normal:
    default:
        return (m_currentIndex > 0) ? (m_currentIndex - 1) : -1;
    }
}

Song PlaylistManager::getNextSong() const {
    int nextIndex = getNextIndex();
    if (nextIndex >= 0 && nextIndex < m_playlist.size()) {
        return m_playlist[nextIndex];
    }
    return Song();
}

Song PlaylistManager::getPreviousSong() const {
    int prevIndex = getPreviousIndex();
    if (prevIndex >= 0 && prevIndex < m_playlist.size()) {
        return m_playlist[prevIndex];
    }
    return Song();
}

int PlaylistManager::findSongIndex(const Song& song) const {
    for (int i = 0; i < m_playlist.size(); ++i) {
        if (m_playlist[i] == song) {
            return i;
        }
    }
    return -1;
}

bool PlaylistManager::containsSong(const Song& song) const {
    return findSongIndex(song) >= 0;
}

int PlaylistManager::getRandomIndex(int excludeIndex) const {
    if (m_playlist.size() <= 1) {
        return m_currentIndex;
    }

    int randomIndex;
    do {
        randomIndex = m_randomGenerator.bounded(m_playlist.size());
    } while (randomIndex == excludeIndex && m_playlist.size() > 1);

    return randomIndex;
}
