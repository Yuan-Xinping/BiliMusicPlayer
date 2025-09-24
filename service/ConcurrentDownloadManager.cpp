#include "ConcurrentDownloadManager.h"
#include "DownloadWorker.h"
#include "../common/AppConfig.h"
#include <QThreadPool>
#include <QMutexLocker>
#include <QDebug>

ConcurrentDownloadManager::ConcurrentDownloadManager(QObject* parent)
    : QObject(parent)
    , m_config(ConcurrentDownloadConfig::getDefault())
    , m_isPaused(false)
    , m_processTimer(new QTimer(this))
    , m_timeoutTimer(new QTimer(this))
    , m_statisticsTimer(new QTimer(this))
    , m_songRepository(new SongRepository(this))
{
    // 设置处理定时器
    m_processTimer->setSingleShot(true);
    connect(m_processTimer, &QTimer::timeout, this, &ConcurrentDownloadManager::processQueue);

    // 设置超时检查定时器
    m_timeoutTimer->setInterval(10000); // 每10秒检查一次超时
    connect(m_timeoutTimer, &QTimer::timeout, this, &ConcurrentDownloadManager::checkTimeouts);
    m_timeoutTimer->start();

    // 设置统计更新定时器
    m_statisticsTimer->setInterval(1000); // 每秒更新统计
    connect(m_statisticsTimer, &QTimer::timeout, this, &ConcurrentDownloadManager::updateStatistics);
    m_statisticsTimer->start();

    // 设置线程池
    QThreadPool::globalInstance()->setMaxThreadCount(m_config.maxConcurrentDownloads * 2); // 留一些余量

    qDebug() << "ConcurrentDownloadManager: 初始化完成，最大并发数:" << m_config.maxConcurrentDownloads;
}

ConcurrentDownloadManager::~ConcurrentDownloadManager() {
    cancelAllTasks();

    // 等待所有工作线程完成
    QThreadPool::globalInstance()->waitForDone(30000); // 最多等待30秒
}

void ConcurrentDownloadManager::setConfig(const ConcurrentDownloadConfig& config) {
    if (!config.isValid()) {
        qWarning() << "ConcurrentDownloadManager: 无效的配置";
        return;
    }

    QMutexLocker locker(&m_tasksMutex);
    m_config = config;

    // 更新线程池大小
    QThreadPool::globalInstance()->setMaxThreadCount(m_config.maxConcurrentDownloads * 2);

    qDebug() << "ConcurrentDownloadManager: 更新配置，最大并发数:" << m_config.maxConcurrentDownloads;

    // 如果配置变更后可以启动新任务，则立即处理队列
    if (!m_isPaused && canStartNewTask() && !m_pendingTaskIds.isEmpty()) {
        m_processTimer->start(100);
    }
}

ConcurrentDownloadConfig ConcurrentDownloadManager::getConfig() const {
    QMutexLocker locker(&m_tasksMutex);
    return m_config;
}

QString ConcurrentDownloadManager::addTask(const QString& identifier, const DownloadOptions& options) {
    QMutexLocker locker(&m_tasksMutex);

    // 检查是否已存在相同标识符的任务
    for (const auto& task : m_tasks.values()) {
        if (task.getIdentifier() == identifier && !task.isFinished()) {
            qDebug() << "ConcurrentDownloadManager: 任务已存在:" << identifier;
            return task.getTaskId(); // 返回现有任务ID
        }
    }

    // 检查数据库中是否已存在
    Song existingSong = m_songRepository->findById(identifier);
    if (!existingSong.getId().isEmpty()) {
        qDebug() << "ConcurrentDownloadManager: 歌曲已存在于数据库中，跳过:" << identifier;
        return QString(); // 返回空字符串表示跳过
    }

    // 创建新任务 - 现在必须显式传递两个参数
    DownloadTaskState newTask(identifier, options);
    QString taskId = newTask.getTaskId();

    m_tasks.insert(taskId, newTask);
    m_pendingTaskIds.enqueue(taskId);

    locker.unlock();

    emit taskAdded(taskId, newTask);
    qDebug() << "ConcurrentDownloadManager: 添加任务:" << identifier << "，任务ID:" << taskId;

    // 如果没有暂停，立即尝试处理队列
    if (!m_isPaused) {
        m_processTimer->start(100);
    }

    return taskId;
}


QStringList ConcurrentDownloadManager::addBatchTasks(const QStringList& identifiers, const DownloadOptions& options) {
    QStringList taskIds;

    for (const QString& identifier : identifiers) {
        QString taskId = addTask(identifier, options);
        if (!taskId.isEmpty()) {
            taskIds.append(taskId);
        }
    }

    qDebug() << "ConcurrentDownloadManager: 批量添加任务，总数:" << identifiers.size() << "，实际添加:" << taskIds.size();

    return taskIds;
}

bool ConcurrentDownloadManager::cancelTask(const QString& taskId) {
    QMutexLocker locker(&m_tasksMutex);

    if (!m_tasks.contains(taskId)) {
        qWarning() << "ConcurrentDownloadManager: 找不到任务:" << taskId;
        return false;
    }

    DownloadTaskState& task = m_tasks[taskId];

    if (task.isFinished()) {
        qDebug() << "ConcurrentDownloadManager: 任务已完成，无法取消:" << taskId;
        return false;
    }

    // 如果任务正在运行，取消工作线程
    if (m_activeWorkers.contains(taskId)) {
        DownloadWorker* worker = m_activeWorkers[taskId];
        if (worker) {
            worker->cancel();
        }
    }

    // 从待处理队列中移除
    QQueue<QString> newQueue;
    while (!m_pendingTaskIds.isEmpty()) {
        QString pendingTaskId = m_pendingTaskIds.dequeue();
        if (pendingTaskId != taskId) {
            newQueue.enqueue(pendingTaskId);
        }
    }
    m_pendingTaskIds = newQueue;

    // 更新任务状态
    task.setStatus(DownloadTaskState::Status::Cancelled);

    locker.unlock();

    emit taskCancelled(taskId);
    qDebug() << "ConcurrentDownloadManager: 取消任务:" << taskId;

    return true;
}

void ConcurrentDownloadManager::cancelAllTasks() {
    QMutexLocker locker(&m_tasksMutex);

    qDebug() << "ConcurrentDownloadManager: 取消所有任务";

    // 取消所有运行中的工作线程
    for (auto worker : m_activeWorkers.values()) {
        if (worker) {
            worker->cancel();
        }
    }

    // 清空待处理队列
    m_pendingTaskIds.clear();

    // 更新所有未完成任务的状态
    QStringList cancelledTaskIds;
    for (auto& task : m_tasks) {
        if (!task.isFinished()) {
            task.setStatus(DownloadTaskState::Status::Cancelled);
            cancelledTaskIds.append(task.getTaskId());
        }
    }

    locker.unlock();

    // 发送取消信号
    for (const QString& taskId : cancelledTaskIds) {
        emit taskCancelled(taskId);
    }
}

void ConcurrentDownloadManager::pauseDownloads() {
    QMutexLocker locker(&m_tasksMutex);
    m_isPaused = true;
    locker.unlock();

    qDebug() << "ConcurrentDownloadManager: 暂停下载";
}

void ConcurrentDownloadManager::resumeDownloads() {
    QMutexLocker locker(&m_tasksMutex);
    m_isPaused = false;
    locker.unlock();

    qDebug() << "ConcurrentDownloadManager: 恢复下载";

    // 立即处理队列
    if (!m_pendingTaskIds.isEmpty()) {
        m_processTimer->start(100);
    }
}

bool ConcurrentDownloadManager::isRunning() const {
    QMutexLocker locker(&m_tasksMutex);
    return !m_activeWorkers.isEmpty();
}

bool ConcurrentDownloadManager::isPaused() const {
    QMutexLocker locker(&m_tasksMutex);
    return m_isPaused;
}

int ConcurrentDownloadManager::getActiveTaskCount() const {
    QMutexLocker locker(&m_tasksMutex);
    return m_activeWorkers.size();
}

int ConcurrentDownloadManager::getPendingTaskCount() const {
    QMutexLocker locker(&m_tasksMutex);
    return m_pendingTaskIds.size();
}

int ConcurrentDownloadManager::getCompletedTaskCount() const {
    QMutexLocker locker(&m_tasksMutex);
    return getTasksByStatus(DownloadTaskState::Status::Completed).size();
}

int ConcurrentDownloadManager::getFailedTaskCount() const {
    QMutexLocker locker(&m_tasksMutex);
    return getTasksByStatus(DownloadTaskState::Status::Failed).size();
}

QList<DownloadTaskState> ConcurrentDownloadManager::getAllTasks() const {
    QMutexLocker locker(&m_tasksMutex);
    return m_tasks.values();
}

QList<DownloadTaskState> ConcurrentDownloadManager::getTasksByStatus(DownloadTaskState::Status status) const {
    QMutexLocker locker(&m_tasksMutex);
    QList<DownloadTaskState> result;

    for (const auto& task : m_tasks.values()) {
        if (task.getStatus() == status) {
            result.append(task);
        }
    }

    return result;
}

DownloadTaskState ConcurrentDownloadManager::getTask(const QString& taskId) const {
    QMutexLocker locker(&m_tasksMutex);
    return m_tasks.value(taskId);
}

ConcurrentDownloadManager::Statistics ConcurrentDownloadManager::getStatistics() const {
    QMutexLocker locker(&m_tasksMutex);
    return m_lastStatistics;
}

void ConcurrentDownloadManager::processQueue() {
    if (m_isPaused) {
        return;
    }

    cleanupFinishedWorkers();

    // 启动新任务
    while (canStartNewTask() && !m_pendingTaskIds.isEmpty()) {
        QMutexLocker locker(&m_tasksMutex);

        if (m_pendingTaskIds.isEmpty()) {
            break;
        }

        QString taskId = m_pendingTaskIds.dequeue();

        if (!m_tasks.contains(taskId)) {
            continue; // 任务可能已被删除
        }

        DownloadTaskState& task = m_tasks[taskId];

        if (task.getStatus() != DownloadTaskState::Status::Pending &&
            task.getStatus() != DownloadTaskState::Status::Retrying) {
            continue; // 任务状态不对
        }

        locker.unlock();

        startTask(task);
    }

    // 如果还有待处理任务，继续定期检查
    if (!m_pendingTaskIds.isEmpty() && !m_isPaused) {
        m_processTimer->start(1000); // 1秒后再次检查
    }
    else if (m_activeWorkers.isEmpty() && m_pendingTaskIds.isEmpty()) {
        // 所有任务都完成了
        emit allTasksCompleted();
        qDebug() << "ConcurrentDownloadManager: 所有任务已完成";
    }
}

void ConcurrentDownloadManager::startTask(DownloadTaskState& task) {
    QString taskId = task.getTaskId();

    qDebug() << "ConcurrentDownloadManager: 启动任务:" << taskId;

    // 创建工作线程
    DownloadWorker* worker = new DownloadWorker(taskId, task);

    // 连接信号
    connect(worker, &DownloadWorker::taskFinished,
        this, &ConcurrentDownloadManager::onTaskFinished);
    connect(worker, &DownloadWorker::taskProgress,
        this, &ConcurrentDownloadManager::onTaskProgress);

    // 更新任务状态
    QMutexLocker locker(&m_tasksMutex);
    task.setStatus(DownloadTaskState::Status::Running);
    m_activeWorkers.insert(taskId, worker);
    locker.unlock();

    emit taskStarted(taskId);

    // 启动工作线程
    QThreadPool::globalInstance()->start(worker);
}

void ConcurrentDownloadManager::onTaskFinished(const QString& taskId, bool success, const QString& error, const Song& song) {
    QMutexLocker locker(&m_tasksMutex);

    if (!m_tasks.contains(taskId)) {
        qWarning() << "ConcurrentDownloadManager: 收到未知任务的完成信号:" << taskId;
        return;
    }

    DownloadTaskState& task = m_tasks[taskId];

    // 从活跃工作线程列表中移除
    if (m_activeWorkers.contains(taskId)) {
        DownloadWorker* worker = m_activeWorkers.take(taskId);
        if (worker) {
            worker->deleteLater();
        }
    }

    if (success) {
        task.setStatus(DownloadTaskState::Status::Completed);
        task.setResultSong(song);
        task.setProgress(1.0, "下载完成");

        locker.unlock();

        // 保存到数据库
        if (!m_songRepository->save(song)) {
            qWarning() << "ConcurrentDownloadManager: 保存歌曲到数据库失败:" << song.getTitle();
        }

        emit taskCompleted(taskId, song);
        qDebug() << "ConcurrentDownloadManager: 任务完成:" << taskId;

    }
    else {
        // 检查是否可以重试
        if (task.canRetry(m_config.maxRetryCount) && m_config.enableAutoRetry) {
            task.setStatus(DownloadTaskState::Status::Retrying);
            task.incrementRetryCount();
            task.setErrorMessage(error);

            // 重新加入队列
            m_pendingTaskIds.enqueue(taskId);

            locker.unlock();

            emit taskRetrying(taskId, task.getRetryCount());
            qDebug() << "ConcurrentDownloadManager: 任务重试:" << taskId << "，重试次数:" << task.getRetryCount();

            // 延迟处理重试
            QTimer::singleShot(m_config.retryDelayMs, this, &ConcurrentDownloadManager::processQueue);

        }
        else {
            // 标记为最终失败
            task.setStatus(DownloadTaskState::Status::Failed);
            task.setErrorMessage(error);

            locker.unlock();

            emit taskFailed(taskId, error);
            qDebug() << "ConcurrentDownloadManager: 任务失败:" << taskId << "，错误:" << error;
        }
    }

    // 立即更新统计信息
    updateStatistics();

    // 继续处理队列
    if (!m_isPaused) {
        m_processTimer->start(100);
    }
}

void ConcurrentDownloadManager::onTaskProgress(const QString& taskId, double progress, const QString& message) {
    QMutexLocker locker(&m_tasksMutex);

    if (m_tasks.contains(taskId)) {
        m_tasks[taskId].setProgress(progress, message);
    }

    locker.unlock();

    emit taskProgress(taskId, progress, message);
}

void ConcurrentDownloadManager::checkTimeouts() {
    QMutexLocker locker(&m_tasksMutex);

    QStringList timeoutTaskIds;
    QDateTime currentTime = QDateTime::currentDateTime();

    for (auto& task : m_tasks) {
        if (task.getStatus() == DownloadTaskState::Status::Running) {
            qint64 elapsedMs = task.getElapsedMs();
            if (elapsedMs > m_config.taskTimeoutMs) {
                timeoutTaskIds.append(task.getTaskId());
            }
        }
    }

    locker.unlock();

    // 处理超时任务
    for (const QString& taskId : timeoutTaskIds) {
        qDebug() << "ConcurrentDownloadManager: 检测到超时任务:" << taskId;

        // 取消超时任务
        if (m_activeWorkers.contains(taskId)) {
            DownloadWorker* worker = m_activeWorkers[taskId];
            if (worker) {
                worker->cancel();
            }
        }

        // 任务会在 onTaskFinished 中被标记为失败或重试
    }
}

void ConcurrentDownloadManager::cleanupFinishedWorkers() {
    QMutexLocker locker(&m_tasksMutex);

    QStringList finishedWorkerIds;

    for (auto it = m_activeWorkers.begin(); it != m_activeWorkers.end(); ++it) {
        DownloadWorker* worker = it.value();
        if (worker && !worker->isRunning()) {
            finishedWorkerIds.append(it.key());
        }
    }

    // 清理已完成的工作线程
    for (const QString& taskId : finishedWorkerIds) {
        DownloadWorker* worker = m_activeWorkers.take(taskId);
        if (worker) {
            worker->deleteLater();
        }
    }
}

void ConcurrentDownloadManager::updateStatistics() {
    QMutexLocker locker(&m_tasksMutex);

    Statistics stats;
    stats.totalTasks = m_tasks.size();
    stats.activeTasks = m_activeWorkers.size();
    stats.pendingTasks = m_pendingTaskIds.size();

    qint64 totalDownloadTime = 0;
    int completedTasks = 0;
    int failedTasks = 0;

    for (const auto& task : m_tasks.values()) {
        switch (task.getStatus()) {
        case DownloadTaskState::Status::Completed:
            stats.completedTasks++;
            completedTasks++;
            totalDownloadTime += task.getElapsedMs();
            break;
        case DownloadTaskState::Status::Failed:
        case DownloadTaskState::Status::Timeout:
            stats.failedTasks++;
            failedTasks++;
            break;
        default:
            break;
        }
    }

    stats.totalDownloadTimeMs = totalDownloadTime;
    stats.averageDownloadTimeMs = completedTasks > 0 ? totalDownloadTime / completedTasks : 0;
    stats.overallProgress = stats.totalTasks > 0 ? (double)stats.completedTasks / stats.totalTasks : 0.0;

    // 确保统计数据的一致性
    if (stats.completedTasks != completedTasks) {
        qDebug() << "统计数据不一致，修正中...";
        stats.completedTasks = completedTasks;
        stats.failedTasks = failedTasks;
    }

    m_lastStatistics = stats;

    locker.unlock();

    emit statisticsUpdated(stats);
}

bool ConcurrentDownloadManager::canStartNewTask() const {
    // 注意：调用者应该已经持有锁
    return m_activeWorkers.size() < m_config.maxConcurrentDownloads;
}