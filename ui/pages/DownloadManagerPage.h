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

#include "../../viewmodel/DownloadViewModel.h" 
#include "../../common/entities/Song.h"

class DownloadTaskItem;

class DownloadManagerPage : public QWidget {
    Q_OBJECT

public:
    explicit DownloadManagerPage(DownloadViewModel* viewModel, QWidget* parent = nullptr);  // ✅ 修改构造函数

public slots:
    void onSettingsChanged();

private slots:
    void onStartDownloadClicked();

    void onTaskAdded(const QString& identifier);
    void onTaskStarted(const QString& identifier);
    void onTaskProgressUpdated(const QString& identifier, double progress, const QString& message);
    void onTaskCompleted(const QString& identifier, const Song& song);
    void onTaskFailed(const QString& identifier, const QString& error);
    void onTaskSkipped(const QString& identifier, const Song& existingSong);

private:
    void setupUI();
    void setupConnections();
    void setupStyles();
    bool validateInput() const;
    void loadDefaultSettings();

    void addTaskToQueue(const QString& identifier);
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

    DownloadViewModel* m_viewModel = nullptr;

    QMap<QString, DownloadTaskItem*> m_taskItems;
};
