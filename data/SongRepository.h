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

private:
    Song songFromQuery(const class QSqlQuery& query);
};
