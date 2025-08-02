// src/main/java/com/bilibili/musicplayer/model/Song.java
package com.bilibili.musicplayer.model;

import java.io.File;
import java.time.LocalDateTime;
import java.util.Objects;

public class Song {
    private String id; // 可以是BVID或yt-dlp生成的唯一ID
    private String title;
    private String artist; // 对应yt-dlp的uploader
    private String bilibiliUrl;
    private String localFilePath; // 本地文件完整路径
    private String coverUrl; // 封面图片的URL
    private long durationSeconds; // 时长，单位秒
    private LocalDateTime downloadDate; // 下载日期
    private boolean isFavorite; // 新增字段：是否喜欢

    public Song(String id, String title, String artist, String bilibiliUrl, String localFilePath, String coverUrl, long durationSeconds, LocalDateTime downloadDate, boolean isFavorite) {
        this.id = id;
        this.title = title;
        this.artist = artist;
        this.bilibiliUrl = bilibiliUrl;
        this.localFilePath = localFilePath;
        this.coverUrl = coverUrl;
        this.durationSeconds = durationSeconds;
        this.downloadDate = downloadDate;
        this.isFavorite = isFavorite; // 初始化
    }

    // 现有构造函数可以重载，或者只保留一个最完整的
    public Song(String id, String title, String artist, String bilibiliUrl, String localFilePath, String coverUrl, long durationSeconds, LocalDateTime downloadDate) {
        this(id, title, artist, bilibiliUrl, localFilePath, coverUrl, durationSeconds, downloadDate, false); // 默认不收藏
    }

    // --- Getters ---
    public String getId() { return id; }
    public String getTitle() { return title; }
    public String getArtist() { return artist; }
    public String getBilibiliUrl() { return bilibiliUrl; }
    public String getLocalFilePath() { return localFilePath; }
    public String getCoverUrl() { return coverUrl; }
    public long getDurationSeconds() { return durationSeconds; }
    public LocalDateTime getDownloadDate() { return downloadDate; }
    public boolean isFavorite() { return isFavorite; } // 新增getter

    // --- Setters ---
    public void setId(String id) { this.id = id; }
    public void setTitle(String title) { this.title = title; }
    public void setArtist(String artist) { this.artist = artist; }
    public void setBilibiliUrl(String bilibiliUrl) { this.bilibiliUrl = bilibiliUrl; }
    public void setLocalFilePath(String localFilePath) { this.localFilePath = localFilePath; }
    public void setCoverUrl(String coverUrl) { this.coverUrl = coverUrl; }
    public void setDurationSeconds(long durationSeconds) { this.durationSeconds = durationSeconds; }
    public void setDownloadDate(LocalDateTime downloadDate) { this.downloadDate = downloadDate; }
    public void setFavorite(boolean favorite) { isFavorite = favorite; } // 新增setter

    @Override
    public String toString() {
        if (artist != null && !artist.isEmpty()) {
            return title + " - " + artist;
        }
        return title;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Song song = (Song) o;
        return Objects.equals(id, song.id);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }
}
