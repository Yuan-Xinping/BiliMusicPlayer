#pragma once
#include <QObject>

class YtDlpClient : public QObject {
    Q_OBJECT
public:
    explicit YtDlpClient(QObject* parent = nullptr);
    static YtDlpClient& instance();
};
