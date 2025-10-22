#include "BiliMusicPlayerApp.h"
#include "../common/AppConfig.h"
#include "../data/DatabaseManager.h"
#include "../service/DownloadService.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

BiliMusicPlayerApp::BiliMusicPlayerApp(QObject* parent)
    : QObject(parent)
    , m_downloadService(nullptr)
{
    QCoreApplication::setApplicationName("BiliMusicPlayer");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("BiliMusicPlayer");

    setupLogging();
}

BiliMusicPlayerApp::~BiliMusicPlayerApp()
{
    if (m_downloadService) {
        delete m_downloadService;
        m_downloadService = nullptr;
    }
}

bool BiliMusicPlayerApp::initialize() {
    qDebug() << "正在初始化 BiliMusicPlayer...";

    qDebug() << "加载配置文件...";
    AppConfig& config = AppConfig::instance();
    if (!config.load()) {
        qWarning() << "配置加载失败，将使用默认值";
        config.save();
    }
    qDebug() << "✅ 配置加载成功:" << config.getConfigFilePath();
    qDebug() << "📊 数据库路径:" << config.getDatabasePath();
    qDebug() << "📁 下载路径:" << config.getDownloadPath();

    if (!initializeDatabase()) {
        qCritical() << "数据库初始化失败";
        return false;
    }

    if (!initializeServices()) {
        qCritical() << "服务初始化失败";
        return false;
    }

    qDebug() << "✅ BiliMusicPlayer 初始化成功";
    return true;
}

bool BiliMusicPlayerApp::initializeDatabase() {
    qDebug() << "初始化数据库...";

    AppConfig& config = AppConfig::instance();
    QString dbPath = config.getDatabasePath();

    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.initialize(dbPath)) {
        qCritical() << "❌ 数据库初始化失败";
        return false;
    }

    qDebug() << "✅ 数据库初始化成功";
    return true;
}

bool BiliMusicPlayerApp::initializeServices() {
    qDebug() << "初始化服务...";

    AppConfig& config = AppConfig::instance();

    QString downloadPath = config.getDownloadPath();
    QDir downloadDir(downloadPath);
    if (!downloadDir.exists()) {
        qDebug() << "📁 下载目录不存在，正在创建:" << downloadPath;
        if (downloadDir.mkpath(".")) {
            qDebug() << "✅ 下载目录创建成功:" << downloadDir.absolutePath();
        }
        else {
            qWarning() << "⚠️ 无法创建下载目录:" << downloadDir.absolutePath();
        }
    }
    else {
        qDebug() << "📁 下载目录已存在:" << downloadDir.absolutePath();
    }

    m_downloadService = new DownloadService(this);

    qDebug() << "✅ 下载服务初始化成功";

    qDebug() << "✅ 服务初始化成功";
    return true;
}

void BiliMusicPlayerApp::setupLogging() {
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss}] %{type}: %{message}");
}

DownloadService* BiliMusicPlayerApp::getDownloadService() const {
    return m_downloadService;
}