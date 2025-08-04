// src/main/java/com/bilibili/musicplayer/model/Playlist.java
package com.bilibili.musicplayer.model;

import java.util.Objects;
import java.util.UUID;

public class Playlist {
    private String id;
    private String name;
    private String description;

    public Playlist(String id, String name, String description) {
        this.id = id;
        this.name = name;
        this.description = description;
    }

    public Playlist(String name, String description) {
        this(UUID.randomUUID().toString(), name, description); // 自动生成ID
    }

    // --- Getters ---
    public String getId() { return id; }
    public String getName() { return name; }
    public String getDescription() { return description; }

    // --- Setters ---
    public void setId(String id) { this.id = id; }
    public void setName(String name) { this.name = name; }
    public void setDescription(String description) { this.description = description; }

    @Override
    public String toString() {
        return name;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Playlist playlist = (Playlist) o;
        return Objects.equals(id, playlist.id);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }
}
