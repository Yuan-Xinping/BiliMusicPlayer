#include "YtDlpClient.h"
#include "DownloadConfig.h"
#include "../common/AppConfig.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

YtDlpClient::YtDlpClient(QObject* parent)
    : QObject(parent)
    , m_processRunner(new ProcessRunner(this))
{
    connect(m_processRunner, &ProcessRunner::finished,
        this, &YtDlpClient::onProcessFinished);
    connect(m_processRunner, &ProcessRunner::error,
        this, &YtDlpClient::onProcessError);
    connect(m_processRunner, &ProcessRunner::outputReady,
        this, &YtDlpClient::onProcessOutput);
}

YtDlpClient::~YtDlpClient() {
    if (m_processRunner && m_processRunner->isRunning()) {
        m_processRunner->terminate();
    }
}

void YtDlpClient::downloadAudio(const QString& identifier, const QString& outputDir) {
    // 使用默认的高质量MP3配置
    DownloadOptions defaultOptions = DownloadOptions::createPreset("high_quality_mp3");
    startDownload(identifier, outputDir, defaultOptions);
}

void YtDlpClient::downloadAudio(const QString& identifier, const QString& outputDir, const DownloadOptions& options) {
    DownloadOptions audioOptions = options;
    audioOptions.extractAudioOnly = true; // 确保提取音频
    startDownload(identifier, outputDir, audioOptions);
}

void YtDlpClient::downloadAudioWithPreset(const QString& identifier, const QString& outputDir, const QString& preset) {
    DownloadOptions options = DownloadOptions::createPreset(preset);
    options.extractAudioOnly = true; // 确保提取音频
    startDownload(identifier, outputDir, options);
}

void YtDlpClient::downloadVideo(const QString& identifier, const QString& outputDir, const DownloadOptions& options) {
    DownloadOptions videoOptions = options;
    videoOptions.extractAudioOnly = false; // 确保下载视频
    startDownload(identifier, outputDir, videoOptions);
}

void YtDlpClient::downloadVideoWithPreset(const QString& identifier, const QString& outputDir, const QString& preset) {
    DownloadOptions options = DownloadOptions::createPreset(preset);
    options.extractAudioOnly = false; // 确保下载视频
    startDownload(identifier, outputDir, options);
}

void YtDlpClient::downloadWithOptions(const QString& identifier, const QString& outputDir, const DownloadOptions& options) {
    startDownload(identifier, outputDir, options);
}

void YtDlpClient::startDownload(const QString& identifier, const QString& outputDir, const DownloadOptions& options) {
    if (isRunning()) {
        emit downloadError("已有下载任务在进行中");
        return;
    }

    m_currentVideoId = extractVideoId(identifier);
    m_currentOutputDir = outputDir;
    m_currentOptions = options;
    m_tempInfoJsonPath.clear();
    m_tempAudioFilePath.clear();

    qDebug() << "YtDlpClient: 开始下载" << identifier;
    qDebug() << "YtDlpClient: 音频格式:" << static_cast<int>(options.audioFormat);
    qDebug() << "YtDlpClient: 音频质量:" << static_cast<int>(options.audioQuality);
    qDebug() << "YtDlpClient: 只提取音频:" << options.extractAudioOnly;

    // 确保输出目录存在
    QDir().mkpath(outputDir);

    // 生成输出模板
    QString outputTemplate = generateOutputTemplate(outputDir, m_currentVideoId, options);

    // 构建参数
    QStringList args = buildYtDlpArguments(identifier, outputTemplate, options);

    qDebug() << "YtDlpClient: 完整参数:" << args;

    // 启动进程
    QString ytDlpPath = getYtDlpPath();
    if (!m_processRunner->start(ytDlpPath, args)) {
        emit downloadError("无法启动 yt-dlp 进程");
    }
}

QStringList YtDlpClient::buildYtDlpArguments(const QString& identifier, const QString& outputTemplate, const DownloadOptions& options) {
    QStringList args;

    // 添加 ffmpeg 路径
    QString ffmpegPath = AppConfig::instance().getFfmpegPath();
    if (!ffmpegPath.isEmpty()) {
        args << "--ffmpeg-location" << ffmpegPath;
    }

    AppConfig& config = AppConfig::instance();
    if (config.getProxyEnabled() && !config.getProxyUrl().isEmpty()) {
        args << "--proxy" << config.getProxyUrl();
        qDebug() << "✅ YtDlpClient: 使用代理:" << config.getProxyUrl();
    }

    // 添加下载配置选项
    args.append(options.toYtDlpArgs());

    // 输出模板
    args << "--output" << outputTemplate;

    // 构建URL
    QString url;
    if (identifier.startsWith("BV") || identifier.startsWith("av")) {
        url = QString("https://www.bilibili.com/video/%1").arg(identifier);
    }
    else if (identifier.startsWith("http")) {
        url = identifier;
    }
    else {
        url = QString("https://www.bilibili.com/video/%1").arg(identifier);
    }

    args << url;

    return args;
}

QString YtDlpClient::generateOutputTemplate(const QString& outputDir, const QString& videoId, const DownloadOptions& options) {
    QString cleanDir = QDir::cleanPath(outputDir);
    QString extension = options.getFileExtension();

    // 使用自定义模板或默认模板
    QString templateStr = options.outputTemplate;
    if (templateStr.isEmpty() || templateStr == "%(title)s.%(ext)s") {
        // 使用我们的默认命名方案
        templateStr = QString("temp_%1.%%(ext)s").arg(videoId);
    }

    return QString("%1/%2").arg(cleanDir, templateStr);
}

void YtDlpClient::setProgressCallback(const ProcessRunner::ProgressCallback& callback) {
    m_processRunner->setProgressCallback(callback);
}

void YtDlpClient::cancel() {
    if (m_processRunner) {
        m_processRunner->terminate();
    }
}

bool YtDlpClient::isRunning() const {
    return m_processRunner && m_processRunner->isRunning();
}

void YtDlpClient::onProcessFinished(int exitCode) {
    qDebug() << "YtDlpClient: 进程完成，退出码:" << exitCode;

    DownloadResult result;
    result.videoId = m_currentVideoId;
    result.usedOptions = m_currentOptions;

    if (exitCode == 0) {
        result.success = true;
        result.tempInfoJsonPath = m_tempInfoJsonPath;
        result.tempAudioFilePath = m_tempAudioFilePath;

        // 验证文件是否存在，如果不存在则尝试查找
        if (!QFileInfo::exists(result.tempAudioFilePath) || result.tempAudioFilePath.isEmpty()) {
            qDebug() << "YtDlpClient: 尝试查找下载的文件...";

            // 尝试查找下载的文件
            QString pattern = QString("temp_%1.*").arg(m_currentVideoId);
            QDir outputDir(m_currentOutputDir);
            QStringList filters;
            filters << pattern;
            QStringList files = outputDir.entryList(filters, QDir::Files);

            qDebug() << "YtDlpClient: 找到文件:" << files;

            for (const QString& file : files) {
                QString fullPath = outputDir.absoluteFilePath(file);
                if (file.endsWith(".json")) {
                    result.tempInfoJsonPath = fullPath;
                    qDebug() << "YtDlpClient: 设置 info.json 路径:" << fullPath;
                }
                else if (!file.endsWith(".part") && !file.endsWith(".tmp")) {
                    result.tempAudioFilePath = fullPath;
                    qDebug() << "YtDlpClient: 设置音频文件路径:" << fullPath;
                }
            }
        }

        if (result.tempAudioFilePath.isEmpty()) {
            result.success = false;
            result.errorMessage = "下载完成但未找到输出文件";
            qWarning() << "YtDlpClient: 未找到下载的音频文件";
        }
    }
    else {
        result.success = false;
        result.errorMessage = QString("yt-dlp 下载失败，退出码: %1").arg(exitCode);
    }

    emit downloadFinished(result);

    // 清理状态
    m_currentVideoId.clear();
    m_currentOutputDir.clear();
    m_tempInfoJsonPath.clear();
    m_tempAudioFilePath.clear();
    m_currentOptions = DownloadOptions(); // 重置为默认
}

void YtDlpClient::onProcessError(const QString& errorString) {
    qWarning() << "YtDlpClient: 进程错误:" << errorString;
    emit downloadError(errorString);
}

void YtDlpClient::onProcessOutput(const QString& line) {
    parseOutputLine(line);

    // 解析进度信息
    if (line.contains("[download]")) {
        QRegularExpression progressRegex(R"(\[download\]\s+(\d+\.?\d*)%)");
        QRegularExpressionMatch match = progressRegex.match(line);
        if (match.hasMatch()) {
            double progress = match.captured(1).toDouble() / 100.0;
            emit downloadProgress(progress, line);
            return;
        }
    }

    // 对于其他输出，发送进度为0（表示处理中）
    emit downloadProgress(0.0, line);
}

QString YtDlpClient::extractVideoId(const QString& identifier) {
    // 从URL或标识符中提取视频ID
    if (identifier.startsWith("BV") || identifier.startsWith("av")) {
        return identifier;
    }

    // 从URL中提取BV号或av号
    QRegularExpression regex(R"(/video/([^/?]+))");
    QRegularExpressionMatch match = regex.match(identifier);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    // 如果都不匹配，使用时间戳作为备用ID
    return QString("unknown_%1").arg(QDateTime::currentMSecsSinceEpoch());
}

void YtDlpClient::parseOutputLine(const QString& line) {
    qDebug() << "YtDlpClient 输出:" << line;

    // 检查是否包含目标文件前缀
    QString targetPrefix = QString("temp_%1").arg(m_currentVideoId);

    if (line.contains(targetPrefix)) {
        // 解析 info.json 文件路径
        if (line.contains(".info.json")) {
            // 匹配多种可能的输出格式
            QStringList infoPatterns = {
                QString(R"(\[info\]\s+Writing.*?to:\s+([^\r\n]+%1[^\r\n]*\.info\.json))").arg(m_currentVideoId),
                QString(R"(([^\s]+%1[^\s]*\.info\.json))").arg(m_currentVideoId)
            };

            for (const QString& pattern : infoPatterns) {
                QRegularExpression regex(pattern);
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    m_tempInfoJsonPath = match.captured(1).trimmed();
                    qDebug() << "YtDlpClient: 找到 info.json 路径:" << m_tempInfoJsonPath;
                    break;
                }
            }
        }

        // 解析音频/视频文件路径
        if (!line.contains(".json") && (
            line.contains("Destination:") ||
            line.contains("has already been downloaded") ||
            line.contains("[download] 100%") ||
            line.contains("[EmbedThumbnail]") ||
            line.contains("[ExtractAudio]"))) {

            // 匹配多种可能的文件扩展名
            QStringList audioPatterns = {
                QString(R"(Destination:\s+([^\r\n]+%1[^\r\n]*\.[a-zA-Z0-9]{2,4}))").arg(m_currentVideoId),
                QString(R"(([^\s]+%1[^\s]*\.(mp3|m4a|opus|flac|wav|mp4|webm|mkv)))").arg(m_currentVideoId),
                QString(R"(\[.*?\]\s+([^\s]+%1[^\s]*\.[a-zA-Z0-9]{2,4}))").arg(m_currentVideoId)
            };

            for (const QString& pattern : audioPatterns) {
                QRegularExpression regex(pattern);
                QRegularExpressionMatch match = regex.match(line);
                if (match.hasMatch()) {
                    QString filePath = match.captured(1).trimmed();

                    // 过滤掉不需要的文件
                    if (!filePath.endsWith(".json") &&
                        !filePath.endsWith(".part") &&
                        !filePath.endsWith(".tmp") &&
                        !filePath.contains("temp")) {

                        m_tempAudioFilePath = filePath;
                        qDebug() << "YtDlpClient: 找到音频文件路径:" << m_tempAudioFilePath;
                        break;
                    }
                }
            }
        }
    }

    // 额外的通用文件路径检测（备用方案）
    if (m_tempAudioFilePath.isEmpty() && line.contains(m_currentVideoId)) {
        QRegularExpression genericRegex(QString(R"(([^\s]*%1[^\s]*\.(mp3|m4a|opus|flac|wav|mp4|webm)))").arg(m_currentVideoId));
        QRegularExpressionMatch match = genericRegex.match(line);
        if (match.hasMatch()) {
            QString filePath = match.captured(1);
            if (!filePath.endsWith(".json") && QFileInfo::exists(filePath)) {
                m_tempAudioFilePath = filePath;
                qDebug() << "YtDlpClient: 通过通用模式找到文件:" << m_tempAudioFilePath;
            }
        }
    }
}

QString YtDlpClient::getYtDlpPath() {
    QString path = AppConfig::instance().getYtDlpPath();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        qWarning() << "YtDlpClient: yt-dlp 路径无效:" << path;
        return QString();
    }
    return path;
}

// 静态方法实现
QStringList YtDlpClient::getAvailablePresets() {
    return DownloadOptions::getAvailablePresets();
}

QString YtDlpClient::getPresetDescription(const QString& preset) {
    if (preset == "high_quality_mp3") return "高质量MP3 (最佳音质)";
    if (preset == "medium_quality_mp3") return "中等质量MP3 (平衡大小和质量)";
    if (preset == "low_quality_mp3") return "低质量MP3 (小文件大小)";
    if (preset == "lossless_flac") return "无损FLAC (最高质量)";
    if (preset == "small_size_opus") return "小体积OPUS (高压缩率)";
    if (preset == "video_720p") return "720p视频";
    if (preset == "video_1080p") return "1080p视频";
    return "未知预设";
}
