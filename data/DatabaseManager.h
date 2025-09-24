#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool initialize();
    QSqlDatabase getConnection();
    bool isInitialized() const { return m_initialized; }

private:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager() override = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTablesIfNotExist();
    QString getDatabasePath() const;
    QString getAppDataDirectory() const;

    bool m_initialized = false;
    QString m_databasePath;
    static const QString DB_FILE_NAME;
};
