#pragma once
#include <QObject>
#include <QList>
#include "../common/entities/Song.h"
#include "../common/PlaybackMode.h"
#include "../common/PlaybackState.h"

struct PlaybackRecord;

class PlaybackService : public QObject {
    Q_OBJECT

public:
    explicit PlaybackService(QObject* parent = nullptr);
    static PlaybackService& instance();

    // 基础播放控制
    void playSong(const Song& song);
    void playPlaylist(const QList<Song>& playlist, int startIndex = 0);
    void togglePlayPause();
    void stop();
    void playNext();
    void playPrevious();

    // 进度和位置控制
    void seek(qint64 position);  // 跳转到指定位置 (毫秒)

    // 状态查询
    PlaybackState getPlaybackState() const;
    bool isPlaying() const;
    bool isPaused() const;
    Song getCurrentSong() const;
    qint64 getCurrentPosition() const;  // 当前播放位置 (毫秒)
    qint64 getDuration() const;         // 总时长 (毫秒)

    // 播放模式和音量
    PlaybackMode getPlaybackMode() const;
    void setPlaybackMode(PlaybackMode mode);
    int getVolume() const;              // 0-100
    void setVolume(int volume);

    // 播放列表管理
    QList<Song> getCurrentPlaylist() const;
    int getCurrentSongIndex() const;

    void addToQueue(const Song& song);
    void addNextToQueue(const Song& song);
    QList<Song> getPlaybackQueue() const;
    void clearPlaybackQueue();
    void removeFromQueue(int index);
    void moveInQueue(int from, int to);

    QList<PlaybackRecord> getPlaybackHistory(int count = 50) const;
    Song getMostPlayedSong() const;
    QList<Song> getFrequentlyPlayedSongs(int count = 10) const;
    qint64 getTotalListeningTime() const;
    int getPlayCount(const Song& song) const;
    qint64 getTotalPlayDuration(const Song& song) const;
    void clearPlaybackHistory();

    QList<Song> generateSmartPlaylist(int maxSongs = 20) const;
    void createSmartPlaylist(int maxSongs = 20);  // 生成并应用智能播放列表
    void resetShuffleHistory();

    void playSmartNext();      // 使用智能逻辑播放下一首
    void playSmartPrevious();  // 使用智能逻辑播放上一首

    // A-B 循环
    void setLoopA(qint64 ms);
    void setLoopB(qint64 ms);
    void clearLoopAB();

signals:
    // 原有状态变化信号
    void playbackStateChanged(PlaybackState state);
    void currentSongChanged(const Song& song);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void volumeChanged(int volume);
    void playbackModeChanged(PlaybackMode mode);
    void playlistChanged(const QList<Song>& playlist);
    void currentSongIndexChanged(int index);

    // 错误信号
    void error(const QString& errorMessage);

    // 扩展信号
    void playbackQueueChanged(const QList<Song>& queue);
    void smartPlaylistGenerated(const QList<Song>& playlist);
    void playbackHistoryChanged();
    void playbackRecordAdded(const PlaybackRecord& record);

    // A-B 循环点变化
    void loopABChanged(qint64 a, qint64 b);

private:
    class Impl;
    Impl* d;
};
