#pragma once
#include "../common/entities/Song.h"
#include <QList>
#include <QString>
#include <QObject>

class SongRepository : public QObject {
    Q_OBJECT

public:
    explicit SongRepository(QObject* parent = nullptr);

    // CRUD 操作
    bool save(const Song& song);
    bool update(const Song& song);
    bool deleteById(const QString& id);

    // 查询操作
    QList<Song> findAll();
    Song findById(const QString& id);
    QList<Song> findByTitle(const QString& title);
    QList<Song> findFavorites();

    // 统计操作
    int count();
    bool exists(const QString& id);

    /**
     * @brief 更新歌曲的标题和艺术家信息
     * @param id 歌曲ID
     * @param title 新标题
     * @param artist 新艺术家
     * @return 是否更新成功
     */
    bool updateSongInfo(const QString& id, const QString& title, const QString& artist);

    /**
     * @brief 删除歌曲记录并删除本地文件
     * @param id 歌曲ID
     * @return 是否删除成功
     */
    bool deleteSongWithFile(const QString& id);

    /**
     * @brief 切换歌曲的收藏状态
     * @param id 歌曲ID
     * @return 是否切换成功
     */
    bool toggleFavorite(const QString& id);

    /**
     * @brief 通过关键词搜索歌曲（同时搜索标题和艺术家）
     * @param keyword 搜索关键词
     * @return 匹配的歌曲列表
     */
    QList<Song> searchByKeyword(const QString& keyword);

    /**
     * @brief 批量删除歌曲
     * @param ids 歌曲ID列表
     * @return 成功删除的数量
     */
    int deleteBatch(const QStringList& ids);

private:
    Song songFromQuery(const class QSqlQuery& query);
};
