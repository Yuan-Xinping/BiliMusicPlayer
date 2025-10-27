#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include "BiliMusicPlayerApp.h"
#include "../ui/windows/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    // 创建应用管理器
    BiliMusicPlayerApp appManager;

    // 初始化应用（加载配置、初始化数据库、创建服务）
    if (!appManager.initialize()) {
        qCritical() << "应用程序初始化失败，程序即将退出。";
        return 1;
    }

    // 创建主窗口
    MainWindow mainWindow;

    // 将应用管理器实例传递给主窗口
    mainWindow.setApp(&appManager);

    // 显示主窗口
    mainWindow.show();

    return app.exec();
}