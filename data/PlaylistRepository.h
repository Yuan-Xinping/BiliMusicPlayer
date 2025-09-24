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

private:
    Playlist playlistFromQuery(const class QSqlQuery& query);
};
