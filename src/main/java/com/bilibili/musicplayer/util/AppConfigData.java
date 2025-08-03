package com.bilibili.musicplayer.util;

import java.io.File;

/**
 * 应用程序配置数据模型，用于 JSON 序列化和反序列化。
 */
public class AppConfigData {

    private String downloadPath;
    private String ytDlpPath;
    private String ffmpegPath;

    // Jackson 需要一个无参构造函数来反序列化 JSON
    public AppConfigData() {
        // 设置默认值
        this.downloadPath = System.getProperty("user.home") + File.separator + "BiliMusicPlayer_Downloads";

        // 现在直接调用 AppConfig.getBundledBinaryPath，它会自己判断并找到最佳路径
        this.ytDlpPath = AppConfig.getBundledBinaryPath("yt-dlp.exe");
        this.ffmpegPath = AppConfig.getBundledBinaryPath("ffmpeg.exe");
    }

    // --- Getters ---
    public String getDownloadPath() {
        return downloadPath;
    }

    public String getYtDlpPath() {
        return ytDlpPath;
    }

    public String getFfmpegPath() {
        return ffmpegPath;
    }

    // --- Setters ---
    public void setDownloadPath(String downloadPath) {
        this.downloadPath = downloadPath;
    }

    public void setYtDlpPath(String ytDlpPath) {
        this.ytDlpPath = ytDlpPath;
    }

    public void setFfmpegPath(String ffmpegPath) {
        this.ffmpegPath = ffmpegPath;
    }
}
