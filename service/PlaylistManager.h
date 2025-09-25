// service/PlaylistManager.h
#pragma once
#include <QObject>
#include <QList>
#include <QRandomGenerator>
#include <QSet>
#include "../common/entities/Song.h"
#include "../common/PlaybackMode.h"

class PlaybackQueue;
class PlaybackHistory;

class PlaylistManager : public QObject {
    Q_OBJECT

public:
    explicit PlaylistManager(QObject* parent = nullptr);

    // 播放列表管理
    void setPlaylist(const QList<Song>& playlist);
    void addSong(const Song& song);
    void removeSong(int index);
    void clearPlaylist();

    QList<Song> getPlaylist() const;
    int getPlaylistSize() const;
    bool isEmpty() const;

    // 当前歌曲管理
    void setCurrentIndex(int index);
    int getCurrentIndex() const;
    Song getCurrentSong() const;
    bool hasCurrentSong() const;

    // 播放模式
    void setPlaybackMode(PlaybackMode mode);
    PlaybackMode getPlaybackMode() const;

    // 导航功能
    int getNextIndex() const;
    int getPreviousIndex() const;
    Song getNextSong() const;
    Song getPreviousSong() const;

    // 查找功能
    int findSongIndex(const Song& song) const;
    bool containsSong(const Song& song) const;

    // 🆕 智能播放功能
    void setPlaybackQueue(PlaybackQueue* queue);
    void setPlaybackHistory(PlaybackHistory* history);

    // 智能下一首（考虑队列和历史）
    Song getSmartNextSong() const;
    Song getSmartPreviousSong() const;

    // 随机播放优化（避免重复）
    void resetShuffleHistory();

    // 🔧 智能播放列表生成 - 两个版本
    QList<Song> generateSmartPlaylist(int maxSongs = 20) const;           // 静默生成
    QList<Song> createAndNotifySmartPlaylist(int maxSongs = 20);          // 生成并发射信号

signals:
    void playlistChanged(const QList<Song>& playlist);
    void currentIndexChanged(int index);
    void playbackModeChanged(PlaybackMode mode);
    void smartPlaylistGenerated(const QList<Song>& playlist);

private:
    int getRandomIndex(int excludeIndex = -1) const;
    int getSmartShuffleIndex() const;  // 智能随机，避免最近播放的歌曲

    QList<Song> m_playlist;
    int m_currentIndex;
    PlaybackMode m_playbackMode;
    mutable QRandomGenerator m_randomGenerator;

    // 🆕 智能播放成员
    PlaybackQueue* m_playbackQueue;
    PlaybackHistory* m_playbackHistory;
    mutable QSet<int> m_recentShuffleIndices;  // 最近随机播放过的索引
    static const int MAX_SHUFFLE_MEMORY = 5;   // 记住最近5首随机歌曲
};
