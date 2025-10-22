#pragma once
#include <QObject>
#include <QString>

class FfmpegClient : public QObject {
    Q_OBJECT

public:
    explicit FfmpegClient(QObject* parent = nullptr);
    static FfmpegClient& instance();

    static bool testFfmpegAvailable(const QString& ffmpegPath, QString* versionOut = nullptr);

    static QString getFfmpegVersion(const QString& ffmpegPath);
};
