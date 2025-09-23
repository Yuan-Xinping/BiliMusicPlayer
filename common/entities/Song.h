#pragma once
#include <QString>

class Song {
public:
    long long id;
    QString title;
    QString artist;
    QString album;
    QString filePath;
    long long duration; // in seconds
};
