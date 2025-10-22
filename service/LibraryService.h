// service/LibraryService.h
#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "../data/SongRepository.h"
#include "../data/PlaylistRepository.h"
#include "../common/entities/Song.h"
#include "../common/entities/Playlist.h"

class LibraryService : public QObject {
    Q_OBJECT

public:
    explicit LibraryService(QObject* parent = nullptr);
    ~LibraryService() override = default;

    // ========== 歌曲管理 ==========

    /**
     * @brief 更新歌曲信息
     * @param id 歌曲ID
     * @param title 新标题
     * @param artist 新艺术家
     * @return 是否更新成功
     */
    bool updateSongInfo(const QString& id, const QString& title, const QString& artist);

    /**
     * @brief 删除歌曲（包括本地文件）
     * @param id 歌曲ID
     * @return 是否删除成功
     */
    bool deleteSong(const QString& id);

    /**
     * @brief 批量删除歌曲
     * @param ids 歌曲ID列表
     * @return 成功删除的数量
     */
    int deleteSongs(const QStringList& ids);

    /**
     * @brief 切换歌曲收藏状态
     * @param id 歌曲ID
     * @return 是否切换成功
     */
    bool toggleFavorite(const QString& id);

    // ========== 歌曲查询 ==========

    /**
     * @brief 获取所有歌曲
     * @return 歌曲列表
     */
    QList<Song> getAllSongs();

    /**
     * @brief 获取收藏的歌曲
     * @return 收藏歌曲列表
     */
    QList<Song> getFavoriteSongs();

    /**
     * @brief 搜索歌曲
     * @param keyword 搜索关键词
     * @return 匹配的歌曲列表
     */
    QList<Song> searchSongs(const QString& keyword);

    /**
     * @brief 根据ID获取歌曲
     * @param id 歌曲ID
     * @return 歌曲对象
     */
    Song getSongById(const QString& id);

    /**
     * @brief 获取歌曲总数
     * @return 歌曲数量
     */
    int getSongCount();

    // ========== 歌单管理 ==========

    /**
     * @brief 创建新歌单
     * @param name 歌单名称
     * @param description 歌单描述
     * @return 新歌单的ID（失败返回空字符串）
     */
    QString createPlaylist(const QString& name, const QString& description = QString());

    /**
     * @brief 更新歌单信息
     * @param id 歌单ID
     * @param name 新名称
     * @param description 新描述
     * @return 是否更新成功
     */
    bool updatePlaylist(const QString& id, const QString& name, const QString& description);

    /**
     * @brief 删除歌单
     * @param id 歌单ID
     * @return 是否删除成功
     */
    bool deletePlaylist(const QString& id);

    /**
     * @brief 清空歌单（移除所有歌曲，保留歌单）
     * @param id 歌单ID
     * @return 是否成功
     */
    bool clearPlaylist(const QString& id);

    // ========== 歌单-歌曲关联 ==========

    /**
     * @brief 添加歌曲到歌单
     * @param playlistId 歌单ID
     * @param songIds 歌曲ID列表
     * @return 成功添加的数量
     */
    int addSongsToPlaylist(const QString& playlistId, const QStringList& songIds);

    /**
     * @brief 从歌单移除歌曲
     * @param playlistId 歌单ID
     * @param songId 歌曲ID
     * @return 是否移除成功
     */
    bool removeSongFromPlaylist(const QString& playlistId, const QString& songId);

    /**
     * @brief 批量从歌单移除歌曲
     * @param playlistId 歌单ID
     * @param songIds 歌曲ID列表
     * @return 成功移除的数量
     */
    int removeSongsFromPlaylist(const QString& playlistId, const QStringList& songIds);

    // ========== 歌单查询 ==========

    /**
     * @brief 获取所有歌单
     * @return 歌单列表
     */
    QList<Playlist> getAllPlaylists();

    /**
     * @brief 根据ID获取歌单
     * @param id 歌单ID
     * @return 歌单对象
     */
    Playlist getPlaylistById(const QString& id);

    /**
     * @brief 获取歌单中的歌曲
     * @param playlistId 歌单ID
     * @return 歌曲列表
     */
    QList<Song> getPlaylistSongs(const QString& playlistId);

    /**
     * @brief 获取歌单中的歌曲数量
     * @param playlistId 歌单ID
     * @return 歌曲数量
     */
    int getPlaylistSongCount(const QString& playlistId);

    /**
     * @brief 检查歌曲是否在歌单中
     * @param playlistId 歌单ID
     * @param songId 歌曲ID
     * @return 是否存在
     */
    bool isSongInPlaylist(const QString& playlistId, const QString& songId);

    // ========== 导出功能 ==========

    /**
     * @brief 歌单导出数据结构
     */
    struct ExportData {
        QString version = "1.0";
        QString playlistName;
        QString playlistDescription;
        QString exportDate;
        QList<Song> songs;

        /**
         * @brief 转换为JSON对象
         */
        QJsonObject toJson() const;

        /**
         * @brief 从JSON对象解析
         */
        static ExportData fromJson(const QJsonObject& json);
    };

    /**
     * @brief 导出歌单到JSON文件
     * @param playlistId 歌单ID
     * @param filePath 保存路径
     * @return 是否导出成功
     */
    bool exportPlaylist(const QString& playlistId, const QString& filePath);

    /**
     * @brief 解析导入的JSON文件
     * @param filePath 文件路径
     * @return 解析结果（失败时songs为空）
     */
    ExportData parseImportFile(const QString& filePath);

    /**
     * @brief 验证导出文件格式
     * @param filePath 文件路径
     * @return 是否为有效格式
     */
    bool validateExportFile(const QString& filePath);

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

private:
    // Repository 实例
    SongRepository* m_songRepository;
    PlaylistRepository* m_playlistRepository;

    // 辅助方法
    bool validateSongId(const QString& id);
    bool validatePlaylistId(const QString& id);
    bool validatePlaylistName(const QString& name);
    QString sanitizePlaylistName(const QString& name);
};
