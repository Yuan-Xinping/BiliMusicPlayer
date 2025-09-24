#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include "../common/AppConfig.h"

void testBinaryAvailability() {
    qDebug() << "=== 测试二进制文件可用性 ===";

    AppConfig& config = AppConfig::instance();

    // 测试 yt-dlp
    qDebug() << "\n--- yt-dlp 测试 ---";
    QString ytDlpPath = config.getYtDlpPath();
    qDebug() << "路径:" << ytDlpPath;

    if (ytDlpPath.isEmpty()) {
        qDebug() << "❌ yt-dlp 路径为空";
    }
    else {
        QFileInfo ytdlpInfo(ytDlpPath);
        qDebug() << "文件存在:" << (ytdlpInfo.exists() ? "是" : "否");
        qDebug() << "文件大小:" << ytdlpInfo.size() << "字节";
        qDebug() << "可执行:" << (ytdlpInfo.isExecutable() ? "是" : "否");

        // 尝试运行 yt-dlp --version
        QProcess ytdlpProcess;
        ytdlpProcess.start(ytDlpPath, QStringList() << "--version");
        if (ytdlpProcess.waitForFinished(5000)) {
            QString version = ytdlpProcess.readAllStandardOutput().trimmed();
            qDebug() << "✓ yt-dlp 版本:" << version;
        }
        else {
            qDebug() << "❌ yt-dlp 版本检查失败:" << ytdlpProcess.errorString();
        }
    }

    // 测试 ffmpeg
    qDebug() << "\n--- ffmpeg 测试 ---";
    QString ffmpegPath = config.getFfmpegPath();
    qDebug() << "路径:" << ffmpegPath;

    if (ffmpegPath.isEmpty()) {
        qDebug() << "❌ ffmpeg 路径为空";
    }
    else {
        QFileInfo ffmpegInfo(ffmpegPath);
        qDebug() << "文件存在:" << (ffmpegInfo.exists() ? "是" : "否");
        qDebug() << "文件大小:" << ffmpegInfo.size() << "字节";
        qDebug() << "可执行:" << (ffmpegInfo.isExecutable() ? "是" : "否");

        // 尝试运行 ffmpeg -version
        QProcess ffmpegProcess;
        ffmpegProcess.start(ffmpegPath, QStringList() << "-version");
        if (ffmpegProcess.waitForFinished(5000)) {
            QString output = ffmpegProcess.readAllStandardOutput();
            QStringList lines = output.split('\n');
            if (!lines.isEmpty()) {
                qDebug() << "✓ ffmpeg 版本:" << lines.first().trimmed();
            }
        }
        else {
            qDebug() << "❌ ffmpeg 版本检查失败:" << ffmpegProcess.errorString();
        }
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== BiliMusicPlayer 二进制文件测试 ===\n";

    // 加载配置
    AppConfig::instance().loadConfig();

    // 测试二进制文件
    testBinaryAvailability();

    qDebug() << "\n=== 测试完成 ===";

    return 0;
}
