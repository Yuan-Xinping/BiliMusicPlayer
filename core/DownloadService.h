#pragma once
#include <QObject>

class DownloadService : public QObject {
    Q_OBJECT
public:
    explicit DownloadService(QObject* parent = nullptr);
    static DownloadService& instance();
};
