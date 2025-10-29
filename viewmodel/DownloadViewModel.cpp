// viewmodel/DownloadViewModel.cpp
#include "DownloadViewModel.h"
#include "../common/AppConfig.h"
#include "../service/ConcurrentDownloadManager.h"
#include "../service/DownloadTaskState.h"
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

    // 连接 Service 信号（串行下载）
    connectServiceSignals();

    // 连接并行下载管理器信号
    connectConcurrentSignals();

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

int DownloadViewModel::addConcurrentBatchTasks(const QStringList& identifiers, const QString& preset)
{
    // 清洗输入
    QStringList ids;
    ids.reserve(identifiers.size());
    for (const QString& s : identifiers) {
        const QString t = s.trimmed();
        if (!t.isEmpty()) ids << t;
    }
    if (ids.isEmpty()) {
        emit errorOccurred("批量下载", "没有有效的 BV 号或 URL");
        return 0;
    }

    // 直接提交到并行下载管理器（使用其默认/全局配置）
    auto& cdm = ConcurrentDownloadManager::instance();
    const QStringList tids = cdm.addBatchTasks(ids /* , DownloadOptions 可按需传入 */);

    // 刷新状态栏显示
    updateStatusText();
    return tids.size();
}

// 并行下载事件 -> 统一转发为 UI 信号
void DownloadViewModel::connectConcurrentSignals()
{
    auto& cdm = ConcurrentDownloadManager::instance();

    // 第一次见到某个 identifier 时，先补发 taskAdded 让 UI 建立条目
    auto ensureAdded = [this](const QString& id) {
        if (id.isEmpty()) return;
        if (!m_taskCache.contains(id)) {
            TaskInfo info;
            info.identifier = id;
            info.title = id;
            info.status = "等待中...";
            info.progress = 0.0;
            m_taskCache[id] = info;
            emit taskAdded(id);
            qDebug() << "VM: ensureAdded ->" << id;
        }
        };

    connect(&cdm, &ConcurrentDownloadManager::taskAdded, this,
        [this, ensureAdded](const QString& /*taskId*/, const DownloadTaskState& task) {
            const QString id = task.getIdentifier();
            ensureAdded(id);
            updateStatusText();
        });

    connect(&cdm, &ConcurrentDownloadManager::taskStarted, this,
        [this, &cdm, ensureAdded](const QString& taskId) {
            const QString id = cdm.getTask(taskId).getIdentifier();
            ensureAdded(id);
            if (m_taskCache.contains(id)) m_taskCache[id].status = "下载中...";
            emit taskStarted(id);
            updateStatusText();
        });

    connect(&cdm, &ConcurrentDownloadManager::taskProgress, this,
        [this, &cdm, ensureAdded](const QString& taskId, double progress, const QString& message) {
            const QString id = cdm.getTask(taskId).getIdentifier();
            ensureAdded(id);
            if (m_taskCache.contains(id)) {
                m_taskCache[id].progress = progress;
                m_taskCache[id].status = message;
            }
            emit taskProgressUpdated(id, progress, message);
            updateStatusText();
        });

    connect(&cdm, &ConcurrentDownloadManager::taskCompleted, this,
        [this, &cdm, ensureAdded](const QString& taskId, const Song& song) {
            QString id = song.getId();
            if (id.isEmpty()) id = cdm.getTask(taskId).getIdentifier();
            ensureAdded(id);
            if (m_taskCache.contains(id)) {
                m_taskCache[id].title = song.getTitle();
                m_taskCache[id].status = "✅ 完成";
                m_taskCache[id].progress = 1.0;
            }
            emit taskCompleted(id, song);
            updateStatusText();
        });

    connect(&cdm, &ConcurrentDownloadManager::taskFailed, this,
        [this, &cdm, ensureAdded](const QString& taskId, const QString& error) {
            const QString id = cdm.getTask(taskId).getIdentifier();
            ensureAdded(id);
            if (m_taskCache.contains(id)) {
                m_taskCache[id].status = "❌ 失败";
                m_taskCache[id].errorMessage = error;
            }
            emit taskFailed(id, error);
            updateStatusText();
        });

    connect(&cdm, &ConcurrentDownloadManager::statisticsUpdated, this,
        [this](const ConcurrentDownloadManager::Statistics&) {
            updateStatusText();
        });

    connect(&cdm, &ConcurrentDownloadManager::allTasksCompleted, this,
        [this]() {
            allTasksCompleted();
            updateStatusText();
        });
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

// === Service 信号处理（串行下载） ===

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

// 页面打开/切换到“下载队列”时调用，补建并行任务的 UI 条目
void DownloadViewModel::syncConcurrentTasksToUI()
{
    auto& cdm = ConcurrentDownloadManager::instance();
    const auto tasks = cdm.getAllTasks();

    for (const auto& t : tasks) {
        const QString id = t.getIdentifier();
        if (id.isEmpty()) continue;

        if (!m_taskCache.contains(id)) {
            TaskInfo info;
            info.identifier = id;
            info.title = id;
            info.status = "等待中...";
            info.progress = 0.0;
            m_taskCache[id] = info;
            emit taskAdded(id); // 让 UI 建条目
            qDebug() << "VM: syncConcurrentTasksToUI -> add" << id;
        }

        // 尝试同步状态（可选）
        if (t.getStatus() == DownloadTaskState::Status::Running) {
            m_taskCache[id].status = "下载中...";
            emit taskStarted(id);
        }
        else if (t.getStatus() == DownloadTaskState::Status::Retrying
            || t.getStatus() == DownloadTaskState::Status::Pending) {
            m_taskCache[id].status = "等待中...";
        }
        // 若 DownloadTaskState 暴露进度，则可取消注释同步当前进度文本
        // double p = t.getProgress();
        // QString msg = t.getProgressMessage();
        // if (p > 0.0) {
        //     m_taskCache[id].progress = p;
        //     if (!msg.isEmpty()) m_taskCache[id].status = msg;
        //     emit taskProgressUpdated(id, p, m_taskCache[id].status);
        // }
    }

    updateStatusText();
}

void DownloadViewModel::updateStatusText()
{
    auto& cdm = ConcurrentDownloadManager::instance();

    const int pending = m_service->getQueueSize() + cdm.getPendingTaskCount();
    const int completed = m_service->getCompletedCount() + cdm.getCompletedTaskCount();
    const bool downloading = m_service->isDownloading() || cdm.getActiveTaskCount() > 0;

    QString newText;
    QString downloadPath = getDownloadPath();

    if (downloading) {
        newText = QString("⏬ 下载中... | 队列: %1 | 完成: %2 | 📁 %3")
            .arg(pending)
            .arg(completed)
            .arg(downloadPath);
    }
    else if (pending > 0) {
        newText = QString("⏸️ 已暂停 | 队列: %1 | 📁 %2")
            .arg(pending)
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
