#pragma once
#include <QObject>

class DownloadService;

class BiliMusicPlayerApp : public QObject {
    Q_OBJECT

public:
    explicit BiliMusicPlayerApp(QObject* parent = nullptr);
    ~BiliMusicPlayerApp() override;

    bool initialize();

    DownloadService* getDownloadService() const;

private:
    bool initializeDatabase();
    bool initializeServices();
    void setupLogging();

    DownloadService* m_downloadService;
};
