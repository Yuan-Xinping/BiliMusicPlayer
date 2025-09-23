#pragma once
#include <QObject>

class FfmpegClient : public QObject {
    Q_OBJECT
public:
    explicit FfmpegClient(QObject* parent = nullptr);
    static FfmpegClient& instance();
};
