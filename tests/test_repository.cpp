#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include "../data/DatabaseManager.h"
#include "../data/SongRepository.h"
#include "../data/PlaylistRepository.h"
#include "../common/entities/Song.h"
#include "../common/entities/Playlist.h"

void testSongRepository() {
    qDebug() << "\n=== 测试 SongRepository ===";

    SongRepository songRepo;

    // 测试保存歌曲
    Song song1("BV1234567890", "测试歌曲1", "测试歌手1",
        "https://bilibili.com/video/BV1234567890",
        "C:/music/test1.mp3", "https://cover1.url",
        240, QDateTime::currentDateTime(), true);

    Song song2("BV1234567891", "测试歌曲2", "测试歌手2",
        "https://bilibili.com/video/BV1234567891",
        "C:/music/test2.mp3", "https://cover2.url",
        180, QDateTime::currentDateTime(), false);

    if (songRepo.save(song1) && songRepo.save(song2)) {
        qDebug() << "✓ 歌曲保存测试通过";
    }

    // 测试查询所有歌曲
    auto allSongs = songRepo.findAll();
    qDebug() << "✓ 查询所有歌曲，共" << allSongs.size() << "首";

    // 测试按ID查找
    Song foundSong = songRepo.findById("BV1234567890");
    if (!foundSong.getId().isEmpty()) {
        qDebug() << "✓ 按ID查找成功:" << foundSong.getTitle();
    }

    // 测试收藏歌曲查询
    auto favorites = songRepo.findFavorites();
    qDebug() << "✓ 收藏歌曲查询，共" << favorites.size() << "首";

    // 测试搜索
    auto searchResults = songRepo.findByTitle("测试");
    qDebug() << "✓ 搜索测试，找到" << searchResults.size() << "首歌曲";

    // 测试统计
    int totalCount = songRepo.count();
    qDebug() << "✓ 总歌曲数:" << totalCount;
}

void testPlaylistRepository() {
    qDebug() << "\n=== 测试 PlaylistRepository ===";

    PlaylistRepository playlistRepo;
    SongRepository songRepo;

    // 创建测试播放列表
    Playlist playlist1("我的收藏", "收藏的歌曲");
    Playlist playlist2("流行音乐", "流行歌曲合集");

    if (playlistRepo.save(playlist1) && playlistRepo.save(playlist2)) {
        qDebug() << "✓ 播放列表保存测试通过";
    }

    // 测试查询所有播放列表
    auto allPlaylists = playlistRepo.findAll();
    qDebug() << "✓ 查询所有播放列表，共" << allPlaylists.size() << "个";

    // 测试添加歌曲到播放列表
    if (!allPlaylists.isEmpty()) {
        QString playlistId = allPlaylists.first().getId();

        // 添加歌曲到播放列表
        if (playlistRepo.addSongToPlaylist(playlistId, "BV1234567890") &&
            playlistRepo.addSongToPlaylist(playlistId, "BV1234567891")) {
            qDebug() << "✓ 添加歌曲到播放列表测试通过";
        }

        // 查询播放列表中的歌曲
        auto songsInPlaylist = playlistRepo.getSongsInPlaylist(playlistId);
        qDebug() << "✓ 播放列表中有" << songsInPlaylist.size() << "首歌曲";

        int songCount = playlistRepo.getSongCountInPlaylist(playlistId);
        qDebug() << "✓ 播放列表歌曲统计:" << songCount << "首";
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== BiliMusicPlayer Repository 层测试 ===";

    // 初始化数据库
    if (!DatabaseManager::instance().initialize()) {
        qDebug() << "❌ 数据库初始化失败";
        return 1;
    }

    // 运行测试
    testSongRepository();
    testPlaylistRepository();

    qDebug() << "\n=== Repository 测试完成 ===";
    qDebug() << "按任意键退出...";
    getchar();

    return 0;
}
