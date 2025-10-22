#include "SongRepository.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QDir>

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

bool SongRepository::updateSongInfo(const QString& id, const QString& title, const QString& artist) {
    if (id.isEmpty()) {
        qWarning() << "SongRepository: 更新失败 - ID 为空";
        return false;
    }

    QSqlQuery query;
    query.prepare("UPDATE songs SET title = ?, artist = ? WHERE id = ?");
    query.addBindValue(title);
    query.addBindValue(artist);
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "SongRepository: 更新歌曲信息失败:" << query.lastError().text();
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected > 0) {
        qDebug() << "SongRepository: 成功更新歌曲信息, ID:" << id;
        qDebug() << "  - 新标题:" << title;
        qDebug() << "  - 新艺术家:" << artist;
    }
    else {
        qWarning() << "SongRepository: 未找到歌曲, ID:" << id;
    }

    return rowsAffected > 0;
}

bool SongRepository::deleteSongWithFile(const QString& id) {
    if (id.isEmpty()) {
        qWarning() << "SongRepository: 删除失败 - ID 为空";
        return false;
    }

    // 获取歌曲信息
    Song song = findById(id);
    if (song.getId().isEmpty()) {
        qWarning() << "SongRepository: 找不到歌曲, ID:" << id;
        return false;
    }

    // 从数据库删除记录
    if (!deleteById(id)) {
        qWarning() << "SongRepository: 从数据库删除失败, ID:" << id;
        return false;
    }

    // 删除本地文件
    QString filePath = song.getLocalFilePath();
    if (!filePath.isEmpty() && QFile::exists(filePath)) {
        QFile file(filePath);
        if (file.remove()) {
            qDebug() << "✅ SongRepository: 成功删除本地文件:" << filePath;
        }
        else {
            qWarning() << "⚠️ SongRepository: 删除本地文件失败:" << filePath;
            qWarning() << "  错误:" << file.errorString();
            // 注意：即使文件删除失败，数据库记录已删除，返回 true
        }
    }
    else {
        qDebug() << "ℹ️ SongRepository: 本地文件不存在或路径为空:" << filePath;
    }

    qDebug() << "✅ SongRepository: 歌曲已完全删除:" << song.getTitle();
    return true;
}

bool SongRepository::toggleFavorite(const QString& id) {
    if (id.isEmpty()) {
        qWarning() << "SongRepository: 切换收藏失败 - ID 为空";
        return false;
    }

    // 获取当前收藏状态
    Song song = findById(id);
    if (song.getId().isEmpty()) {
        qWarning() << "SongRepository: 找不到歌曲, ID:" << id;
        return false;
    }

    // 切换状态
    bool newFavoriteState = !song.isFavorite();

    QSqlQuery query;
    query.prepare("UPDATE songs SET is_favorite = ? WHERE id = ?");
    query.addBindValue(newFavoriteState ? 1 : 0);
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "SongRepository: 切换收藏状态失败:" << query.lastError().text();
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    if (rowsAffected > 0) {
        QString stateText = newFavoriteState ? "已收藏" : "已取消收藏";
        qDebug() << "✅ SongRepository:" << stateText << "-" << song.getTitle();
    }

    return rowsAffected > 0;
}

QList<Song> SongRepository::searchByKeyword(const QString& keyword) {
    QList<Song> songs;

    if (keyword.trimmed().isEmpty()) {
        qDebug() << "SongRepository: 搜索关键词为空，返回所有歌曲";
        return findAll();
    }

    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM songs 
        WHERE title LIKE ? OR artist LIKE ? 
        ORDER BY download_date DESC
    )");

    QString searchPattern = "%" + keyword + "%";
    query.addBindValue(searchPattern);
    query.addBindValue(searchPattern);

    if (!query.exec()) {
        qWarning() << "SongRepository: 搜索失败:" << query.lastError().text();
        return songs;
    }

    while (query.next()) {
        songs.append(songFromQuery(query));
    }

    qDebug() << "SongRepository: 搜索关键词'" << keyword << "'，找到" << songs.size() << "首歌曲";
    return songs;
}

int SongRepository::deleteBatch(const QStringList& ids) {
    if (ids.isEmpty()) {
        qWarning() << "SongRepository: 批量删除失败 - ID 列表为空";
        return 0;
    }

    int successCount = 0;

    // 使用事务提高性能
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    for (const QString& id : ids) {
        if (deleteSongWithFile(id)) {
            successCount++;
        }
    }

    if (successCount > 0) {
        db.commit();
        qDebug() << "✅ SongRepository: 批量删除成功，共删除" << successCount << "首歌曲";
    }
    else {
        db.rollback();
        qWarning() << "⚠️ SongRepository: 批量删除失败，已回滚";
    }

    return successCount;
}

