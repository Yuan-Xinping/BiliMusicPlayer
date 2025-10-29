// service/LibraryService.h
#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>                 
#include "../data/SongRepository.h"
#include "../data/PlaylistRepository.h"
#include "../common/entities/Song.h"
#include "../common/entities/Playlist.h"

class ConcurrentDownloadManager;

class LibraryService : public QObject {
    Q_OBJECT

public:
    explicit LibraryService(QObject* parent = nullptr);
    ~LibraryService() override = default;

    // ========== 歌曲管理 ==========
    bool updateSongInfo(const QString& id, const QString& title, const QString& artist);
    bool deleteSong(const QString& id);
    int deleteSongs(const QStringList& ids);
    bool toggleFavorite(const QString& id);

    // ========== 歌曲查询 ==========
    QList<Song> getAllSongs();
    QList<Song> getFavoriteSongs();
    QList<Song> searchSongs(const QString& keyword);
    Song getSongById(const QString& id);
    int getSongCount();

    // ========== 歌单管理 ==========
    QString createPlaylist(const QString& name, const QString& description = QString());
    bool updatePlaylist(const QString& id, const QString& name, const QString& description);
    bool deletePlaylist(const QString& id);
    bool clearPlaylist(const QString& id);

    // ========== 歌单-歌曲关联 ==========
    int addSongsToPlaylist(const QString& playlistId, const QStringList& songIds);
    bool removeSongFromPlaylist(const QString& playlistId, const QString& songId);
    int removeSongsFromPlaylist(const QString& playlistId, const QStringList& songIds);

    // ========== 歌单查询 ==========
    QList<Playlist> getAllPlaylists();
    Playlist getPlaylistById(const QString& id);
    QList<Song> getPlaylistSongs(const QString& playlistId);
    int getPlaylistSongCount(const QString& playlistId);
    bool isSongInPlaylist(const QString& playlistId, const QString& songId);

    // ========== 导出功能 ==========
    struct ExportData {
        QString version = "1.0";
        QString playlistName;
        QString playlistDescription;
        QString exportDate;
        QList<Song> songs;

        QJsonObject toJson() const;
        static ExportData fromJson(const QJsonObject& json);
    };
    bool exportPlaylist(const QString& playlistId, const QString& filePath);
    ExportData parseImportFile(const QString& filePath);
    bool validateExportFile(const QString& filePath);

    // ========== 导入并触发并行下载 ==========
    // 对导入的歌曲：本地存在 -> 直接加入歌单；本地不存在 -> 提交到并行下载队列
    bool importAndDownloadMissingSongs(const QString& playlistId, const QList<Song>& songs);

signals:
    // ========== 歌曲操作信号 ==========
    void songUpdated(const Song& song);
    void songDeleted(const QString& id);
    void songFavoriteToggled(const QString& id, bool isFavorite);

    // ========== 歌单操作信号 ==========
    void playlistCreated(const Playlist& playlist);
    void playlistUpdated(const Playlist& playlist);
    void playlistDeleted(const QString& id);
    void playlistCleared(const QString& id);

    // ========== 歌单-歌曲关联信号 ==========
    void songsAddedToPlaylist(const QString& playlistId, int count);
    void songRemovedFromPlaylist(const QString& playlistId, const QString& songId);

    // ========== 导出信号 ==========
    void exportCompleted(bool success, const QString& message);

    // ========== 错误信号 ==========
    void operationFailed(const QString& operation, const QString& error);

private slots:
    // 并行下载完成：将下载好的歌曲加入对应歌单
    void onConcurrentTaskCompleted(const QString& taskId, const Song& song);

private:
    // Repository 实例
    SongRepository* m_songRepository;
    PlaylistRepository* m_playlistRepository;

    // 用于追踪任务 -> 目标歌单
    QHash<QString, QString> m_taskToPlaylist;

    // 辅助方法
    bool validateSongId(const QString& id);
    bool validatePlaylistId(const QString& id);
    bool validatePlaylistName(const QString& name);
    QString sanitizePlaylistName(const QString& name);
};
