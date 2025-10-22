#include "FfmpegClient.h"
#include "../common/AppConfig.h"
#include <QProcess>
#include <QDebug>

FfmpegClient::FfmpegClient(QObject* parent) : QObject(parent) {
}

FfmpegClient& FfmpegClient::instance() {
    static FfmpegClient instance;
    return instance;
}

bool FfmpegClient::testFfmpegAvailable(const QString& ffmpegPath, QString* versionOut) {
    if (ffmpegPath.isEmpty()) {
        qWarning() << "FFmpeg 路径为空";
        return false;
    }

    QProcess process;
    process.start(ffmpegPath, QStringList() << "-version");

    if (!process.waitForFinished(3000)) {
        qWarning() << "FFmpeg 测试超时";
        return false;
    }

    QString output = process.readAllStandardOutput();
    bool available = output.contains("ffmpeg version");

    if (available && versionOut) {
        *versionOut = getFfmpegVersion(ffmpegPath);
    }

    return available;
}

QString FfmpegClient::getFfmpegVersion(const QString& ffmpegPath) {
    QProcess process;
    process.start(ffmpegPath, QStringList() << "-version");

    if (process.waitForFinished(3000)) {
        QString output = process.readAllStandardOutput();
        QStringList lines = output.split('\n');
        if (!lines.isEmpty()) {
            return lines.first().trimmed();
        }
    }

    return "未知版本";
}
