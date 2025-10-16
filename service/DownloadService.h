#pragma once
#include <QObject>
#include <QQueue>
#include <QTimer>
#include "../infra/YtDlpClient.h"
#include "../infra/MetadataParser.h"
#include "../data/SongRepository.h"
#include "../common/entities/Song.h"

class DownloadService : public QObject {
    Q_OBJECT

public:
    enum class DownloadStatus {
        Idle,
        Downloading,
        Processing,
        Completed,
        Failed,
        Cancelled
    };

    struct DownloadTask {
        QString identifier;  // BV号或URL
        QString outputDir;
        DownloadOptions options;
        DownloadStatus status = DownloadStatus::Idle;
        QString errorMessage;
        Song resultSong;
    };

    explicit DownloadService(QObject* parent = nullptr);
    ~DownloadService();

    // 添加下载任务
    void addDownloadTask(const QString& identifier, const QString& preset = "high_quality_mp3");
    void addDownloadTask(const QString& identifier, const DownloadOptions& options);

    // 批量添加任务
    void addBatchDownloadTasks(const QStringList& identifiers, const QString& preset = "high_quality_mp3");

    // 控制下载
    void startDownload();
    void pauseDownload();
    void cancelDownload();
    void cancelAllTasks();

    // 状态查询
    bool isDownloading() const;
    int getQueueSize() const;
    int getCompletedCount() const;
    int getFailedCount() const;

    // 获取当前任务
    DownloadTask getCurrentTask() const;
    QList<DownloadTask> getAllTasks() const;

signals:
    void taskAdded(const DownloadTask& task);
    void taskStarted(const DownloadTask& task);
    void taskProgress(const DownloadTask& task, double progress, const QString& message);
    void taskCompleted(const DownloadTask& task, const Song& song);
    void taskFailed(const DownloadTask& task, const QString& error);
    void taskCancelled(const DownloadTask& task);

    void allTasksCompleted();
    void downloadServiceError(const QString& error);
    void taskSkipped(const QString& identifier, const Song& existingSong);

private slots:
    void onDownloadFinished(const YtDlpClient::DownloadResult& result);
    void onDownloadError(const QString& error);
    void processNextTask();

private:
    void processCurrentTask();
    QString generateFinalFilename(const Song& song, const DownloadOptions& options);
    bool moveAndRenameFile(const QString& tempFilePath, const QString& finalFilePath);
    void cleanupTempFiles(const YtDlpClient::DownloadResult& result);
    void completeCurrentTask(const Song& song);
    void failCurrentTask(const QString& error);

    YtDlpClient* m_ytDlpClient;
    MetadataParser* m_metadataParser;
    SongRepository* m_songRepository;

    QQueue<DownloadTask> m_taskQueue;
    DownloadTask m_currentTask;
    bool m_isDownloading;
    bool m_isPaused;

    int m_completedCount;
    int m_failedCount;

    QString m_downloadDir;
    QTimer* m_processTimer;
};
