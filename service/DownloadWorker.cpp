#include "DownloadWorker.h"
#include "../common/AppConfig.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>

DownloadWorker::DownloadWorker(const QString& taskId, const DownloadTaskState& task, QObject* parent)
    : QObject(parent)
    , m_taskId(taskId)
    , m_taskState(task)
    , m_ytDlpClient(nullptr)
    , m_metadataParser(nullptr)
    , m_isRunning(false)
    , m_isCancelled(false)
    , m_timeoutTimer(new QTimer(this))
{
    setAutoDelete(false); // 手动管理内存

    m_downloadDir = AppConfig::instance().getDownloadPath();

    // 设置超时定时器
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(300000); // 5分钟超时
    connect(m_timeoutTimer, &QTimer::timeout, this, &DownloadWorker::onTimeout);

    qDebug() << "DownloadWorker: 创建工作线程，任务ID:" << m_taskId;
}

DownloadWorker::~DownloadWorker() {
    cleanup();
}

void DownloadWorker::run() {
    QMutexLocker locker(&m_stateMutex);

    if (m_isCancelled) {
        qDebug() << "DownloadWorker: 任务已取消，退出:" << m_taskId;
        return;
    }

    m_isRunning = true;
    locker.unlock();

    qDebug() << "DownloadWorker: 开始执行任务:" << m_taskId;

    // 在工作线程中创建 YtDlpClient 和 MetadataParser
    setupYtDlpClient();

    // 使用 QTimer::singleShot 代替直接启动定时器
    int timeoutMs = 300000; // 5分钟超时

    // 开始下载
    emit taskProgress(m_taskId, 0.0, "开始下载...");

    m_ytDlpClient->downloadWithOptions(
        m_taskState.getIdentifier(),
        m_downloadDir,
        m_taskState.getOptions()
    );

    // 启动事件循环等待下载完成
    QEventLoop eventLoop;

    // 连接完成信号
    connect(m_ytDlpClient, &YtDlpClient::downloadFinished, &eventLoop, &QEventLoop::quit);
    connect(m_ytDlpClient, &YtDlpClient::downloadError, &eventLoop, &QEventLoop::quit);

    // 使用 QTimer::singleShot 设置超时
    QTimer::singleShot(timeoutMs, &eventLoop, &QEventLoop::quit);

    // 等待下载完成或超时
    eventLoop.exec();

    // 清理
    cleanup();

    QMutexLocker finishLocker(&m_stateMutex);
    m_isRunning = false;
}


void DownloadWorker::setupYtDlpClient() {
    m_ytDlpClient = new YtDlpClient();
    m_metadataParser = new MetadataParser();

    // 连接信号
    connect(m_ytDlpClient, &YtDlpClient::downloadFinished,
        this, &DownloadWorker::onDownloadFinished);
    connect(m_ytDlpClient, &YtDlpClient::downloadError,
        this, &DownloadWorker::onDownloadError);

    // 设置进度回调
    m_ytDlpClient->setProgressCallback([this](double progress, const QString& message) {
        if (!m_isCancelled) {
            emit taskProgress(m_taskId, progress, message);
        }
        });
}

void DownloadWorker::cancel() {
    QMutexLocker locker(&m_stateMutex);

    qDebug() << "DownloadWorker: 取消任务:" << m_taskId;
    m_isCancelled = true;

    if (m_ytDlpClient && m_ytDlpClient->isRunning()) {
        m_ytDlpClient->cancel();
    }

    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }
}

void DownloadWorker::onDownloadFinished(const YtDlpClient::DownloadResult& result) {
    if (m_isCancelled) {
        return;
    }

    m_timeoutTimer->stop();

    qDebug() << "DownloadWorker: 下载完成，任务ID:" << m_taskId << "，成功:" << result.success;

    if (!result.success) {
        emit taskFinished(m_taskId, false, result.errorMessage, Song());
        return;
    }

    // 解析元数据
    emit taskProgress(m_taskId, 0.9, "解析元数据...");

    MetadataParser::ParseResult parseResult = m_metadataParser->parseFromInfoJson(result.tempInfoJsonPath);

    if (!parseResult.success) {
        QString error = QString("元数据解析失败: %1").arg(parseResult.errorMessage);
        emit taskFinished(m_taskId, false, error, Song());
        return;
    }

    Song song = parseResult.song;

    // 生成最终文件名并移动文件
    emit taskProgress(m_taskId, 0.95, "重命名文件...");

    QString finalFilename = generateFinalFilename(song, m_taskState.getOptions());
    QString finalFilePath = QDir(m_downloadDir).filePath(finalFilename);

    if (!moveAndRenameFile(result.tempAudioFilePath, finalFilePath)) {
        emit taskFinished(m_taskId, false, "文件重命名失败", Song());
        return;
    }

    // 设置本地文件路径
    song.setLocalFilePath(finalFilePath);

    // 清理临时文件
    cleanupTempFiles(result);

    emit taskFinished(m_taskId, true, QString(), song);
}

void DownloadWorker::onDownloadError(const QString& error) {
    if (m_isCancelled) {
        return;
    }

    m_timeoutTimer->stop();
    qDebug() << "DownloadWorker: 下载错误，任务ID:" << m_taskId << "，错误:" << error;
    emit taskFinished(m_taskId, false, error, Song());
}

void DownloadWorker::onTimeout() {
    qDebug() << "DownloadWorker: 任务超时，任务ID:" << m_taskId;

    if (m_ytDlpClient && m_ytDlpClient->isRunning()) {
        m_ytDlpClient->cancel();
    }

    emit taskFinished(m_taskId, false, "下载超时", Song());
}

QString DownloadWorker::generateFinalFilename(const Song& song, const DownloadOptions& options) {
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

bool DownloadWorker::moveAndRenameFile(const QString& tempFilePath, const QString& finalFilePath) {
    if (!QFile::exists(tempFilePath)) {
        qWarning() << "DownloadWorker: 临时文件不存在:" << tempFilePath;
        return false;
    }

    // 如果目标文件已存在，删除它
    if (QFile::exists(finalFilePath)) {
        if (!QFile::remove(finalFilePath)) {
            qWarning() << "DownloadWorker: 无法删除已存在的目标文件:" << finalFilePath;
            return false;
        }
    }

    // 移动文件
    if (!QFile::rename(tempFilePath, finalFilePath)) {
        qWarning() << "DownloadWorker: 文件重命名失败:" << tempFilePath << "->" << finalFilePath;
        return false;
    }

    qDebug() << "DownloadWorker: 文件重命名成功:" << QFileInfo(finalFilePath).fileName();
    return true;
}

void DownloadWorker::cleanupTempFiles(const YtDlpClient::DownloadResult& result) {
    // 删除临时 info.json 文件
    if (!result.tempInfoJsonPath.isEmpty() && QFile::exists(result.tempInfoJsonPath)) {
        if (QFile::remove(result.tempInfoJsonPath)) {
            qDebug() << "DownloadWorker: 清理临时文件:" << result.tempInfoJsonPath;
        }
    }
}

void DownloadWorker::cleanup() {
    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
    }

    if (m_ytDlpClient) {
        if (m_ytDlpClient->isRunning()) {
            m_ytDlpClient->cancel();
        }
        m_ytDlpClient->deleteLater();
        m_ytDlpClient = nullptr;
    }

    if (m_metadataParser) {
        m_metadataParser->deleteLater();
        m_metadataParser = nullptr;
    }
}
