#pragma once
#include <QObject>

class BiliMusicPlayerApp : public QObject {
    Q_OBJECT
public:
    explicit BiliMusicPlayerApp(QObject* parent = nullptr);
    ~BiliMusicPlayerApp() override = default;

    bool initialize();

private:
    bool initializeServices();
    bool initializeDatabase();
    void setupLogging();
};
