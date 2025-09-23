#pragma once
#include <QObject>

class SongRepository : public QObject {
    Q_OBJECT
public:
    explicit SongRepository(QObject* parent = nullptr);
    static SongRepository& instance();
};
