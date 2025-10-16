#include "DownloadService.h"
#include "../common/AppConfig.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

DownloadService::DownloadService(QObject* parent)
    : QObject(parent)
    , m_ytDlpClient(new YtDlpClient(this))
    , m_metadataParser(new MetadataParser(this))
    , m_songRepository(new SongRepository(this))
    , m_isDownloading(false)
    , m_isPaused(false)
    , m_completedCount(0)
    , m_failedCount(0)
    , m_processTimer(new QTimer(this))
{
    m_downloadDir = AppConfig::instance().getDownloadPath();

    // 确保下载目录存在
    QDir().mkpath(m_downloadDir);

    // 连接 YtDlpClient 信号
    connect(m_ytDlpClient, &YtDlpClient::downloadFinished,
        this, &DownloadService::onDownloadFinished);
    connect(m_ytDlpClient, &YtDlpClient::downloadError,
        this, &DownloadService::onDownloadError);

    // 设置进度回调
    m_ytDlpClient->setProgressCallback([this](double progress, const QString& message) {
        if (!m_taskQueue.isEmpty() || m_currentTask.status != DownloadStatus::Idle) {
            DownloadTask task = m_currentTask;
            emit taskProgress(task, progress, message);
        }
        });

    // 设置处理定时器
    m_processTimer->setSingleShot(true);
    connect(m_processTimer, &QTimer::timeout, this, &DownloadService::processNextTask);

    qDebug() << "DownloadService: 初始化完成，下载目录:" << m_downloadDir;
}

DownloadService::~DownloadService() {
    cancelAllTasks();
}

void DownloadService::addDownloadTask(const QString& identifier, const QString& preset) {
    DownloadOptions options = DownloadOptions::createPreset(preset);
    addDownloadTask(identifier, options);
}

void DownloadService::addDownloadTask(const QString& identifier, const DownloadOptions& options) {
    // 检查是否已存在
    Song existingSong = m_songRepository->findById(identifier);
    if (!existingSong.getId().isEmpty()) {
        qDebug() << "DownloadService: 歌曲已存在，跳过:" << identifier;
        emit taskSkipped(identifier, existingSong);
        return;
    }

    DownloadTask task;
    task.identifier = identifier;
    task.outputDir = m_downloadDir;
    task.options = options;
    task.status = DownloadStatus::Idle;

    m_taskQueue.enqueue(task);
    emit taskAdded(task);

    qDebug() << "DownloadService: 添加下载任务:" << identifier;

    // 如果当前没有在下载，立即开始处理
    if (!m_isDownloading && !m_isPaused) {
        m_processTimer->start(100); // 延迟100ms开始处理
    }
}

void DownloadService::addBatchDownloadTasks(const QStringList& identifiers, const QString& preset) {
    for (const QString& identifier : identifiers) {
        addDownloadTask(identifier, preset);
    }
}

void DownloadService::startDownload() {
    if (m_isPaused) {
        m_isPaused = false;
        qDebug() << "DownloadService: 恢复下载";

        if (!m_isDownloading && !m_taskQueue.isEmpty()) {
            m_processTimer->start(100);
        }
    }
}

void DownloadService::pauseDownload() {
    m_isPaused = true;
    qDebug() << "DownloadService: 暂停下载";
}

void DownloadService::cancelDownload() {
    if (m_isDownloading && m_ytDlpClient->isRunning()) {
        m_ytDlpClient->cancel();
    }
}

void DownloadService::cancelAllTasks() {
    qDebug() << "DownloadService: 取消所有任务";

    cancelDownload();

    // 标记所有待处理任务为已取消
    while (!m_taskQueue.isEmpty()) {
        DownloadTask task = m_taskQueue.dequeue();
        task.status = DownloadStatus::Cancelled;
        emit taskCancelled(task);
    }

    m_isDownloading = false;
    m_isPaused = false;
}

bool DownloadService::isDownloading() const {
    return m_isDownloading;
}

int DownloadService::getQueueSize() const {
    return m_taskQueue.size();
}

int DownloadService::getCompletedCount() const {
    return m_completedCount;
}

int DownloadService::getFailedCount() const {
    return m_failedCount;
}

DownloadService::DownloadTask DownloadService::getCurrentTask() const {
    return m_currentTask;
}

QList<DownloadService::DownloadTask> DownloadService::getAllTasks() const {
    return m_taskQueue.toList();
}

void DownloadService::processNextTask() {
    if (m_isPaused) {
        qDebug() << "DownloadService: 下载已暂停，停止处理任务";
        return;
    }

    if (m_taskQueue.isEmpty()) {
        qDebug() << "DownloadService: 所有任务完成";
        m_isDownloading = false;
        emit allTasksCompleted();
        return;
    }

    if (m_isDownloading) {
        qDebug() << "DownloadService: 已有任务在下载中";
        return;
    }

    m_currentTask = m_taskQueue.dequeue();
    processCurrentTask();
}

void DownloadService::processCurrentTask() {
    m_isDownloading = true;
    m_currentTask.status = DownloadStatus::Downloading;

    qDebug() << "DownloadService: 开始处理任务:" << m_currentTask.identifier;
    emit taskStarted(m_currentTask);

    // 使用配置下载
    m_ytDlpClient->downloadWithOptions(
        m_currentTask.identifier,
        m_currentTask.outputDir,
        m_currentTask.options
    );
}

void DownloadService::onDownloadFinished(const YtDlpClient::DownloadResult& result) {
    qDebug() << "DownloadService: 下载完成，成功:" << result.success;

    if (!result.success) {
        failCurrentTask(result.errorMessage);
        return;
    }

    m_currentTask.status = DownloadStatus::Processing;

    // 解析元数据
    MetadataParser::ParseResult parseResult = m_metadataParser->parseFromInfoJson(result.tempInfoJsonPath);

    if (!parseResult.success) {
        failCurrentTask(QString("元数据解析失败: %1").arg(parseResult.errorMessage));
        return;
    }

    Song song = parseResult.song;

    // 生成最终文件名
    QString finalFilename = generateFinalFilename(song, m_currentTask.options);
    QString finalFilePath = QDir(m_downloadDir).filePath(finalFilename);

    // 移动并重命名文件
    if (!moveAndRenameFile(result.tempAudioFilePath, finalFilePath)) {
        failCurrentTask("文件重命名失败");
        return;
    }

    // 设置本地文件路径
    song.setLocalFilePath(finalFilePath);

    // 保存到数据库
    if (!m_songRepository->save(song)) {
        failCurrentTask("保存到数据库失败");
        return;
    }

    // 清理临时文件
    cleanupTempFiles(result);

    // 完成任务
    completeCurrentTask(song);
}

void DownloadService::onDownloadError(const QString& error) {
    qDebug() << "DownloadService: 下载错误:" << error;
    failCurrentTask(error);
}

QString DownloadService::generateFinalFilename(const Song& song, const DownloadOptions& options) {
    QString title = MetadataParser::sanitizeFilename(song.getTitle());
    QString artist = MetadataParser::sanitizeFilename(song.getArtist());
    QString extension = options.getFileExtension();

    QString filename;
    if (!artist.isEmpty() && artist != "未知艺术家") {
        filename = QString("%1 - %2.%3").arg(artist, title, extension);
    }
    else {
        filename = QString("%1.%2").arg(title, extension);
    }

    return filename;
}

bool DownloadService::moveAndRenameFile(const QString& tempFilePath, const QString& finalFilePath) {
    if (!QFile::exists(tempFilePath)) {
        qWarning() << "DownloadService: 临时文件不存在:" << tempFilePath;
        return false;
    }

    // 如果目标文件已存在，删除它
    if (QFile::exists(finalFilePath)) {
        if (!QFile::remove(finalFilePath)) {
            qWarning() << "DownloadService: 无法删除已存在的目标文件:" << finalFilePath;
            return false;
        }
    }

    // 移动文件
    if (!QFile::rename(tempFilePath, finalFilePath)) {
        qWarning() << "DownloadService: 文件重命名失败:" << tempFilePath << "->" << finalFilePath;
        return false;
    }

    qDebug() << "DownloadService: 文件重命名成功:" << QFileInfo(finalFilePath).fileName();
    return true;
}

void DownloadService::cleanupTempFiles(const YtDlpClient::DownloadResult& result) {
    // 删除临时 info.json 文件
    if (!result.tempInfoJsonPath.isEmpty() && QFile::exists(result.tempInfoJsonPath)) {
        if (QFile::remove(result.tempInfoJsonPath)) {
            qDebug() << "DownloadService: 清理临时文件:" << result.tempInfoJsonPath;
        }
    }
}

void DownloadService::completeCurrentTask(const Song& song) {
    m_currentTask.status = DownloadStatus::Completed;
    m_currentTask.resultSong = song;
    m_completedCount++;
    m_isDownloading = false;

    qDebug() << "DownloadService: 任务完成:" << song.getTitle();
    emit taskCompleted(m_currentTask, song);

    // 处理下一个任务
    m_processTimer->start(500); // 延迟500ms处理下一个任务
}

void DownloadService::failCurrentTask(const QString& error) {
    m_currentTask.status = DownloadStatus::Failed;
    m_currentTask.errorMessage = error;
    m_failedCount++;
    m_isDownloading = false;

    qWarning() << "DownloadService: 任务失败:" << m_currentTask.identifier << "-" << error;
    emit taskFailed(m_currentTask, error);

    // 处理下一个任务
    m_processTimer->start(500); // 延迟500ms处理下一个任务
}
