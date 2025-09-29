#include <QApplication>
#include <QDebug>
#include <QFile>
#include "BiliMusicPlayerApp.h"
#include "../ui/windows/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);

    BiliMusicPlayerApp appManager;

    qDebug() << "====== 资源存在性检查 ======";
    if (QFile::exists(":/main.qss")) {
        qDebug() << "诊断成功: 资源 ':/main.qss' 确实存在于可执行文件中！";
    }
    else {
        qDebug() << "诊断失败: 资源 ':/main.qss' 未被编译进可执行文件！问题在CMake配置。";
    }
    qDebug() << "===========================";

    if (!appManager.initialize()) {
        qCritical() << "应用程序初始化失败，程序即将退出。";
        return 1;
    }

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
