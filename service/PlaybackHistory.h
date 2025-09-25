// service/PlaybackHistory.h
#pragma once
#include <QObject>
#include <QList>
#include <QDateTime>
#include "../common/entities/Song.h"

struct PlaybackRecord {
    Song song;
    QDateTime playTime;
    qint64 playDuration;  // 实际播放时长（毫秒）
    bool completed;       // 是否播放完成

    PlaybackRecord() : playDuration(0), completed(false) {}
    PlaybackRecord(const Song& s, const QDateTime& time)
        : song(s), playTime(time), playDuration(0), completed(false) {
    }
};

class PlaybackHistory : public QObject {
    Q_OBJECT

public:
    explicit PlaybackHistory(QObject* parent = nullptr);

    // 历史记录管理
    void addRecord(const Song& song);
    void updateCurrentRecord(qint64 duration, bool completed);
    void clearHistory();

    // 查询功能
    QList<PlaybackRecord> getRecentRecords(int count = 50) const;
    QList<PlaybackRecord> getRecordsForSong(const Song& song) const;
    int getTotalPlayCount(const Song& song) const;
    qint64 getTotalPlayDuration(const Song& song) const;

    // 统计功能
    Song getMostPlayedSong() const;
    QList<Song> getFrequentlyPlayedSongs(int count = 10) const;
    qint64 getTotalListeningTime() const;

signals:
    void recordAdded(const PlaybackRecord& record);
    void historyChanged();

private:
    void limitHistorySize();
    static const int MAX_HISTORY_SIZE = 1000;

    QList<PlaybackRecord> m_records;
    PlaybackRecord* m_currentRecord;
};
