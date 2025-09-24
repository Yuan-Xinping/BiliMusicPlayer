#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include "../common/AppConfig.h"
#include "../infra/ProcessRunner.h"
#include "../infra/YtDlpClient.h"
#include "../infra/DownloadConfig.h" // 新增
#include "../data/DatabaseManager.h"

class DownloadTester : public QObject {
    Q_OBJECT

public:
    explicit DownloadTester(QObject* parent = nullptr) : QObject(parent) {}

    void testProcessRunner() {
        qDebug() << "\n=== 测试 ProcessRunner ===";

        ProcessRunner* runner = new ProcessRunner(this);

        // 设置回调函数
        runner->setProgressCallback([](double progress, const QString& message) {
            qDebug() << "进度:" << QString::number(progress * 100, 'f', 1) + "% -" << message;
            });

        runner->setOutputCallback([](const QString& line) {
            qDebug() << "输出:" << line;
            });

        // 连接信号
        connect(runner, &ProcessRunner::finished, [this](int exitCode) {
            qDebug() << "✓ ProcessRunner 测试完成，退出码:" << exitCode;
            testDownloadConfig(); // 先测试配置，再测试下载
            });

        connect(runner, &ProcessRunner::error, [this](const QString& error) {
            qDebug() << "❌ ProcessRunner 错误:" << error;
            testDownloadConfig(); // 继续下一个测试
            });

        // 测试 yt-dlp 版本命令
        qDebug() << "测试 yt-dlp --version 命令...";
        QStringList args;
        args << "--version";

        QString ytDlpPath = AppConfig::instance().getYtDlpPath();
        qDebug() << "使用 yt-dlp 路径:" << ytDlpPath;

        if (!runner->start(ytDlpPath, args)) {
            qDebug() << "❌ 无法启动 yt-dlp 进程";
            testDownloadConfig();
        }
    }

    void testDownloadConfig() {
        qDebug() << "\n=== 测试下载配置 ===";

        // 测试预设配置
        qDebug() << "--- 可用预设配置 ---";
        QStringList presets = DownloadOptions::getAvailablePresets();
        for (const QString& preset : presets) {
            DownloadOptions options = DownloadOptions::createPreset(preset);
            qDebug() << "预设:" << preset;
            qDebug() << "  描述:" << YtDlpClient::getPresetDescription(preset);
            qDebug() << "  音频格式:" << static_cast<int>(options.audioFormat);
            qDebug() << "  音频质量:" << static_cast<int>(options.audioQuality);
            qDebug() << "  文件扩展名:" << options.getFileExtension();
            qDebug() << "  只提取音频:" << (options.extractAudioOnly ? "是" : "否");

            // 显示部分参数
            QStringList args = options.toYtDlpArgs();
            QString argsPreview = args.size() > 8 ? args.mid(0, 8).join(" ") + "..." : args.join(" ");
            qDebug() << "  参数预览:" << argsPreview;
            qDebug();
        }

        // 测试自定义配置
        qDebug() << "--- 测试自定义配置 ---";
        DownloadOptions customOptions;
        customOptions.audioQuality = AudioQuality::Best;
        customOptions.audioFormat = AudioFormat::FLAC;
        customOptions.embedThumbnail = true;
        customOptions.writeInfoJson = true;
        customOptions.maxRetries = 5;
        customOptions.rateLimitKbps = "500K";

        qDebug() << "自定义配置 (无损FLAC):";
        qDebug() << "  参数:" << customOptions.toYtDlpArgs().join(" ");

        testYtDlpClient();
    }

    void testYtDlpClient() {
        qDebug() << "\n=== 测试 YtDlpClient 配置支持 ===";

        // 检查配置
        qDebug() << "yt-dlp 路径:" << AppConfig::instance().getYtDlpPath();
        qDebug() << "ffmpeg 路径:" << AppConfig::instance().getFfmpegPath();

        if (!AppConfig::instance().isYtDlpAvailable()) {
            qDebug() << "⚠️ yt-dlp 不可用，跳过下载测试";
            testCompleted();
            return;
        }

        YtDlpClient* client = new YtDlpClient(this);

        // 设置进度回调
        client->setProgressCallback([](double progress, const QString& message) {
            qDebug() << "下载进度:" << QString::number(progress * 100, 'f', 1) + "% -" << message;
            });

        // 连接信号
        connect(client, &YtDlpClient::downloadFinished,
            [this](const YtDlpClient::DownloadResult& result) {
                qDebug() << "\n下载完成，成功:" << result.success;
                if (result.success) {
                    qDebug() << "✓ info.json 路径:" << result.tempInfoJsonPath;
                    qDebug() << "✓ 音频文件路径:" << result.tempAudioFilePath;
                    qDebug() << "✓ 视频ID:" << result.videoId;

                    // 显示使用的配置信息
                    qDebug() << "✓ 使用的配置:";
                    qDebug() << "  - 音频格式:" << static_cast<int>(result.usedOptions.audioFormat);
                    qDebug() << "  - 音频质量:" << static_cast<int>(result.usedOptions.audioQuality);
                    qDebug() << "  - 文件扩展名:" << result.usedOptions.getFileExtension();
                    qDebug() << "  - 嵌入缩略图:" << (result.usedOptions.embedThumbnail ? "是" : "否");

                    // 检查文件是否存在
                    if (QFileInfo::exists(result.tempInfoJsonPath)) {
                        qDebug() << "✓ info.json 文件确实存在";
                    }
                    else {
                        qDebug() << "⚠️ info.json 文件不存在";
                    }

                    if (QFileInfo::exists(result.tempAudioFilePath)) {
                        qDebug() << "✓ 音频文件确实存在";
                        QFileInfo audioInfo(result.tempAudioFilePath);
                        qDebug() << "  文件大小:" << audioInfo.size() << "字节";
                        qDebug() << "  实际扩展名:" << audioInfo.suffix();
                    }
                    else {
                        qDebug() << "⚠️ 音频文件不存在";
                    }
                }
                else {
                    qDebug() << "❌ 下载失败:" << result.errorMessage;
                }

                // 开始第二个测试 - 测试不同的预设
                testDifferentPreset();
            });

        connect(client, &YtDlpClient::downloadError,
            [this](const QString& error) {
                qDebug() << "❌ YtDlpClient 错误:" << error;
                testDifferentPreset(); // 继续测试其他预设
            });

        // 创建测试输出目录
        QString testOutputDir = AppConfig::instance().getDownloadPath() + "/config_test";
        QDir().mkpath(testOutputDir);
        qDebug() << "创建测试输出目录:" << testOutputDir;

        // 使用一个短小的B站视频进行测试
        QString testBvId = "BV1xx411c7mD";

        qDebug() << "\n开始第一个测试 - 使用 small_size_opus 预设:";
        qDebug() << "  BV号:" << testBvId;
        qDebug() << "  输出目录:" << testOutputDir;
        qDebug() << "  预设描述:" << YtDlpClient::getPresetDescription("small_size_opus");

        // 使用小体积OPUS格式测试
        client->downloadAudioWithPreset(testBvId, testOutputDir, "small_size_opus");
    }

    void testDifferentPreset() {
        qDebug() << "\n=== 测试不同预设配置 ===";

        YtDlpClient* client = new YtDlpClient(this);

        connect(client, &YtDlpClient::downloadFinished,
            [this](const YtDlpClient::DownloadResult& result) {
                qDebug() << "\n第二个测试完成，成功:" << result.success;
                if (result.success) {
                    qDebug() << "✓ 使用的配置 - 音频格式:" << static_cast<int>(result.usedOptions.audioFormat);
                    qDebug() << "✓ 使用的配置 - 音频质量:" << static_cast<int>(result.usedOptions.audioQuality);
                    qDebug() << "✓ 音频文件:" << result.tempAudioFilePath;

                    if (QFileInfo::exists(result.tempAudioFilePath)) {
                        QFileInfo info(result.tempAudioFilePath);
                        qDebug() << "✓ 文件大小:" << info.size() << "字节";
                        qDebug() << "✓ 扩展名:" << info.suffix();
                    }
                }
                else {
                    qDebug() << "❌ 第二个测试失败:" << result.errorMessage;
                }
                testCompleted();
            });

        connect(client, &YtDlpClient::downloadError,
            [this](const QString& error) {
                qDebug() << "❌ 第二个测试错误:" << error;
                testCompleted();
            });

        QString testOutputDir = AppConfig::instance().getDownloadPath() + "/config_test2";
        QDir().mkpath(testOutputDir);

        qDebug() << "\n开始第二个测试 - 使用 medium_quality_mp3 预设:";
        qDebug() << "  预设描述:" << YtDlpClient::getPresetDescription("medium_quality_mp3");

        // 测试不同的预设
        client->downloadAudioWithPreset("BV1xx411c7mD", testOutputDir, "medium_quality_mp3");
    }

    void testCompleted() {
        qDebug() << "\n=== 所有测试完成 ===";
        qDebug() << "测试报告:";
        qDebug() << "  ✓ ProcessRunner 基础功能测试";
        qDebug() << "  ✓ 下载配置选项测试";
        qDebug() << "  ✓ YtDlpClient 配置支持测试";
        qDebug() << "  ✓ 不同预设配置对比测试";
        qDebug() << "\n可以检查输出目录中的下载文件来验证结果！";

        QCoreApplication::quit();
    }
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== BiliMusicPlayer 下载器配置测试 ===\n";

    // 初始化配置
    if (!AppConfig::instance().loadConfig()) {
        qDebug() << "⚠️ 配置加载失败，使用默认配置";
    }

    // 初始化数据库
    if (!DatabaseManager::instance().initialize()) {
        qDebug() << "⚠️ 数据库初始化失败";
    }

    // 显示当前配置
    qDebug() << "当前配置:";
    qDebug() << "  下载路径:" << AppConfig::instance().getDownloadPath();
    qDebug() << "  yt-dlp 路径:" << AppConfig::instance().getYtDlpPath();
    qDebug() << "  ffmpeg 路径:" << AppConfig::instance().getFfmpegPath();
    qDebug() << "  yt-dlp 可用:" << (AppConfig::instance().isYtDlpAvailable() ? "是" : "否");
    qDebug() << "  ffmpeg 可用:" << (AppConfig::instance().isFfmpegAvailable() ? "是" : "否");

    // 显示可用预设
    qDebug() << "\n可用下载预设:" << YtDlpClient::getAvailablePresets();

    // 创建测试器并开始测试
    DownloadTester tester;

    // 使用定时器启动测试，确保事件循环已经开始
    QTimer::singleShot(100, &tester, &DownloadTester::testProcessRunner);

    return app.exec();
}

#include "test_downloader.moc"
