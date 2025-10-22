#pragma once
#include "../common/entities/Playlist.h"
#include "../common/entities/Song.h"
#include <QList>
#include <QString>
#include <QObject>

class PlaylistRepository : public QObject {
    Q_OBJECT

public:
    explicit PlaylistRepository(QObject* parent = nullptr);

    // Playlist CRUD
    bool save(const Playlist& playlist);
    bool update(const Playlist& playlist);
    bool deleteById(const QString& id);

    // Playlist 查询
    QList<Playlist> findAll();
    Playlist findById(const QString& id);
    Playlist findByName(const QString& name);

    // 播放列表-歌曲关系管理
    bool addSongToPlaylist(const QString& playlistId, const QString& songId);
    bool removeSongFromPlaylist(const QString& playlistId, const QString& songId);
    QList<Song> getSongsInPlaylist(const QString& playlistId);

    // 统计
    int count();
    int getSongCountInPlaylist(const QString& playlistId);

    // ========== 🆕 新增：批量操作 ==========
    /**
     * @brief 批量添加歌曲到歌单
     * @param playlistId 歌单ID
     * @param songIds 歌曲ID列表
     * @return 成功添加的数量
     */
    int addSongsToPlaylist(const QString& playlistId, const QStringList& songIds);

    /**
     * @brief 检查歌曲是否在歌单中
     * @param playlistId 歌单ID
     * @param songId 歌曲ID
     * @return 是否存在
     */
    bool isSongInPlaylist(const QString& playlistId, const QString& songId);

    /**
     * @brief 从歌单中移除多首歌曲
     * @param playlistId 歌单ID
     * @param songIds 歌曲ID列表
     * @return 成功移除的数量
     */
    int removeSongsFromPlaylist(const QString& playlistId, const QStringList& songIds);

    /**
     * @brief 清空歌单（移除所有歌曲，但不删除歌单本身）
     * @param playlistId 歌单ID
     * @return 是否成功
     */
    bool clearPlaylist(const QString& playlistId);

private:
    Playlist playlistFromQuery(const class QSqlQuery& query);
};
