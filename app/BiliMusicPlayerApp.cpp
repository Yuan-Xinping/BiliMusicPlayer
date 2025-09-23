#include "BiliMusicPlayerApp.h"
#include <QDebug>

BiliMusicPlayerApp::BiliMusicPlayerApp(QObject* parent)
    : QObject(parent)
{
}

bool BiliMusicPlayerApp::init()
{
    qDebug() << "BiliMusicPlayerApp initialized successfully.";
    return true;
}
