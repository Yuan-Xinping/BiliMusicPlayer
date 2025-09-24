#pragma once
#include <QString>
#include <QDateTime>

class Song {
public:
    Song() = default;
    Song(const QString& id, const QString& title, const QString& artist,
        const QString& bilibiliUrl, const QString& localFilePath,
        const QString& coverUrl, qlonglong durationSeconds,
        const QDateTime& downloadDate, bool isFavorite = false);

    // Getters
    QString getId() const { return m_id; }
    QString getTitle() const { return m_title; }
    QString getArtist() const { return m_artist; }
    QString getBilibiliUrl() const { return m_bilibiliUrl; }
    QString getLocalFilePath() const { return m_localFilePath; }
    QString getCoverUrl() const { return m_coverUrl; }
    qlonglong getDurationSeconds() const { return m_durationSeconds; }
    QDateTime getDownloadDate() const { return m_downloadDate; }
    bool isFavorite() const { return m_isFavorite; }

    // Setters
    void setId(const QString& id) { m_id = id; }
    void setTitle(const QString& title) { m_title = title; }
    void setArtist(const QString& artist) { m_artist = artist; }
    void setBilibiliUrl(const QString& url) { m_bilibiliUrl = url; }
    void setLocalFilePath(const QString& path) { m_localFilePath = path; }
    void setCoverUrl(const QString& url) { m_coverUrl = url; }
    void setDurationSeconds(qlonglong duration) { m_durationSeconds = duration; }
    void setDownloadDate(const QDateTime& date) { m_downloadDate = date; }
    void setFavorite(bool favorite) { m_isFavorite = favorite; }

    QString toString() const;
    bool operator==(const Song& other) const;

private:
    QString m_id;
    QString m_title;
    QString m_artist;
    QString m_bilibiliUrl;
    QString m_localFilePath;
    QString m_coverUrl;
    qlonglong m_durationSeconds = 0;
    QDateTime m_downloadDate;
    bool m_isFavorite = false;
};
