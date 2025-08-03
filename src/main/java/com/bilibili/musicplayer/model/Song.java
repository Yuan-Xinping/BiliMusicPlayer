// src/main/java/com/bilibili/musicplayer/model/Song.java
package com.bilibili.musicplayer.model;

import java.io.File;
import java.time.LocalDateTime;
import java.util.Objects;

public class Song {
    private String id;
    private String title;
    private String artist;
    private String bilibiliUrl;
    private String localFilePath;
    private String coverUrl;
    private long durationSeconds;
    private LocalDateTime downloadDate;
    private boolean isFavorite;

    public Song(String id, String title, String artist, String bilibiliUrl, String localFilePath, String coverUrl, long durationSeconds, LocalDateTime downloadDate, boolean isFavorite) {
        this.id = id;
        this.title = title;
        this.artist = artist;
        this.bilibiliUrl = bilibiliUrl;
        this.localFilePath = localFilePath;
        this.coverUrl = coverUrl;
        this.durationSeconds = durationSeconds;
        this.downloadDate = downloadDate;
        this.isFavorite = isFavorite;
    }

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
