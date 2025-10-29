// service/ConcurrentDownloadManager.h
#pragma once
#include <QObject>
#include <QQueue>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QThreadPool>
#include <QMutexLocker>
#include "DownloadTaskState.h"
#include "ConcurrentDownloadConfig.h"
#include "../infra/YtDlpClient.h"
#include "../infra/MetadataParser.h"
#include "../data/SongRepository.h"

class DownloadWorker; // 前向声明

class ConcurrentDownloadManager : public QObject {
    Q_OBJECT

public:
    explicit ConcurrentDownloadManager(QObject* parent = nullptr);
    ~ConcurrentDownloadManager();

    // 🆕 全局单例（供全局复用）
    static ConcurrentDownloadManager& instance();
    ConcurrentDownloadManager(const ConcurrentDownloadManager&) = delete;
    ConcurrentDownloadManager& operator=(const ConcurrentDownloadManager&) = delete;

    // 配置管理
    void setConfig(const ConcurrentDownloadConfig& config);
    ConcurrentDownloadConfig getConfig() const;

    // 任务管理
    QString addTask(const QString& identifier, const DownloadOptions& options = DownloadOptions::createPreset("high_quality_mp3"));
    QStringList addBatchTasks(const QStringList& identifiers, const DownloadOptions& options = DownloadOptions::createPreset("high_quality_mp3"));

    bool cancelTask(const QString& taskId);
    void cancelAllTasks();
    void pauseDownloads();
    void resumeDownloads();

    // 状态查询
    bool isRunning() const;
    bool isPaused() const;
    int getActiveTaskCount() const;
    int getPendingTaskCount() const;
    int getCompletedTaskCount() const;
    int getFailedTaskCount() const;

    // 任务信息
    QList<DownloadTaskState> getAllTasks() const;
    QList<DownloadTaskState> getTasksByStatus(DownloadTaskState::Status status) const;
    DownloadTaskState getTask(const QString& taskId) const;

    // 统计信息
    struct Statistics {
        int totalTasks = 0;
        int completedTasks = 0;
        int failedTasks = 0;
        int activeTasks = 0;
        int pendingTasks = 0;
        double overallProgress = 0.0;
        qint64 totalDownloadTimeMs = 0;
        qint64 averageDownloadTimeMs = 0;
    };

    Statistics getStatistics() const;

signals:
    void taskAdded(const QString& taskId, const DownloadTaskState& task);
    void taskStarted(const QString& taskId);
    void taskProgress(const QString& taskId, double progress, const QString& message);
    void taskCompleted(const QString& taskId, const Song& song);
    void taskFailed(const QString& taskId, const QString& error);
    void taskRetrying(const QString& taskId, int retryCount);
    void taskCancelled(const QString& taskId);

    void allTasksCompleted();
    void statisticsUpdated(const Statistics& stats);
    void managerError(const QString& error);

private slots:
    void onTaskFinished(const QString& taskId, bool success, const QString& error = QString(), const Song& song = Song());
    void onTaskProgress(const QString& taskId, double progress, const QString& message);
    void processQueue();
    void checkTimeouts();

private:
    void startTask(DownloadTaskState& task);
    void retryTask(DownloadTaskState& task);
    void cleanupFinishedWorkers();
    void updateStatistics();
    bool canStartNewTask() const;

    ConcurrentDownloadConfig m_config;
    QHash<QString, DownloadTaskState> m_tasks; // taskId -> task
    QQueue<QString> m_pendingTaskIds;
    QHash<QString, DownloadWorker*> m_activeWorkers; // taskId -> worker

    mutable QMutex m_tasksMutex;
    bool m_isPaused;

    QTimer* m_processTimer;
    QTimer* m_timeoutTimer;
    QTimer* m_statisticsTimer;

    SongRepository* m_songRepository;

    Statistics m_lastStatistics;
};
