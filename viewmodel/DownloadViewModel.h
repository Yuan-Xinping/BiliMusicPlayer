#pragma once
#include <QObject>

class DownloadViewModel : public QObject {
    Q_OBJECT
public:
    explicit DownloadViewModel(QObject* parent = nullptr);
    static DownloadViewModel& instance();
};
