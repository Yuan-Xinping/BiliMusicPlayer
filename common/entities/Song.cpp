#include "Song.h"

Song::Song(const QString& id, const QString& title, const QString& artist,
    const QString& bilibiliUrl, const QString& localFilePath,
    const QString& coverUrl, qlonglong durationSeconds,
    const QDateTime& downloadDate, bool isFavorite)
    : m_id(id), m_title(title), m_artist(artist)
    , m_bilibiliUrl(bilibiliUrl), m_localFilePath(localFilePath)
    , m_coverUrl(coverUrl), m_durationSeconds(durationSeconds)
    , m_downloadDate(downloadDate), m_isFavorite(isFavorite)
{
}

QString Song::toString() const {
    if (!m_artist.isEmpty()) {
        return m_title + " - " + m_artist;
    }
    return m_title;
}

bool Song::operator==(const Song& other) const {
    return m_id == other.m_id;
}
