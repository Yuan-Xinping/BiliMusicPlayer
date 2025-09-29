#include "BiliMusicPlayerApp.h"
#include "../common/AppConfig.h"
#include "../data/DatabaseManager.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

BiliMusicPlayerApp::BiliMusicPlayerApp(QObject* parent)
    : QObject(parent) {

    // 设置应用程序信息
    QCoreApplication::setApplicationName("BiliMusicPlayer");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("BiliMusicPlayer");

    setupLogging();
}

bool BiliMusicPlayerApp::initialize() {
    qDebug() << "正在初始化 BiliMusicPlayer...";

    if (!initializeDatabase()) {
        qCritical() << "数据库初始化失败";
        return false;
    }

    if (!initializeServices()) {
        qCritical() << "服务初始化失败";
        return false;
    }

    qDebug() << "BiliMusicPlayer 初始化成功";
    return true;
}

bool BiliMusicPlayerApp::initializeDatabase() {
    qDebug() << "初始化数据库...";

    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.initialize()) {
        qCritical() << "数据库初始化失败";
        return false;
    }

    qDebug() << "数据库初始化成功";
    return true;
}

bool BiliMusicPlayerApp::initializeServices() {
    qDebug() << "初始化服务...";

    // 加载配置
    AppConfig& config = AppConfig::instance();
    if (!config.loadConfig()) {
        qWarning() << "配置加载失败，将使用默认值";
    }

    // 确保下载目录存在
    QDir downloadDir(config.getDownloadPath());
    if (!downloadDir.exists()) {
        if (downloadDir.mkpath(".")) {
            qDebug() << "创建下载目录:" << downloadDir.absolutePath();
        }
        else {
            qWarning() << "无法创建下载目录:" << downloadDir.absolutePath();
        }
    }

    qDebug() << "服务初始化成功";
    return true;
}

void BiliMusicPlayerApp::setupLogging() {
    // 设置日志格式
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss}] %{type}: %{message}");
}
