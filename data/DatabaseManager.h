#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool initialize(const QString& dbPath);
    QSqlDatabase getConnection();
    bool isInitialized() const { return m_initialized; }
    QString getDatabasePath() const { return m_databasePath; }

private:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager() override = default;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTablesIfNotExist();

    bool m_initialized = false;
    QString m_databasePath;
};
