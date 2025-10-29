// viewmodel/DownloadViewModel.h
#pragma once
#include <QObject>
#include <QMap>
#include "../service/DownloadService.h"
#include "../common/entities/Song.h"

class DownloadViewModel : public QObject {
    Q_OBJECT
        Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
        Q_PROPERTY(int queueSize READ queueSize NOTIFY queueSizeChanged)
        Q_PROPERTY(int completedCount READ completedCount NOTIFY completedCountChanged)
        Q_PROPERTY(QString currentQualityPreset READ currentQualityPreset WRITE setCurrentQualityPreset NOTIFY currentQualityPresetChanged)

public:
    // 下载任务信息（UI 友好的数据结构）
    struct TaskInfo {
        QString identifier;
        QString title;
        QString status;
        double progress;
        QString errorMessage;
    };

    explicit DownloadViewModel(DownloadService* service, QObject* parent = nullptr);
    ~DownloadViewModel() override = default;

    // === 属性访问器 ===
    QString statusText() const { return m_statusText; }
    int queueSize() const { return m_queueSize; }
    int completedCount() const { return m_completedCount; }
    QString currentQualityPreset() const { return m_currentQualityPreset; }

    void setCurrentQualityPreset(const QString& preset);

    // === 业务操作 ===
    Q_INVOKABLE void addDownloadTask(const QString& identifier);
    Q_INVOKABLE void addDownloadTaskWithPreset(const QString& identifier, const QString& preset);
    Q_INVOKABLE void startDownload();
    Q_INVOKABLE void pauseDownload();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void cancelAllTasks();
    Q_INVOKABLE void openDownloadFolder();
    Q_INVOKABLE int addConcurrentBatchTasks(const QStringList& identifiers, const QString& preset);

    // === 查询方法 ===
    Q_INVOKABLE bool isDownloading() const;
    Q_INVOKABLE QList<TaskInfo> getAllTasks() const;
    Q_INVOKABLE TaskInfo getTaskInfo(const QString& identifier) const;

    // 配置相关
    Q_INVOKABLE void refreshConfig();
    Q_INVOKABLE QString getDownloadPath() const;

    // 页面打开或切到“下载队列”Tab时，用它补建并行任务的UI条目
    Q_INVOKABLE void syncConcurrentTasksToUI();

signals:
    // UI 状态信号
    void statusTextChanged();
    void queueSizeChanged();
    void completedCountChanged();
    void currentQualityPresetChanged();

    // 任务状态信号（供 UI 订阅）
    void taskAdded(const QString& identifier);
    void taskStarted(const QString& identifier);
    void taskProgressUpdated(const QString& identifier, double progress, const QString& message);
    void taskCompleted(const QString& identifier, const Song& song);
    void taskFailed(const QString& identifier, const QString& error);
    void taskSkipped(const QString& identifier, const Song& existingSong);

    // 批量操作信号
    void allTasksCompleted();

    // 错误信号
    void errorOccurred(const QString& title, const QString& message);

private slots:
    // 连接 Service 层信号
    void onServiceTaskAdded(const DownloadService::DownloadTask& task);
    void onServiceTaskStarted(const DownloadService::DownloadTask& task);
    void onServiceTaskProgress(const DownloadService::DownloadTask& task, double progress, const QString& message);
    void onServiceTaskCompleted(const DownloadService::DownloadTask& task, const Song& song);
    void onServiceTaskFailed(const DownloadService::DownloadTask& task, const QString& error);
    void onServiceTaskSkipped(const QString& identifier, const Song& existingSong);

private:
    void updateStatusText();
    void connectServiceSignals();
    // 订阅并行下载管理器的事件并转发给 UI
    void connectConcurrentSignals();

    // Service 层引用
    DownloadService* m_service;

    // ViewModel 状态
    QString m_statusText;
    int m_queueSize = 0;
    int m_completedCount = 0;
    QString m_currentQualityPreset;

    // 任务缓存（用于快速查询）
    QMap<QString, TaskInfo> m_taskCache;
};
