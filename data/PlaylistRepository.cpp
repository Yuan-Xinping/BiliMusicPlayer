#include "PlaylistRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

PlaylistRepository::PlaylistRepository(QObject* parent) : QObject(parent) {
}

bool PlaylistRepository::save(const Playlist& playlist) {
    QSqlQuery query;
    query.prepare(R"(
        INSERT OR REPLACE INTO playlists (id, name, description) 
        VALUES (?, ?, ?)
    )");

    query.addBindValue(playlist.getId());
    query.addBindValue(playlist.getName());
    query.addBindValue(playlist.getDescription());

    if (!query.exec()) {
        qWarning() << "保存播放列表失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "播放列表保存成功:" << playlist.getName();
    return true;
}

bool PlaylistRepository::update(const Playlist& playlist) {
    return save(playlist); // INSERT OR REPLACE 处理更新
}

bool PlaylistRepository::deleteById(const QString& id) {
    QSqlQuery query;
    query.prepare("DELETE FROM playlists WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "删除播放列表失败:" << query.lastError().text();
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    qDebug() << "删除了播放列表, ID:" << id;
    return rowsAffected > 0;
}

QList<Playlist> PlaylistRepository::findAll() {
    QList<Playlist> playlists;
    QSqlQuery query("SELECT * FROM playlists ORDER BY name");

    while (query.next()) {
        playlists.append(playlistFromQuery(query));
    }

    qDebug() << "查询到" << playlists.size() << "个播放列表";
    return playlists;
}

Playlist PlaylistRepository::findById(const QString& id) {
    QSqlQuery query;
    query.prepare("SELECT * FROM playlists WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        return playlistFromQuery(query);
    }

    return Playlist(); // 返回空对象
}

Playlist PlaylistRepository::findByName(const QString& name) {
    QSqlQuery query;
    query.prepare("SELECT * FROM playlists WHERE name = ?");
    query.addBindValue(name);

    if (query.exec() && query.next()) {
        return playlistFromQuery(query);
    }

    return Playlist();
}

bool PlaylistRepository::addSongToPlaylist(const QString& playlistId, const QString& songId) {
    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO playlist_songs (playlist_id, song_id) VALUES (?, ?)");
    query.addBindValue(playlistId);
    query.addBindValue(songId);

    if (!query.exec()) {
        qWarning() << "添加歌曲到播放列表失败:" << query.lastError().text();
        return false;
    }

    return true;
}

bool PlaylistRepository::removeSongFromPlaylist(const QString& playlistId, const QString& songId) {
    QSqlQuery query;
    query.prepare("DELETE FROM playlist_songs WHERE playlist_id = ? AND song_id = ?");
    query.addBindValue(playlistId);
    query.addBindValue(songId);

    return query.exec();
}

QList<Song> PlaylistRepository::getSongsInPlaylist(const QString& playlistId) {
    QList<Song> songs;
    QSqlQuery query;
    query.prepare(R"(
        SELECT s.* FROM songs s
        INNER JOIN playlist_songs ps ON s.id = ps.song_id
        WHERE ps.playlist_id = ?
        ORDER BY s.title
    )");
    query.addBindValue(playlistId);

    if (query.exec()) {
        while (query.next()) {
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

            songs.append(song);
        }
    }

    return songs;
}

int PlaylistRepository::count() {
    QSqlQuery query("SELECT COUNT(*) FROM playlists");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int PlaylistRepository::getSongCountInPlaylist(const QString& playlistId) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM playlist_songs WHERE playlist_id = ?");
    query.addBindValue(playlistId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

Playlist PlaylistRepository::playlistFromQuery(const QSqlQuery& query) {
    Playlist playlist;
    playlist.setId(query.value("id").toString());
    playlist.setName(query.value("name").toString());
    playlist.setDescription(query.value("description").toString());

    return playlist;
}

int PlaylistRepository::addSongsToPlaylist(const QString& playlistId, const QStringList& songIds) {
    if (playlistId.isEmpty() || songIds.isEmpty()) {
        qWarning() << "PlaylistRepository: 批量添加失败 - 参数为空";
        return 0;
    }

    int successCount = 0;

    // 使用事务
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO playlist_songs (playlist_id, song_id) VALUES (?, ?)");

    for (const QString& songId : songIds) {
        query.addBindValue(playlistId);
        query.addBindValue(songId);

        if (query.exec()) {
            // 检查是否真的插入了新记录
            if (query.numRowsAffected() > 0) {
                successCount++;
            }
        }
        else {
            qWarning() << "PlaylistRepository: 添加歌曲失败, songId:" << songId
                << ", 错误:" << query.lastError().text();
        }
    }

    if (successCount > 0) {
        db.commit();
        qDebug() << "✅ PlaylistRepository: 成功添加" << successCount << "首歌曲到歌单";
    }
    else {
        db.rollback();
        qWarning() << "⚠️ PlaylistRepository: 批量添加失败，已回滚";
    }

    return successCount;
}

bool PlaylistRepository::isSongInPlaylist(const QString& playlistId, const QString& songId) {
    if (playlistId.isEmpty() || songId.isEmpty()) {
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT 1 FROM playlist_songs WHERE playlist_id = ? AND song_id = ? LIMIT 1");
    query.addBindValue(playlistId);
    query.addBindValue(songId);

    if (query.exec() && query.next()) {
        return true;
    }

    return false;
}

int PlaylistRepository::removeSongsFromPlaylist(const QString& playlistId, const QStringList& songIds) {
    if (playlistId.isEmpty() || songIds.isEmpty()) {
        qWarning() << "PlaylistRepository: 批量移除失败 - 参数为空";
        return 0;
    }

    int successCount = 0;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    for (const QString& songId : songIds) {
        if (removeSongFromPlaylist(playlistId, songId)) {
            successCount++;
        }
    }

    if (successCount > 0) {
        db.commit();
        qDebug() << "✅ PlaylistRepository: 成功从歌单移除" << successCount << "首歌曲";
    }
    else {
        db.rollback();
    }

    return successCount;
}

bool PlaylistRepository::clearPlaylist(const QString& playlistId) {
    if (playlistId.isEmpty()) {
        qWarning() << "PlaylistRepository: 清空歌单失败 - ID 为空";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM playlist_songs WHERE playlist_id = ?");
    query.addBindValue(playlistId);

    if (!query.exec()) {
        qWarning() << "PlaylistRepository: 清空歌单失败:" << query.lastError().text();
        return false;
    }

    int removedCount = query.numRowsAffected();
    qDebug() << "✅ PlaylistRepository: 已清空歌单，移除" << removedCount << "首歌曲";

    return true;
}
