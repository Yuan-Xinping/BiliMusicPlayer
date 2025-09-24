#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QUuid>
#include "../infra/DownloadConfig.h"
#include "../common/entities/Song.h"

class DownloadTaskState {
public:
    enum class Status {
        Pending,        // 等待中
        Running,        // 运行中
        Completed,      // 已完成
        Failed,         // 失败
        Retrying,       // 重试中
        Cancelled,      // 已取消
        Timeout         // 超时
    };

    // 默认构造函数 - 为了支持 QHash
    DownloadTaskState()
        : m_taskId(QUuid::createUuid().toString())
        , m_identifier("")
        , m_options(DownloadOptions::createPreset("high_quality_mp3"))
        , m_status(Status::Pending)
        , m_progress(0.0)
        , m_retryCount(0)
        , m_createdTime(QDateTime::currentDateTime())
    {
    }

    // 带参数的构造函数 - 移除默认参数，避免与上面的构造函数冲突
    DownloadTaskState(const QString& identifier, const DownloadOptions& options)
        : m_taskId(QUuid::createUuid().toString())
        , m_identifier(identifier)
        , m_options(options)
        , m_status(Status::Pending)
        , m_progress(0.0)
        , m_retryCount(0)
        , m_createdTime(QDateTime::currentDateTime())
    {
    }

    // 拷贝构造函数
    DownloadTaskState(const DownloadTaskState& other)
        : m_taskId(other.m_taskId)
        , m_identifier(other.m_identifier)
        , m_options(other.m_options)
        , m_status(other.m_status)
        , m_progress(other.m_progress)
        , m_currentMessage(other.m_currentMessage)
        , m_errorMessage(other.m_errorMessage)
        , m_retryCount(other.m_retryCount)
        , m_createdTime(other.m_createdTime)
        , m_startTime(other.m_startTime)
        , m_finishTime(other.m_finishTime)
        , m_resultSong(other.m_resultSong)
    {
    }

    // 赋值运算符
    DownloadTaskState& operator=(const DownloadTaskState& other) {
        if (this != &other) {
            m_taskId = other.m_taskId;
            m_identifier = other.m_identifier;
            m_options = other.m_options;
            m_status = other.m_status;
            m_progress = other.m_progress;
            m_currentMessage = other.m_currentMessage;
            m_errorMessage = other.m_errorMessage;
            m_retryCount = other.m_retryCount;
            m_createdTime = other.m_createdTime;
            m_startTime = other.m_startTime;
            m_finishTime = other.m_finishTime;
            m_resultSong = other.m_resultSong;
        }
        return *this;
    }

    // Getters
    QString getTaskId() const { return m_taskId; }
    QString getIdentifier() const { return m_identifier; }
    DownloadOptions getOptions() const { return m_options; }
    Status getStatus() const { return m_status; }
    double getProgress() const { return m_progress; }
    QString getCurrentMessage() const { return m_currentMessage; }
    QString getErrorMessage() const { return m_errorMessage; }
    int getRetryCount() const { return m_retryCount; }
    QDateTime getCreatedTime() const { return m_createdTime; }
    QDateTime getStartTime() const { return m_startTime; }
    QDateTime getFinishTime() const { return m_finishTime; }
    Song getResultSong() const { return m_resultSong; }

    // Setters
    void setStatus(Status status) {
        m_status = status;
        if (status == Status::Running && m_startTime.isNull()) {
            m_startTime = QDateTime::currentDateTime();
        }
        else if ((status == Status::Completed || status == Status::Failed || status == Status::Cancelled) && m_finishTime.isNull()) {
            m_finishTime = QDateTime::currentDateTime();
        }
    }

    void setProgress(double progress, const QString& message = QString()) {
        m_progress = progress;
        if (!message.isEmpty()) {
            m_currentMessage = message;
        }
    }

    void setErrorMessage(const QString& error) { m_errorMessage = error; }
    void incrementRetryCount() { m_retryCount++; }
    void setResultSong(const Song& song) { m_resultSong = song; }

    // 工具方法
    bool canRetry(int maxRetryCount) const {
        return m_retryCount < maxRetryCount &&
            (m_status == Status::Failed || m_status == Status::Timeout);
    }

    bool isFinished() const {
        return m_status == Status::Completed ||
            m_status == Status::Cancelled ||
            (m_status == Status::Failed && !canRetry(3));
    }

    qint64 getElapsedMs() const {
        if (m_startTime.isNull()) return 0;

        QDateTime endTime = m_finishTime.isNull() ? QDateTime::currentDateTime() : m_finishTime;
        return m_startTime.msecsTo(endTime);
    }

    QString getStatusString() const {
        switch (m_status) {
        case Status::Pending: return "等待中";
        case Status::Running: return "下载中";
        case Status::Completed: return "已完成";
        case Status::Failed: return "失败";
        case Status::Retrying: return "重试中";
        case Status::Cancelled: return "已取消";
        case Status::Timeout: return "超时";
        }
        return "未知";
    }

    // 用于调试的方法
    bool isValid() const {
        return !m_taskId.isEmpty();
    }

private:
    QString m_taskId;
    QString m_identifier;
    DownloadOptions m_options;
    Status m_status;
    double m_progress;
    QString m_currentMessage;
    QString m_errorMessage;
    int m_retryCount;
    QDateTime m_createdTime;
    QDateTime m_startTime;
    QDateTime m_finishTime;
    Song m_resultSong;
};
