#include "BiliMusicPlayerApp.h"
#include <QApplication>
#include "../ui/windows/MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    BiliMusicPlayerApp app;
    if (!app.init()) {
        return 1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
