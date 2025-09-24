#include <QApplication>
#include <QDebug>
#include "BiliMusicPlayerApp.h"
#include "../ui/windows/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    qDebug() << "��� BiliMusicPlayer...";

    // ����Ӧ�ó��������
    BiliMusicPlayerApp appManager;

    // ��ʼ��Ӧ�ó���
    if (!appManager.initialize()) {
        qCritical() << "Ӧ�ó����ʼ��ʧ��";
        return 1;
    }

    // ��������ʾ������
    MainWindow mainWindow;
    mainWindow.show();

    qDebug() << "BiliMusicPlayer �����";

    return app.exec();
}
