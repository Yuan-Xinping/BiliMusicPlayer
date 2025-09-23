#pragma once

#include <QObject>

class BiliMusicPlayerApp : public QObject
{
    Q_OBJECT

public:
    explicit BiliMusicPlayerApp(QObject* parent = nullptr);

    bool init();
};
