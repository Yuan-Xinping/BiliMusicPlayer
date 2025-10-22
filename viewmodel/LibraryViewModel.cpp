// viewmodel/LibraryViewModel.cpp
#include "LibraryViewModel.h"
#include <QDebug>

LibraryViewModel::LibraryViewModel(QObject* parent)
    : QObject(parent)
    , m_libraryService(new LibraryService(this))
{
    // 连接 LibraryService 的信号
    connect(m_libraryService, &LibraryService::songUpdated,
        this, &LibraryViewModel::onSongUpdated);
    connect(m_libraryService, &LibraryService::songDeleted,
        this, &LibraryViewModel::onSongDeleted);
    connect(m_libraryService, &LibraryService::songFavoriteToggled,
        this, &LibraryViewModel::onSongFavoriteToggled);

    connect(m_libraryService, &LibraryService::playlistCreated,
        this, &LibraryViewModel::onPlaylistCreated);
    connect(m_libraryService, &LibraryService::playlistUpdated,
        this, &LibraryViewModel::onPlaylistUpdated);
    connect(m_libraryService, &LibraryService::playlistDeleted,
        this, &LibraryViewModel::onPlaylistDeleted);
    connect(m_libraryService, &LibraryService::playlistCleared,
        this, &LibraryViewModel::onPlaylistCleared);

    connect(m_libraryService, &LibraryService::songsAddedToPlaylist,
        this, &LibraryViewModel::onSongsAddedToPlaylist);
    connect(m_libraryService, &LibraryService::songRemovedFromPlaylist,
        this, &LibraryViewModel::onSongRemovedFromPlaylist);

    connect(m_libraryService, &LibraryService::exportCompleted,
        this, &LibraryViewModel::onExportCompleted);
    connect(m_libraryService, &LibraryService::operationFailed,
        this, &LibraryViewModel::onOperationFailed);

    qDebug() << "✅ LibraryViewModel 初始化完成";
}

LibraryViewModel& LibraryViewModel::instance() {
    static LibraryViewModel instance;
    return instance;
}

// ========== 歌曲操作 ==========

QList<Song> LibraryViewModel::getAllSongs() {
    return m_libraryService->getAllSongs();
}

QList<Song> LibraryViewModel::getFavoriteSongs() {
    return m_libraryService->getFavoriteSongs();
}

QList<Song> LibraryViewModel::searchSongs(const QString& keyword) {
    return m_libraryService->searchSongs(keyword);
}

void LibraryViewModel::updateSong(const QString& id, const QString& title, const QString& artist) {
    qDebug() << "LibraryViewModel: 请求更新歌曲 -" << id;
    m_libraryService->updateSongInfo(id, title, artist);
}

void LibraryViewModel::deleteSong(const QString& id) {
    qDebug() << "LibraryViewModel: 请求删除歌曲 -" << id;
    m_libraryService->deleteSong(id);
}

void LibraryViewModel::deleteSongs(const QStringList& ids) {
    qDebug() << "LibraryViewModel: 请求批量删除" << ids.size() << "首歌曲";
    int count = m_libraryService->deleteSongs(ids);
    if (count > 0) {
        emit songsDeleted(count);
        invalidateCache();
    }
}

void LibraryViewModel::toggleFavorite(const QString& id) {
    qDebug() << "LibraryViewModel: 请求切换收藏状态 -" << id;
    m_libraryService->toggleFavorite(id);
}

Song LibraryViewModel::getSongById(const QString& id) {
    return m_libraryService->getSongById(id);
}

// ========== 歌单操作 ==========

QList<Playlist> LibraryViewModel::getAllPlaylists() {
    return m_libraryService->getAllPlaylists();
}

QString LibraryViewModel::createPlaylist(const QString& name, const QString& description) {
    qDebug() << "LibraryViewModel: 请求创建歌单 -" << name;
    return m_libraryService->createPlaylist(name, description);
}

void LibraryViewModel::updatePlaylist(const QString& id, const QString& name, const QString& description) {
    qDebug() << "LibraryViewModel: 请求更新歌单 -" << id;
    m_libraryService->updatePlaylist(id, name, description);
}

void LibraryViewModel::deletePlaylist(const QString& id) {
    qDebug() << "LibraryViewModel: 请求删除歌单 -" << id;
    m_libraryService->deletePlaylist(id);
}

void LibraryViewModel::clearPlaylist(const QString& id) {
    qDebug() << "LibraryViewModel: 请求清空歌单 -" << id;
    m_libraryService->clearPlaylist(id);
}

QList<Song> LibraryViewModel::getPlaylistSongs(const QString& playlistId) {
    return m_libraryService->getPlaylistSongs(playlistId);
}

void LibraryViewModel::addSongsToPlaylist(const QString& playlistId, const QStringList& songIds) {
    qDebug() << "LibraryViewModel: 请求添加" << songIds.size() << "首歌曲到歌单";
    m_libraryService->addSongsToPlaylist(playlistId, songIds);
}

void LibraryViewModel::removeSongFromPlaylist(const QString& playlistId, const QString& songId) {
    qDebug() << "LibraryViewModel: 请求从歌单移除歌曲";
    m_libraryService->removeSongFromPlaylist(playlistId, songId);
}

void LibraryViewModel::removeSongsFromPlaylist(const QString& playlistId, const QStringList& songIds) {
    qDebug() << "LibraryViewModel: 请求从歌单批量移除" << songIds.size() << "首歌曲";
    m_libraryService->removeSongsFromPlaylist(playlistId, songIds);
}

bool LibraryViewModel::isSongInPlaylist(const QString& playlistId, const QString& songId) {
    return m_libraryService->isSongInPlaylist(playlistId, songId);
}

// ========== 导出/导入 ==========

void LibraryViewModel::exportPlaylist(const QString& playlistId, const QString& filePath) {
    qDebug() << "LibraryViewModel: 请求导出歌单到" << filePath;
    m_libraryService->exportPlaylist(playlistId, filePath);
}

bool LibraryViewModel::validateExportFile(const QString& filePath) {
    return m_libraryService->validateExportFile(filePath);
}

LibraryService::ExportData LibraryViewModel::parseImportFile(const QString& filePath) {
    return m_libraryService->parseImportFile(filePath);
}

// ========== 统计信息 ==========

int LibraryViewModel::songCount() const {
    if (m_cachedSongCount < 0) {
        m_cachedSongCount = m_libraryService->getSongCount();
    }
    return m_cachedSongCount;
}

int LibraryViewModel::playlistCount() const {
    if (m_cachedPlaylistCount < 0) {
        m_cachedPlaylistCount = m_libraryService->getAllPlaylists().size();
    }
    return m_cachedPlaylistCount;
}

int LibraryViewModel::getPlaylistSongCount(const QString& playlistId) {
    return m_libraryService->getPlaylistSongCount(playlistId);
}

// ========== 信号处理 ==========

void LibraryViewModel::onSongUpdated(const Song& song) {
    emit songUpdated(song);
    emit songsChanged();
    emit operationSuccess(QString("歌曲'%1'已更新").arg(song.getTitle()));
    qDebug() << "✅ LibraryViewModel: 歌曲更新通知已发送";
}

void LibraryViewModel::onSongDeleted(const QString& id) {
    emit songDeleted(id);
    emit songsChanged();
    invalidateCache();
    emit operationSuccess("歌曲已删除");
    qDebug() << "✅ LibraryViewModel: 歌曲删除通知已发送";
}

void LibraryViewModel::onSongFavoriteToggled(const QString& id, bool isFavorite) {
    emit songFavoriteToggled(id, isFavorite);
    emit songsChanged();
    QString message = isFavorite ? "已添加到收藏" : "已取消收藏";
    emit operationSuccess(message);
    qDebug() << "✅ LibraryViewModel: 收藏状态切换通知已发送";
}

void LibraryViewModel::onPlaylistCreated(const Playlist& playlist) {
    emit playlistCreated(playlist);
    emit playlistsChanged();
    invalidateCache();
    emit operationSuccess(QString("歌单'%1'已创建").arg(playlist.getName()));
    qDebug() << "✅ LibraryViewModel: 歌单创建通知已发送";
}

void LibraryViewModel::onPlaylistUpdated(const Playlist& playlist) {
    emit playlistUpdated(playlist);
    emit playlistsChanged();
    emit operationSuccess(QString("歌单'%1'已更新").arg(playlist.getName()));
    qDebug() << "✅ LibraryViewModel: 歌单更新通知已发送";
}

void LibraryViewModel::onPlaylistDeleted(const QString& id) {
    emit playlistDeleted(id);
    emit playlistsChanged();
    invalidateCache();
    emit operationSuccess("歌单已删除");
    qDebug() << "✅ LibraryViewModel: 歌单删除通知已发送";
}

void LibraryViewModel::onPlaylistCleared(const QString& id) {
    emit playlistCleared(id);
    emit operationSuccess("歌单已清空");
    qDebug() << "✅ LibraryViewModel: 歌单清空通知已发送";
}

void LibraryViewModel::onSongsAddedToPlaylist(const QString& playlistId, int count) {
    emit songsAddedToPlaylist(playlistId, count);
    emit operationSuccess(QString("已添加 %1 首歌曲到歌单").arg(count));
    qDebug() << "✅ LibraryViewModel: 歌曲添加到歌单通知已发送";
}

void LibraryViewModel::onSongRemovedFromPlaylist(const QString& playlistId, const QString& songId) {
    emit songRemovedFromPlaylist(playlistId, songId);
    emit operationSuccess("歌曲已从歌单移除");
    qDebug() << "✅ LibraryViewModel: 歌曲从歌单移除通知已发送";
}

void LibraryViewModel::onExportCompleted(bool success, const QString& message) {
    emit exportCompleted(success, message);
    if (success) {
        emit operationSuccess(message);
    }
    else {
        emit operationFailed(message);
    }
    qDebug() << "✅ LibraryViewModel: 导出完成通知已发送";
}

void LibraryViewModel::onOperationFailed(const QString& operation, const QString& error) {
    QString fullError = QString("%1失败: %2").arg(operation, error);
    emit operationFailed(fullError);
    qWarning() << "❌ LibraryViewModel:" << fullError;
}

void LibraryViewModel::invalidateCache() {
    m_cachedSongCount = -1;
    m_cachedPlaylistCount = -1;
    emit songCountChanged();
    emit playlistCountChanged();
}
