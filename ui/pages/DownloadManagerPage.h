// ui/pages/DownloadManagerPage.h
#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QMap>

#include "../../service/DownloadService.h"
#include "../../common/entities/Song.h"

class DownloadTaskItem;

class DownloadManagerPage : public QWidget {
    Q_OBJECT

public:
    explicit DownloadManagerPage(DownloadService* downloadService, QWidget* parent = nullptr);

public slots:
    void onSettingsChanged();

private slots:
    void onStartDownloadClicked();

    void onTaskAdded(const DownloadService::DownloadTask& task);
    void onTaskStarted(const DownloadService::DownloadTask& task);
    void onTaskProgress(const DownloadService::DownloadTask& task,
        double progress, const QString& message);
    void onTaskCompleted(const DownloadService::DownloadTask& task,
        const Song& song);
    void onTaskFailed(const DownloadService::DownloadTask& task,
        const QString& error);
    void onTaskSkipped(const QString& identifier, const Song& existingSong);

private:
    void setupUI();
    void setupConnections();
    void setupStyles();
    bool validateInput() const;
    void loadDefaultSettings();

    void addTaskToQueue(const DownloadService::DownloadTask& task);
    void moveTaskToHistory(const QString& identifier, bool success);
    DownloadTaskItem* findTaskItem(const QString& identifier) const;

    // UI 组件
    QLineEdit* m_urlInput = nullptr;
    QComboBox* m_qualityCombo = nullptr;
    QPushButton* m_startBtn = nullptr;
    QPushButton* m_batchDownloadBtn = nullptr;

    QTabWidget* m_tabWidget = nullptr;
    QListWidget* m_queueList = nullptr;
    QTableWidget* m_historyTable = nullptr;

    QLabel* m_statusLabel = nullptr;

    DownloadService* m_downloadService = nullptr;

    QMap<QString, DownloadTaskItem*> m_taskItems;
};
