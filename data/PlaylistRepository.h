#pragma once
#include <QObject>

class PlaylistRepository : public QObject {
    Q_OBJECT
public:
    explicit PlaylistRepository(QObject* parent = nullptr);
    static PlaylistRepository& instance();
};
