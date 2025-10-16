#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

const QString DatabaseManager::DB_FILE_NAME = "bili_music_player.db";

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject* parent) : QObject(parent) {
    m_databasePath = getDatabasePath();

    QString dbPath = m_databasePath;

    qDebug() << "📊 数据库路径：" << dbPath;
}

bool DatabaseManager::initialize() {
    if (m_initialized) {
        return true;
    }

    // 确锟斤拷锟斤拷锟斤拷目录锟斤拷锟斤拷
    QFileInfo dbFileInfo(m_databasePath);
    QDir dbDir = dbFileInfo.dir();
    if (!dbDir.exists()) {
        if (!dbDir.mkpath(".")) {
            qWarning() << "Failed to create database directory:" << dbDir.absolutePath();
            return false;
        }
        qDebug() << "Created database directory:" << dbDir.absolutePath();
    }

    // 锟斤拷锟斤拷锟斤拷锟捷匡拷锟斤拷锟斤拷
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(m_databasePath);

    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully:" << m_databasePath;

    // 锟斤拷锟斤拷锟斤拷锟皆硷拷锟?
    QSqlQuery query;
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:" << query.lastError().text();
    }

    // 锟斤拷锟斤拷锟斤拷
    if (!createTablesIfNotExist()) {
        return false;
    }

    m_initialized = true;
    return true;
}

QSqlDatabase DatabaseManager::getConnection() {
    return QSqlDatabase::database();
}

bool DatabaseManager::createTablesIfNotExist() {
    QSqlQuery query;

    // 锟斤拷锟斤拷 songs 锟斤拷
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
        qWarning() << "Failed to create songs table:" << query.lastError().text();
        return false;
    }

    // 锟斤拷锟斤拷 playlists 锟斤拷
    QString createPlaylistsTable = R"(
        CREATE TABLE IF NOT EXISTS playlists (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL UNIQUE,
            description TEXT
        )
    )";

    if (!query.exec(createPlaylistsTable)) {
        qWarning() << "Failed to create playlists table:" << query.lastError().text();
        return false;
    }

    // 锟斤拷锟斤拷 playlist_songs 锟斤拷锟斤拷锟斤拷
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
        qWarning() << "Failed to create playlist_songs table:" << query.lastError().text();
        return false;
    }

    qDebug() << "All database tables created successfully";
    return true;
}

QString DatabaseManager::getDatabasePath() const {
    QString appDataDir = getAppDataDirectory();
    QDir dir(appDataDir);
    if (!dir.exists("BiliMusicPlayer")) {
        dir.mkpath("BiliMusicPlayer");
    }

    return dir.filePath("BiliMusicPlayer/" + DB_FILE_NAME);
}

QString DatabaseManager::getAppDataDirectory() const {
#ifdef Q_OS_WIN
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_MAC)
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#else
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#endif
}
