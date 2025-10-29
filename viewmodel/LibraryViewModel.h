// viewmodel/LibraryViewModel.h
#pragma once

#include <QObject>
#include <QList>
#include "../service/LibraryService.h"
#include "../common/entities/Song.h"
#include "../common/entities/Playlist.h"

class LibraryViewModel : public QObject {
    Q_OBJECT

        Q_PROPERTY(int songCount READ songCount NOTIFY songCountChanged)
        Q_PROPERTY(int playlistCount READ playlistCount NOTIFY playlistCountChanged)

public:
    explicit LibraryViewModel(QObject* parent = nullptr);
    ~LibraryViewModel() override = default;

    static LibraryViewModel& instance();

    // ========== 歌曲操作 ==========

    /**
     * @brief 获取所有歌曲
     */
    Q_INVOKABLE QList<Song> getAllSongs();

    /**
     * @brief 获取收藏的歌曲
     */
    Q_INVOKABLE QList<Song> getFavoriteSongs();

    /**
     * @brief 搜索歌曲
     */
    Q_INVOKABLE QList<Song> searchSongs(const QString& keyword);

    /**
     * @brief 更新歌曲信息
     */
    Q_INVOKABLE void updateSong(const QString& id, const QString& title, const QString& artist);

    /**
     * @brief 删除歌曲
     */
    Q_INVOKABLE void deleteSong(const QString& id);

    /**
     * @brief 批量删除歌曲
     */
    Q_INVOKABLE void deleteSongs(const QStringList& ids);

    /**
     * @brief 切换收藏状态
     */
    Q_INVOKABLE void toggleFavorite(const QString& id);

    /**
     * @brief 根据ID获取歌曲
     */
    Q_INVOKABLE Song getSongById(const QString& id);

    // ========== 歌单操作 ==========

    /**
     * @brief 获取所有歌单
     */
    Q_INVOKABLE QList<Playlist> getAllPlaylists();

    /**
     * @brief 创建新歌单
     */
    Q_INVOKABLE QString createPlaylist(const QString& name, const QString& description = QString());

    /**
     * @brief 更新歌单信息
     */
    Q_INVOKABLE void updatePlaylist(const QString& id, const QString& name, const QString& description);

    /**
     * @brief 删除歌单
     */
    Q_INVOKABLE void deletePlaylist(const QString& id);

    /**
     * @brief 清空歌单
     */
    Q_INVOKABLE void clearPlaylist(const QString& id);

    /**
     * @brief 获取歌单中的歌曲
     */
    Q_INVOKABLE QList<Song> getPlaylistSongs(const QString& playlistId);

    /**
     * @brief 添加歌曲到歌单
     */
    Q_INVOKABLE void addSongsToPlaylist(const QString& playlistId, const QStringList& songIds);

    /**
     * @brief 从歌单移除歌曲
     */
    Q_INVOKABLE void removeSongFromPlaylist(const QString& playlistId, const QString& songId);

    /**
     * @brief 批量从歌单移除歌曲
     */
    Q_INVOKABLE void removeSongsFromPlaylist(const QString& playlistId, const QStringList& songIds);

    /**
     * @brief 检查歌曲是否在歌单中
     */
    Q_INVOKABLE bool isSongInPlaylist(const QString& playlistId, const QString& songId);

    // ========== 导出/导入 ==========

    /**
     * @brief 导出歌单
     */
    Q_INVOKABLE void exportPlaylist(const QString& playlistId, const QString& filePath);

    /**
     * @brief 验证导出文件
     */
    Q_INVOKABLE bool validateExportFile(const QString& filePath);

    /**
     * @brief 解析导入文件
     */
    Q_INVOKABLE LibraryService::ExportData parseImportFile(const QString& filePath);

    /**
     * @brief 导入并下载缺失的歌曲
	 */
    Q_INVOKABLE bool importAndDownloadMissingSongs(const QString& playlistId, const QList<Song>& songs);
    // ========== 统计信息 ==========

    int songCount() const;
    int playlistCount() const;
    int getPlaylistSongCount(const QString& playlistId);

signals:
    // ========== 数据变更信号 ==========
    void songCountChanged();
    void playlistCountChanged();
    void songsChanged();
    void playlistsChanged();

    // ========== 歌曲操作结果信号 ==========
    void songUpdated(const Song& song);
    void songDeleted(const QString& id);
    void songsDeleted(int count);
    void songFavoriteToggled(const QString& id, bool isFavorite);

    // ========== 歌单操作结果信号 ==========
    void playlistCreated(const Playlist& playlist);
    void playlistUpdated(const Playlist& playlist);
    void playlistDeleted(const QString& id);
    void playlistCleared(const QString& id);

    // ========== 歌单-歌曲关联信号 ==========
    void songsAddedToPlaylist(const QString& playlistId, int count);
    void songRemovedFromPlaylist(const QString& playlistId, const QString& songId);

    // ========== 导出信号 ==========
    void exportCompleted(bool success, const QString& message);

    // ========== 通用操作信号 ==========
    void operationSuccess(const QString& message);
    void operationFailed(const QString& error);

private slots:
    // LibraryService 信号处理
    void onSongUpdated(const Song& song);
    void onSongDeleted(const QString& id);
    void onSongFavoriteToggled(const QString& id, bool isFavorite);
    void onPlaylistCreated(const Playlist& playlist);
    void onPlaylistUpdated(const Playlist& playlist);
    void onPlaylistDeleted(const QString& id);
    void onPlaylistCleared(const QString& id);
    void onSongsAddedToPlaylist(const QString& playlistId, int count);
    void onSongRemovedFromPlaylist(const QString& playlistId, const QString& songId);
    void onExportCompleted(bool success, const QString& message);
    void onOperationFailed(const QString& operation, const QString& error);

private:
    LibraryService* m_libraryService;

    // 缓存数据（可选，用于优化性能）
    mutable int m_cachedSongCount = -1;
    mutable int m_cachedPlaylistCount = -1;

    void invalidateCache();
};
