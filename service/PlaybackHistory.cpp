// service/PlaybackHistory.cpp
#include "PlaybackHistory.h"
#include <QDebug>
#include <algorithm>
#include <unordered_map>

PlaybackHistory::PlaybackHistory(QObject* parent)
    : QObject(parent)
    , m_currentRecord(nullptr)
{
    qDebug() << "PlaybackHistory initialized";
}

void PlaybackHistory::addRecord(const Song& song) {
    if (song.getId().isEmpty()) {
        qWarning() << "PlaybackHistory: 尝试添加空歌曲记录";
        return;
    }

    // 完成当前记录（如果有）
    if (m_currentRecord) {
        m_currentRecord->completed = false; // 新歌曲开始，之前的未完成
    }

    // 创建新记录
    PlaybackRecord newRecord(song, QDateTime::currentDateTime());
    m_records.append(newRecord);
    m_currentRecord = &m_records.last();

    // 限制历史记录大小
    limitHistorySize();

    emit recordAdded(newRecord);
    emit historyChanged();

    qDebug() << "PlaybackHistory: 添加播放记录:" << song.getTitle();
}

void PlaybackHistory::updateCurrentRecord(qint64 duration, bool completed) {
    if (!m_currentRecord) {
        return;
    }

    m_currentRecord->playDuration = duration;
    m_currentRecord->completed = completed;

    if (completed) {
        qDebug() << "PlaybackHistory: 歌曲播放完成:" << m_currentRecord->song.getTitle()
            << "时长:" << duration << "ms";
    }
}

void PlaybackHistory::clearHistory() {
    m_records.clear();
    m_currentRecord = nullptr;
    emit historyChanged();
    qDebug() << "PlaybackHistory: 清空播放历史";
}

QList<PlaybackRecord> PlaybackHistory::getRecentRecords(int count) const {
    int startIndex = qMax(0, m_records.size() - count);
    return m_records.mid(startIndex);
}

QList<PlaybackRecord> PlaybackHistory::getRecordsForSong(const Song& song) const {
    QList<PlaybackRecord> songRecords;
    for (const auto& record : m_records) {
        if (record.song == song) {
            songRecords.append(record);
        }
    }
    return songRecords;
}

int PlaybackHistory::getTotalPlayCount(const Song& song) const {
    return getRecordsForSong(song).size();
}

qint64 PlaybackHistory::getTotalPlayDuration(const Song& song) const {
    qint64 totalDuration = 0;
    for (const auto& record : getRecordsForSong(song)) {
        totalDuration += record.playDuration;
    }
    return totalDuration;
}

Song PlaybackHistory::getMostPlayedSong() const {
    if (m_records.isEmpty()) {
        return Song();
    }

    std::unordered_map<QString, int> playCount;
    for (const auto& record : m_records) {
        playCount[record.song.getId()]++;
    }

    QString mostPlayedId;
    int maxCount = 0;
    for (const auto& pair : playCount) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            mostPlayedId = pair.first;
        }
    }

    // 找到对应的歌曲对象
    for (const auto& record : m_records) {
        if (record.song.getId() == mostPlayedId) {
            return record.song;
        }
    }

    return Song();
}

QList<Song> PlaybackHistory::getFrequentlyPlayedSongs(int count) const {
    std::unordered_map<QString, int> playCount;
    std::unordered_map<QString, Song> songMap;

    // 统计播放次数
    for (const auto& record : m_records) {
        const QString& id = record.song.getId();
        playCount[id]++;
        songMap[id] = record.song;
    }

    // 转换为向量并排序
    std::vector<std::pair<QString, int>> sortedSongs(playCount.begin(), playCount.end());
    std::sort(sortedSongs.begin(), sortedSongs.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // 转换为 QList<Song>
    QList<Song> result;
    int resultCount = qMin(count, static_cast<int>(sortedSongs.size()));
    for (int i = 0; i < resultCount; ++i) {
        result.append(songMap[sortedSongs[i].first]);
    }

    return result;
}

qint64 PlaybackHistory::getTotalListeningTime() const {
    qint64 totalTime = 0;
    for (const auto& record : m_records) {
        totalTime += record.playDuration;
    }
    return totalTime;
}

void PlaybackHistory::limitHistorySize() {
    while (m_records.size() > MAX_HISTORY_SIZE) {
        m_records.removeFirst();
    }

    // 更新当前记录指针
    if (!m_records.isEmpty()) {
        m_currentRecord = &m_records.last();
    }
    else {
        m_currentRecord = nullptr;
    }
}
