#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include "../common/AppConfig.h"
#include "../common/entities/Song.h" 
#include "../common/entities/Playlist.h"
#include "../data/DatabaseManager.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== BiliMusicPlayer C++ 基础功能测试 ===\n";

    // 测试 1: 基本实体类
    qDebug() << "1. 测试基本实体类...";
    try {
        Song song("BV123456789", "测试歌曲", "测试艺术家",
            "https://www.bilibili.com/video/BV123456789",
            "/path/to/song.mp3", "https://example.com/cover.jpg",
            180, QDateTime::currentDateTime());

        qDebug() << "   ✓ Song 对象创建成功";
        qDebug() << "   歌曲信息:" << song.toString();
        qDebug() << "   ID:" << song.getId();
        qDebug() << "   时长:" << song.getDurationSeconds() << "秒";

        Playlist playlist("我的收藏", "最喜欢的歌曲");
        qDebug() << "   ✓ Playlist 对象创建成功";
        qDebug() << "   播放列表:" << playlist.toString();
        qDebug() << "   ID:" << playlist.getId();

    }
    catch (...) {
        qDebug() << "   ✗ 实体类测试失败";
        return 1;
    }

    // 测试 2: AppConfig
    qDebug() << "\n2. 测试 AppConfig...";
    try {
        AppConfig& config = AppConfig::instance();

        if (config.loadConfig()) {
            qDebug() << "   ✓ 配置加载成功";
        }
        else {
            qDebug() << "   ⚠ 配置加载失败，使用默认值";
        }

        qDebug() << "   下载路径:" << config.getDownloadPath();
        qDebug() << "   yt-dlp 路径:" << config.getYtDlpPath();
        qDebug() << "   ffmpeg 路径:" << config.getFfmpegPath();

        // 测试路径是否存在
        QDir downloadDir(config.getDownloadPath());
        if (!downloadDir.exists()) {
            qDebug() << "   ⚠ 下载目录不存在，尝试创建...";
            if (downloadDir.mkpath(".")) {
                qDebug() << "   ✓ 下载目录创建成功";
            }
        }

    }
    catch (...) {
        qDebug() << "   ✗ AppConfig 测试失败";
        return 1;
    }

    // 测试 3: 数据库初始化
    qDebug() << "\n3. 测试数据库初始化...";
    try {
        DatabaseManager& dbManager = DatabaseManager::instance();

        if (dbManager.initialize()) {
            qDebug() << "   ✓ 数据库初始化成功";
            qDebug() << "   数据库已准备就绪";
        }
        else {
            qDebug() << "   ✗ 数据库初始化失败";
            return 1;
        }

    }
    catch (...) {
        qDebug() << "   ✗ 数据库测试失败";
        return 1;
    }

    qDebug() << "\n=== 所有基础测试通过！ ===";
    qDebug() << "按任意键继续...";

    // 等待用户输入，防止程序立即退出
    QTextStream stream(stdin);
    stream.readLine();

    return 0;
}
