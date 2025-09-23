#pragma once
#include <QString>
#include <QList>
#include "Song.h"

class Playlist {
public:
    long long id;
    QString name;
    QList<Song> songs;
};

