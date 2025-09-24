#include "SongRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

SongRepository::SongRepository(QObject* parent) : QObject(parent) {
}

bool SongRepository::save(const Song& song) {
    QSqlQuery query;
    query.prepare(R"(
        INSERT OR REPLACE INTO songs (
            id, title, artist, bilibili_url, local_file_path, 
            cover_url, duration_seconds, download_date, is_favorite
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(song.getId());
    query.addBindValue(song.getTitle());
    query.addBindValue(song.getArtist());
    query.addBindValue(song.getBilibiliUrl());
    query.addBindValue(song.getLocalFilePath());
    query.addBindValue(song.getCoverUrl());
    query.addBindValue(static_cast<qlonglong>(song.getDurationSeconds())); // 修复：强制转换为 qlonglong
    query.addBindValue(song.getDownloadDate().toString(Qt::ISODate));
    query.addBindValue(song.isFavorite() ? 1 : 0);

    if (!query.exec()) {
        qWarning() << "保存歌曲失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "歌曲保存成功:" << song.getTitle();
    return true;
}

bool SongRepository::update(const Song& song) {
    return save(song); // INSERT OR REPLACE 已经处理了更新
}

bool SongRepository::deleteById(const QString& id) {
    QSqlQuery query;
    query.prepare("DELETE FROM songs WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "删除歌曲失败:" << query.lastError().text();
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    qDebug() << "删除了" << rowsAffected << "首歌曲, ID:" << id;
    return rowsAffected > 0;
}

QList<Song> SongRepository::findAll() {
    QList<Song> songs;
    QSqlQuery query("SELECT * FROM songs ORDER BY download_date DESC");

    while (query.next()) {
        songs.append(songFromQuery(query));
    }

    qDebug() << "查询到" << songs.size() << "首歌曲";
    return songs;
}

Song SongRepository::findById(const QString& id) {
    QSqlQuery query;
    query.prepare("SELECT * FROM songs WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        return songFromQuery(query);
    }

    return Song(); // 返回空对象
}

QList<Song> SongRepository::findByTitle(const QString& title) {
    QList<Song> songs;
    QSqlQuery query;
    query.prepare("SELECT * FROM songs WHERE title LIKE ? ORDER BY title");
    query.addBindValue("%" + title + "%");

    if (query.exec()) {
        while (query.next()) {
            songs.append(songFromQuery(query));
        }
    }

    return songs;
}

QList<Song> SongRepository::findFavorites() {
    QList<Song> songs;
    QSqlQuery query("SELECT * FROM songs WHERE is_favorite = 1 ORDER BY title");

    while (query.next()) {
        songs.append(songFromQuery(query));
    }

    qDebug() << "查询到" << songs.size() << "首收藏歌曲";
    return songs;
}

int SongRepository::count() {
    QSqlQuery query("SELECT COUNT(*) FROM songs");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool SongRepository::exists(const QString& id) {
    QSqlQuery query;
    query.prepare("SELECT 1 FROM songs WHERE id = ? LIMIT 1");
    query.addBindValue(id);

    return query.exec() && query.next();
}

Song SongRepository::songFromQuery(const QSqlQuery& query) {
    Song song;
    song.setId(query.value("id").toString());
    song.setTitle(query.value("title").toString());
    song.setArtist(query.value("artist").toString());
    song.setBilibiliUrl(query.value("bilibili_url").toString());
    song.setLocalFilePath(query.value("local_file_path").toString());
    song.setCoverUrl(query.value("cover_url").toString());
    song.setDurationSeconds(query.value("duration_seconds").toLongLong());
    song.setDownloadDate(QDateTime::fromString(query.value("download_date").toString(), Qt::ISODate));
    song.setFavorite(query.value("is_favorite").toInt() == 1);

    return song;
}
