#pragma once
#include <QString>
#include <QStringList>

enum class AudioQuality {
    Best = 0,      // 最佳质量 (--audio-quality 0)
    High = 2,      // 高质量 (--audio-quality 2)
    Medium = 5,    // 中等质量 (--audio-quality 5)
    Low = 9        // 低质量 (--audio-quality 9)
};

enum class AudioFormat {
    MP3,           // mp3 格式
    M4A,           // m4a/aac 格式
    OPUS,          // opus 格式 (更小文件)
    FLAC,          // flac 格式 (无损)
    WAV            // wav 格式 (无压缩)
};

enum class VideoQuality {
    Best,          // 最佳视频质量
    Worst,         // 最低视频质量
    P1080,         // 1080p
    P720,          // 720p
    P480,          // 480p
    P360           // 360p
};

struct DownloadOptions {
    // 音频选项
    AudioQuality audioQuality = AudioQuality::High;
    AudioFormat audioFormat = AudioFormat::MP3;
    bool extractAudioOnly = true; // 是否只提取音频

    // 视频选项（当不只提取音频时）
    VideoQuality videoQuality = VideoQuality::P720;
    QString videoFormat = "mp4"; // 视频容器格式

    // 元数据选项
    bool embedThumbnail = true;        // 嵌入缩略图
    bool writeInfoJson = true;         // 写入信息JSON
    bool writeDescription = false;     // 写入描述文件
    bool writeSubtitles = false;       // 下载字幕
    bool writeAutoSubtitles = false;   // 下载自动生成字幕

    // 文件名选项
    QString outputTemplate = "%(title)s.%(ext)s"; // 输出文件名模板
    bool restrictFilenames = true;     // 限制文件名字符

    // 网络选项
    int maxRetries = 3;                // 最大重试次数
    int fragmentRetries = 10;          // 分片重试次数
    QString proxy;                     // 代理设置（可选）

    // 速度限制
    QString rateLimitKbps;             // 速度限制（如 "1000K"）

    // 高级选项
    bool ignoreCertificateErrors = false; // 忽略SSL证书错误
    bool noCheckCertificate = false;      // 不检查证书
    QStringList customArgs;               // 自定义参数

    // 转换为 yt-dlp 命令行参数
    QStringList toYtDlpArgs() const;

    // 获取文件扩展名
    QString getFileExtension() const;

    // 预设配置
    static DownloadOptions createPreset(const QString& presetName);
    static QStringList getAvailablePresets();
};
