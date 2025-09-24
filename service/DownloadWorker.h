#pragma once
#include <QObject>
#include <QRunnable>
#include <QTimer>
#include <QMutex>
#include "../infra/YtDlpClient.h"
#include "../infra/MetadataParser.h"
#include "DownloadTaskState.h"

class DownloadWorker : public QObject, public QRunnable {
    Q_OBJECT

public:
    explicit DownloadWorker(const QString& taskId, const DownloadTaskState& task, QObject* parent = nullptr);
    ~DownloadWorker();

    void run() override;

    QString getTaskId() const { return m_taskId; }
    bool isRunning() const { return m_isRunning; }

    // 取消任务
    void cancel();

signals:
    void taskFinished(const QString& taskId, bool success, const QString& error, const Song& song);
    void taskProgress(const QString& taskId, double progress, const QString& message);

private slots:
    void onDownloadFinished(const YtDlpClient::DownloadResult& result);
    void onDownloadError(const QString& error);
    void onTimeout();

private:
    void setupYtDlpClient();
    void cleanup();
    QString generateFinalFilename(const Song& song, const DownloadOptions& options);
    bool moveAndRenameFile(const QString& tempFilePath, const QString& finalFilePath);
    void cleanupTempFiles(const YtDlpClient::DownloadResult& result);

    QString m_taskId;
    DownloadTaskState m_taskState;
    YtDlpClient* m_ytDlpClient;
    MetadataParser* m_metadataParser;

    bool m_isRunning;
    bool m_isCancelled;

    QTimer* m_timeoutTimer;
    QString m_downloadDir;

    mutable QMutex m_stateMutex;
};
