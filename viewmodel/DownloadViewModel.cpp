#include "DownloadViewModel.h"
#include "../common/AppConfig.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

DownloadViewModel::DownloadViewModel(DownloadService* service, QObject* parent)
    : QObject(parent)
    , m_service(service)
{
    Q_ASSERT(m_service != nullptr);

    // 加载默认配置
    m_currentQualityPreset = AppConfig::instance().getDefaultQualityPreset();

    // 连接 Service 信号
    connectServiceSignals();

    // 初始化状态
    updateStatusText();

    qDebug() << "✅ DownloadViewModel 已初始化";
    qDebug() << "  - 默认音质预设:" << m_currentQualityPreset;
    qDebug() << "  - 下载路径:" << getDownloadPath();
}

void DownloadViewModel::connectServiceSignals()
{
    connect(m_service, &DownloadService::taskAdded,
        this, &DownloadViewModel::onServiceTaskAdded);
    connect(m_service, &DownloadService::taskStarted,
        this, &DownloadViewModel::onServiceTaskStarted);
    connect(m_service, &DownloadService::taskProgress,
        this, &DownloadViewModel::onServiceTaskProgress);
    connect(m_service, &DownloadService::taskCompleted,
        this, &DownloadViewModel::onServiceTaskCompleted);
    connect(m_service, &DownloadService::taskFailed,
        this, &DownloadViewModel::onServiceTaskFailed);
    connect(m_service, &DownloadService::taskSkipped,
        this, &DownloadViewModel::onServiceTaskSkipped);
    connect(m_service, &DownloadService::allTasksCompleted,
        this, &DownloadViewModel::allTasksCompleted);
}

void DownloadViewModel::setCurrentQualityPreset(const QString& preset)
{
    if (m_currentQualityPreset != preset) {
        m_currentQualityPreset = preset;
        emit currentQualityPresetChanged();
        qDebug() << "ViewModel: 切换音质预设 ->" << preset;
    }
}

void DownloadViewModel::addDownloadTask(const QString& identifier)
{
    addDownloadTaskWithPreset(identifier, m_currentQualityPreset);
}

void DownloadViewModel::addDownloadTaskWithPreset(const QString& identifier, const QString& preset)
{
    if (identifier.trimmed().isEmpty()) {
        emit errorOccurred("输入错误", "请输入有效的 BV 号或 URL");
        return;
    }

    qDebug() << "ViewModel: 添加下载任务" << identifier << "音质:" << preset;
    m_service->addDownloadTask(identifier, preset);
}

void DownloadViewModel::startDownload()
{
    m_service->startDownload();
}

void DownloadViewModel::pauseDownload()
{
    m_service->pauseDownload();
}

void DownloadViewModel::cancelDownload()
{
    m_service->cancelDownload();
}

void DownloadViewModel::cancelAllTasks()
{
    m_service->cancelAllTasks();
    m_taskCache.clear();
    m_queueSize = 0;
    m_completedCount = 0;
    emit queueSizeChanged();
    emit completedCountChanged();
    updateStatusText();
}

void DownloadViewModel::openDownloadFolder()
{
    QString path = getDownloadPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool DownloadViewModel::isDownloading() const
{
    return m_service->isDownloading();
}

QList<DownloadViewModel::TaskInfo> DownloadViewModel::getAllTasks() const
{
    return m_taskCache.values();
}

DownloadViewModel::TaskInfo DownloadViewModel::getTaskInfo(const QString& identifier) const
{
    return m_taskCache.value(identifier);
}

void DownloadViewModel::refreshConfig()
{
    m_currentQualityPreset = AppConfig::instance().getDefaultQualityPreset();
    m_service->refreshConfig();

    emit currentQualityPresetChanged();
    updateStatusText();

    qDebug() << "✅ ViewModel: 配置已刷新";
    qDebug() << "  - 音质预设:" << m_currentQualityPreset;
    qDebug() << "  - 下载路径:" << getDownloadPath();
}

QString DownloadViewModel::getDownloadPath() const
{
    return AppConfig::instance().getDownloadPath();
}

// === Service 信号处理 ===

void DownloadViewModel::onServiceTaskAdded(const DownloadService::DownloadTask& task)
{
    TaskInfo info;
    info.identifier = task.identifier;
    info.title = task.identifier; // 初始标题
    info.status = "等待中...";
    info.progress = 0.0;

    m_taskCache[task.identifier] = info;
    m_queueSize = m_service->getQueueSize();

    emit taskAdded(task.identifier);
    emit queueSizeChanged();
    updateStatusText();
}

void DownloadViewModel::onServiceTaskStarted(const DownloadService::DownloadTask& task)
{
    if (m_taskCache.contains(task.identifier)) {
        m_taskCache[task.identifier].status = "下载中...";
    }

    emit taskStarted(task.identifier);
    updateStatusText();
}

void DownloadViewModel::onServiceTaskProgress(
    const DownloadService::DownloadTask& task,
    double progress,
    const QString& message)
{
    if (m_taskCache.contains(task.identifier)) {
        m_taskCache[task.identifier].progress = progress;
        m_taskCache[task.identifier].status = message;
    }

    emit taskProgressUpdated(task.identifier, progress, message);
}

void DownloadViewModel::onServiceTaskCompleted(
    const DownloadService::DownloadTask& task,
    const Song& song)
{
    if (m_taskCache.contains(task.identifier)) {
        m_taskCache[task.identifier].title = song.getTitle();
        m_taskCache[task.identifier].status = "✅ 完成";
        m_taskCache[task.identifier].progress = 100.0;
    }

    m_completedCount = m_service->getCompletedCount();
    m_queueSize = m_service->getQueueSize();

    emit taskCompleted(task.identifier, song);
    emit completedCountChanged();
    emit queueSizeChanged();
    updateStatusText();
}

void DownloadViewModel::onServiceTaskFailed(
    const DownloadService::DownloadTask& task,
    const QString& error)
{
    if (m_taskCache.contains(task.identifier)) {
        m_taskCache[task.identifier].status = "❌ 失败";
        m_taskCache[task.identifier].errorMessage = error;
    }

    m_queueSize = m_service->getQueueSize();

    emit taskFailed(task.identifier, error);
    emit queueSizeChanged();
    updateStatusText();
}

void DownloadViewModel::onServiceTaskSkipped(const QString& identifier, const Song& existingSong)
{
    emit taskSkipped(identifier, existingSong);
    updateStatusText();
}

void DownloadViewModel::updateStatusText()
{
    QString newText;
    QString downloadPath = getDownloadPath();

    if (isDownloading()) {
        newText = QString("⏬ 下载中... | 队列: %1 | 完成: %2 | 📁 %3")
            .arg(m_queueSize)
            .arg(m_completedCount)
            .arg(downloadPath);
    }
    else if (m_queueSize > 0) {
        newText = QString("⏸️ 已暂停 | 队列: %1 | 📁 %2")
            .arg(m_queueSize)
            .arg(downloadPath);
    }
    else {
        newText = QString("💤 等待任务 | 📁 %1").arg(downloadPath);
    }

    if (m_statusText != newText) {
        m_statusText = newText;
        emit statusTextChanged();
    }
}
