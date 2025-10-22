// service/LibraryService.cpp
#include "LibraryService.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QFileInfo>
#include <QDebug>
#include <QUuid>

LibraryService::LibraryService(QObject* parent)
    : QObject(parent)
    , m_songRepository(new SongRepository(this))
    , m_playlistRepository(new PlaylistRepository(this))
{
    qDebug() << "✅ LibraryService 初始化完成";
}

// ========== 歌曲管理 ==========

bool LibraryService::updateSongInfo(const QString& id, const QString& title, const QString& artist) {
    if (!validateSongId(id)) {
        emit operationFailed("更新歌曲", "无效的歌曲ID");
        return false;
    }

    if (title.trimmed().isEmpty()) {
        emit operationFailed("更新歌曲", "歌曲标题不能为空");
        return false;
    }

    bool success = m_songRepository->updateSongInfo(id, title, artist);

    if (success) {
        Song updatedSong = m_songRepository->findById(id);
        emit songUpdated(updatedSong);
        qDebug() << "✅ LibraryService: 歌曲信息已更新 -" << title;
    }
    else {
        emit operationFailed("更新歌曲", "数据库更新失败");
    }

    return success;
}

bool LibraryService::deleteSong(const QString& id) {
    if (!validateSongId(id)) {
        emit operationFailed("删除歌曲", "无效的歌曲ID");
        return false;
    }

    // 获取歌曲信息用于日志
    Song song = m_songRepository->findById(id);
    QString songTitle = song.getId().isEmpty() ? id : song.getTitle();

    bool success = m_songRepository->deleteSongWithFile(id);

    if (success) {
        emit songDeleted(id);
        qDebug() << "✅ LibraryService: 歌曲已删除 -" << songTitle;
    }
    else {
        emit operationFailed("删除歌曲", "删除失败");
    }

    return success;
}

int LibraryService::deleteSongs(const QStringList& ids) {
    if (ids.isEmpty()) {
        emit operationFailed("批量删除", "歌曲列表为空");
        return 0;
    }

    int successCount = m_songRepository->deleteBatch(ids);

    if (successCount > 0) {
        for (const QString& id : ids) {
            emit songDeleted(id);
        }
        qDebug() << "✅ LibraryService: 批量删除完成，共删除" << successCount << "首歌曲";
    }

    return successCount;
}

bool LibraryService::toggleFavorite(const QString& id) {
    if (!validateSongId(id)) {
        emit operationFailed("切换收藏", "无效的歌曲ID");
        return false;
    }

    bool success = m_songRepository->toggleFavorite(id);

    if (success) {
        Song song = m_songRepository->findById(id);
        emit songFavoriteToggled(id, song.isFavorite());
        qDebug() << "✅ LibraryService: 收藏状态已切换 -" << song.getTitle();
    }
    else {
        emit operationFailed("切换收藏", "操作失败");
    }

    return success;
}

// ========== 歌曲查询 ==========

QList<Song> LibraryService::getAllSongs() {
    QList<Song> songs = m_songRepository->findAll();
    qDebug() << "📚 LibraryService: 获取所有歌曲，共" << songs.size() << "首";
    return songs;
}

QList<Song> LibraryService::getFavoriteSongs() {
    QList<Song> songs = m_songRepository->findFavorites();
    qDebug() << "❤️ LibraryService: 获取收藏歌曲，共" << songs.size() << "首";
    return songs;
}

QList<Song> LibraryService::searchSongs(const QString& keyword) {
    QList<Song> songs = m_songRepository->searchByKeyword(keyword);
    qDebug() << "🔍 LibraryService: 搜索'" << keyword << "'，找到" << songs.size() << "首";
    return songs;
}

Song LibraryService::getSongById(const QString& id) {
    return m_songRepository->findById(id);
}

int LibraryService::getSongCount() {
    return m_songRepository->count();
}

// ========== 歌单管理 ==========

QString LibraryService::createPlaylist(const QString& name, const QString& description) {
    QString sanitizedName = sanitizePlaylistName(name);

    if (!validatePlaylistName(sanitizedName)) {
        emit operationFailed("创建歌单", "歌单名称无效");
        return QString();
    }

    // 检查是否已存在同名歌单
    Playlist existing = m_playlistRepository->findByName(sanitizedName);
    if (!existing.getId().isEmpty()) {
        emit operationFailed("创建歌单", QString("歌单'%1'已存在").arg(sanitizedName));
        return QString();
    }

    // 创建新歌单
    QString newId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    Playlist newPlaylist(newId, sanitizedName, description);

    bool success = m_playlistRepository->save(newPlaylist);

    if (success) {
        emit playlistCreated(newPlaylist);
        qDebug() << "✅ LibraryService: 歌单已创建 -" << sanitizedName;
        return newId;
    }
    else {
        emit operationFailed("创建歌单", "保存失败");
        return QString();
    }
}

bool LibraryService::updatePlaylist(const QString& id, const QString& name, const QString& description) {
    if (!validatePlaylistId(id)) {
        emit operationFailed("更新歌单", "无效的歌单ID");
        return false;
    }

    QString sanitizedName = sanitizePlaylistName(name);
    if (!validatePlaylistName(sanitizedName)) {
        emit operationFailed("更新歌单", "歌单名称无效");
        return false;
    }

    Playlist playlist = m_playlistRepository->findById(id);
    if (playlist.getId().isEmpty()) {
        emit operationFailed("更新歌单", "歌单不存在");
        return false;
    }

    playlist.setName(sanitizedName);
    playlist.setDescription(description);

    bool success = m_playlistRepository->update(playlist);

    if (success) {
        emit playlistUpdated(playlist);
        qDebug() << "✅ LibraryService: 歌单已更新 -" << sanitizedName;
    }
    else {
        emit operationFailed("更新歌单", "更新失败");
    }

    return success;
}

bool LibraryService::deletePlaylist(const QString& id) {
    if (!validatePlaylistId(id)) {
        emit operationFailed("删除歌单", "无效的歌单ID");
        return false;
    }

    Playlist playlist = m_playlistRepository->findById(id);
    QString playlistName = playlist.getId().isEmpty() ? id : playlist.getName();

    bool success = m_playlistRepository->deleteById(id);

    if (success) {
        emit playlistDeleted(id);
        qDebug() << "✅ LibraryService: 歌单已删除 -" << playlistName;
    }
    else {
        emit operationFailed("删除歌单", "删除失败");
    }

    return success;
}

bool LibraryService::clearPlaylist(const QString& id) {
    if (!validatePlaylistId(id)) {
        emit operationFailed("清空歌单", "无效的歌单ID");
        return false;
    }

    bool success = m_playlistRepository->clearPlaylist(id);

    if (success) {
        emit playlistCleared(id);
        qDebug() << "✅ LibraryService: 歌单已清空";
    }
    else {
        emit operationFailed("清空歌单", "操作失败");
    }

    return success;
}

// ========== 歌单-歌曲关联 ==========

int LibraryService::addSongsToPlaylist(const QString& playlistId, const QStringList& songIds) {
    if (!validatePlaylistId(playlistId)) {
        emit operationFailed("添加歌曲到歌单", "无效的歌单ID");
        return 0;
    }

    if (songIds.isEmpty()) {
        emit operationFailed("添加歌曲到歌单", "歌曲列表为空");
        return 0;
    }

    int successCount = m_playlistRepository->addSongsToPlaylist(playlistId, songIds);

    if (successCount > 0) {
        emit songsAddedToPlaylist(playlistId, successCount);
        qDebug() << "✅ LibraryService: 已添加" << successCount << "首歌曲到歌单";
    }

    return successCount;
}

bool LibraryService::removeSongFromPlaylist(const QString& playlistId, const QString& songId) {
    if (!validatePlaylistId(playlistId) || !validateSongId(songId)) {
        emit operationFailed("从歌单移除歌曲", "无效的ID");
        return false;
    }

    bool success = m_playlistRepository->removeSongFromPlaylist(playlistId, songId);

    if (success) {
        emit songRemovedFromPlaylist(playlistId, songId);
        qDebug() << "✅ LibraryService: 歌曲已从歌单移除";
    }
    else {
        emit operationFailed("从歌单移除歌曲", "移除失败");
    }

    return success;
}

int LibraryService::removeSongsFromPlaylist(const QString& playlistId, const QStringList& songIds) {
    if (!validatePlaylistId(playlistId)) {
        emit operationFailed("批量移除", "无效的歌单ID");
        return 0;
    }

    int successCount = m_playlistRepository->removeSongsFromPlaylist(playlistId, songIds);

    if (successCount > 0) {
        for (const QString& songId : songIds) {
            emit songRemovedFromPlaylist(playlistId, songId);
        }
        qDebug() << "✅ LibraryService: 已从歌单移除" << successCount << "首歌曲";
    }

    return successCount;
}

// ========== 歌单查询 ==========

QList<Playlist> LibraryService::getAllPlaylists() {
    QList<Playlist> playlists = m_playlistRepository->findAll();
    qDebug() << "📋 LibraryService: 获取所有歌单，共" << playlists.size() << "个";
    return playlists;
}

Playlist LibraryService::getPlaylistById(const QString& id) {
    return m_playlistRepository->findById(id);
}

QList<Song> LibraryService::getPlaylistSongs(const QString& playlistId) {
    return m_playlistRepository->getSongsInPlaylist(playlistId);
}

int LibraryService::getPlaylistSongCount(const QString& playlistId) {
    return m_playlistRepository->getSongCountInPlaylist(playlistId);
}

bool LibraryService::isSongInPlaylist(const QString& playlistId, const QString& songId) {
    return m_playlistRepository->isSongInPlaylist(playlistId, songId);
}

// ========== 导出功能 ==========

QJsonObject LibraryService::ExportData::toJson() const {
    QJsonObject root;
    root["version"] = version;

    QJsonObject playlistObj;
    playlistObj["name"] = playlistName;
    playlistObj["description"] = playlistDescription;
    playlistObj["exportDate"] = exportDate;
    root["playlist"] = playlistObj;

    QJsonArray songsArray;
    for (const Song& song : songs) {
        QJsonObject songObj;
        songObj["title"] = song.getTitle();
        songObj["artist"] = song.getArtist();
        songObj["biliUrl"] = song.getBilibiliUrl();
        songObj["coverUrl"] = song.getCoverUrl();
        songObj["duration"] = static_cast<qint64>(song.getDurationSeconds());
        songsArray.append(songObj);
    }
    root["songs"] = songsArray;

    return root;
}

LibraryService::ExportData LibraryService::ExportData::fromJson(const QJsonObject& json) {
    ExportData data;

    data.version = json["version"].toString();

    QJsonObject playlistObj = json["playlist"].toObject();
    data.playlistName = playlistObj["name"].toString();
    data.playlistDescription = playlistObj["description"].toString();
    data.exportDate = playlistObj["exportDate"].toString();

    QJsonArray songsArray = json["songs"].toArray();
    for (const QJsonValue& value : songsArray) {
        QJsonObject songObj = value.toObject();

        Song song;
        song.setTitle(songObj["title"].toString());
        song.setArtist(songObj["artist"].toString());
        song.setBilibiliUrl(songObj["biliUrl"].toString());
        song.setCoverUrl(songObj["coverUrl"].toString());
        song.setDurationSeconds(songObj["duration"].toVariant().toLongLong());

        data.songs.append(song);
    }

    return data;
}

bool LibraryService::exportPlaylist(const QString& playlistId, const QString& filePath) {
    if (!validatePlaylistId(playlistId)) {
        emit exportCompleted(false, "无效的歌单ID");
        return false;
    }

    // 获取歌单信息
    Playlist playlist = m_playlistRepository->findById(playlistId);
    if (playlist.getId().isEmpty()) {
        emit exportCompleted(false, "歌单不存在");
        return false;
    }

    // 获取歌单中的歌曲
    QList<Song> songs = m_playlistRepository->getSongsInPlaylist(playlistId);
    if (songs.isEmpty()) {
        emit exportCompleted(false, "歌单为空，无法导出");
        return false;
    }

    // 构建导出数据
    ExportData exportData;
    exportData.playlistName = playlist.getName();
    exportData.playlistDescription = playlist.getDescription();
    exportData.exportDate = QDateTime::currentDateTime().toString(Qt::ISODate);
    exportData.songs = songs;

    // 转换为JSON
    QJsonObject jsonObj = exportData.toJson();
    QJsonDocument doc(jsonObj);

    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QString error = QString("无法创建文件: %1").arg(file.errorString());
        emit exportCompleted(false, error);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    QString message = QString("成功导出歌单'%1'，共%2首歌曲")
        .arg(playlist.getName())
        .arg(songs.size());
    emit exportCompleted(true, message);
    qDebug() << "✅ LibraryService:" << message;

    return true;
}

LibraryService::ExportData LibraryService::parseImportFile(const QString& filePath) {
    ExportData emptyData;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "LibraryService: 无法打开文件:" << filePath;
        return emptyData;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "LibraryService: JSON 解析错误:" << parseError.errorString();
        return emptyData;
    }

    if (!doc.isObject()) {
        qWarning() << "LibraryService: JSON 根节点不是对象";
        return emptyData;
    }

    ExportData importData = ExportData::fromJson(doc.object());
    qDebug() << "✅ LibraryService: 成功解析导入文件，包含" << importData.songs.size() << "首歌曲";

    return importData;
}

bool LibraryService::validateExportFile(const QString& filePath) {
    if (!QFileInfo::exists(filePath)) {
        return false;
    }

    ExportData data = parseImportFile(filePath);
    return !data.songs.isEmpty();
}

// ========== 辅助方法 ==========

bool LibraryService::validateSongId(const QString& id) {
    return !id.trimmed().isEmpty() && m_songRepository->exists(id);
}

bool LibraryService::validatePlaylistId(const QString& id) {
    if (id.trimmed().isEmpty()) {
        return false;
    }
    Playlist playlist = m_playlistRepository->findById(id);
    return !playlist.getId().isEmpty();
}

bool LibraryService::validatePlaylistName(const QString& name) {
    QString trimmed = name.trimmed();
    return !trimmed.isEmpty() && trimmed.length() <= 100;
}

QString LibraryService::sanitizePlaylistName(const QString& name) {
    return name.trimmed();
}
