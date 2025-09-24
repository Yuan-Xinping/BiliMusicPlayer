#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include "ProcessRunner.h"
#include "DownloadConfig.h"
#include "../common/entities/Song.h"

class YtDlpClient : public QObject {
    Q_OBJECT

public:
    struct DownloadResult {
        bool success;
        QString errorMessage;
        QString tempInfoJsonPath;
        QString tempAudioFilePath;
        QString videoId;
        DownloadOptions usedOptions; //记录使用的下载选项
    };

    explicit YtDlpClient(QObject* parent = nullptr);
    ~YtDlpClient();

    // 保持向后兼容
    void downloadAudio(const QString& identifier, const QString& outputDir);

    // 使用自定义配置（重载）
    void downloadAudio(const QString& identifier, const QString& outputDir, const DownloadOptions& options);

    // 视频下载
    void downloadVideo(const QString& identifier, const QString& outputDir, const DownloadOptions& options);

    // 使用配置对象
    void downloadWithOptions(const QString& identifier, const QString& outputDir, const DownloadOptions& options);

    // 使用预设配置
    void downloadAudioWithPreset(const QString& identifier, const QString& outputDir, const QString& preset = "high_quality_mp3");
    void downloadVideoWithPreset(const QString& identifier, const QString& outputDir, const QString& preset = "video_720p");

    // 设置进度回调
    void setProgressCallback(const ProcessRunner::ProgressCallback& callback);

    // 取消下载
    void cancel();

    bool isRunning() const;

    // 静态方法
    static QStringList getAvailablePresets();
    static QString getPresetDescription(const QString& preset);

signals:
    void downloadFinished(const DownloadResult& result);
    void downloadProgress(double progress, const QString& message);
    void downloadError(const QString& error);

private slots:
    void onProcessFinished(int exitCode);
    void onProcessError(const QString& errorString);
    void onProcessOutput(const QString& line);

private:
    ProcessRunner* m_processRunner;
    QString m_currentVideoId;
    QString m_currentOutputDir;
    QString m_tempInfoJsonPath;
    QString m_tempAudioFilePath;
    DownloadOptions m_currentOptions;

    QStringList buildYtDlpArguments(const QString& identifier, const QString& outputTemplate, const DownloadOptions& options);
    QString extractVideoId(const QString& identifier);
    void parseOutputLine(const QString& line);
    QString getYtDlpPath();
    QString generateOutputTemplate(const QString& outputDir, const QString& videoId, const DownloadOptions& options);
    void startDownload(const QString& identifier, const QString& outputDir, const DownloadOptions& options);
};
