#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
}

bool DatabaseManager::initialize(const QString& dbPath) {
    if (m_initialized) {
        qDebug() << "⚠️ DatabaseManager 已经初始化";
        return true;
    }

    if (dbPath.isEmpty()) {
        qCritical() << "❌ 数据库路径为空！";
        return false;
    }

    m_databasePath = dbPath;
    qDebug() << "📊 数据库路径设置为:" << m_databasePath;

    // 确保数据库目录存在
    QFileInfo dbFileInfo(m_databasePath);
    QDir dbDir = dbFileInfo.dir();

    if (!dbDir.exists()) {
        qDebug() << "📁 数据库目录不存在，正在创建:" << dbDir.absolutePath();
        if (!dbDir.mkpath(".")) {
            qCritical() << "❌ 无法创建数据库目录:" << dbDir.absolutePath();
            return false;
        }
        qDebug() << "✅ 数据库目录创建成功:" << dbDir.absolutePath();
    }
    else {
        qDebug() << "📁 数据库目录已存在:" << dbDir.absolutePath();
    }

    // 打开数据库连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(m_databasePath);

    if (!db.open()) {
        qCritical() << "❌ 无法打开数据库:" << db.lastError().text();
        return false;
    }

    qDebug() << "✅ 数据库连接成功:" << m_databasePath;

    // 启用外键约束
    QSqlQuery query;
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "⚠️ 无法启用外键约束:" << query.lastError().text();
    }
    else {
        qDebug() << "✅ 外键约束已启用";
    }

    // 创建表
    if (!createTablesIfNotExist()) {
        qCritical() << "❌ 创建数据库表失败";
        return false;
    }

    m_initialized = true;
    qDebug() << "✅ DatabaseManager 初始化完成";
    return true;
}

QSqlDatabase DatabaseManager::getConnection() {
    return QSqlDatabase::database();
}

bool DatabaseManager::createTablesIfNotExist() {
    QSqlQuery query;

    // 创建 songs 表
    QString createSongsTable = R"(
        CREATE TABLE IF NOT EXISTS songs (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            artist TEXT,
            bilibili_url TEXT NOT NULL,
            local_file_path TEXT NOT NULL,
            cover_url TEXT,
            duration_seconds INTEGER,
            download_date TEXT NOT NULL,
            is_favorite INTEGER DEFAULT 0
        )
    )";

    if (!query.exec(createSongsTable)) {
        qCritical() << "❌ 创建 songs 表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✅ songs 表已创建";

    // 创建 playlists 表
    QString createPlaylistsTable = R"(
        CREATE TABLE IF NOT EXISTS playlists (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL UNIQUE,
            description TEXT
        )
    )";

    if (!query.exec(createPlaylistsTable)) {
        qCritical() << "❌ 创建 playlists 表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✅ playlists 表已创建";

    // 创建 playlist_songs 关联表
    QString createPlaylistSongsTable = R"(
        CREATE TABLE IF NOT EXISTS playlist_songs (
            playlist_id TEXT NOT NULL,
            song_id TEXT NOT NULL,
            PRIMARY KEY (playlist_id, song_id),
            FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,
            FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE
        )
    )";

    if (!query.exec(createPlaylistSongsTable)) {
        qCritical() << "❌ 创建 playlist_songs 表失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "✅ playlist_songs 表已创建";

    qDebug() << "✅ 所有数据库表已创建";
    return true;
}
