// src/main/java/com/bilibili/musicplayer/service/SongDAO.java
package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.util.DatabaseManager;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;

public class SongDAO {

    /**
     * 将歌曲信息保存到数据库。
     * 如果歌曲ID已存在，则更新现有记录；否则插入新记录。
     * @param song 要保存的歌曲对象
     * @return true 如果保存成功，false 如果失败
     */
    public boolean saveSong(Song song) {
        String insertSQL = "INSERT INTO songs (id, title, artist, bilibili_url, local_file_path, cover_url, duration_seconds, download_date, is_favorite) " +
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
        String updateSQL = "UPDATE songs SET title=?, artist=?, bilibili_url=?, local_file_path=?, cover_url=?, duration_seconds=?, download_date=?, is_favorite=? WHERE id=?";

        try (Connection conn = DatabaseManager.getConnection()) {
            // 尝试查询是否存在该ID的歌曲，使用新的公共方法
            if (getSongById(song.getId()) != null) { // MODIFIED: 使用新的 getSongById 方法
                try (PreparedStatement pstmt = conn.prepareStatement(updateSQL)) {
                    pstmt.setString(1, song.getTitle());
                    pstmt.setString(2, song.getArtist());
                    pstmt.setString(3, song.getBilibiliUrl());
                    pstmt.setString(4, song.getLocalFilePath());
                    pstmt.setString(5, song.getCoverUrl());
                    pstmt.setLong(6, song.getDurationSeconds());
                    pstmt.setString(7, song.getDownloadDate().toString());
                    pstmt.setInt(8, song.isFavorite() ? 1 : 0); // is_favorite
                    pstmt.setString(9, song.getId());
                    int rowsAffected = pstmt.executeUpdate();
                    System.out.println("更新歌曲: " + song.getTitle() + ", 影响行数: " + rowsAffected);
                    return rowsAffected > 0;
                }
            } else {
                try (PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
                    pstmt.setString(1, song.getId());
                    pstmt.setString(2, song.getTitle());
                    pstmt.setString(3, song.getArtist());
                    pstmt.setString(4, song.getBilibiliUrl());
                    pstmt.setString(5, song.getLocalFilePath());
                    pstmt.setString(6, song.getCoverUrl());
                    pstmt.setLong(7, song.getDurationSeconds());
                    pstmt.setString(8, song.getDownloadDate().toString());
                    pstmt.setInt(9, song.isFavorite() ? 1 : 0); // is_favorite
                    int rowsAffected = pstmt.executeUpdate();
                    System.out.println("插入歌曲: " + song.getTitle() + ", 影响行数: " + rowsAffected);
                    return rowsAffected > 0;
                }
            }
        } catch (SQLException e) {
            System.err.println("保存歌曲到数据库失败: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }

    /**
     * 根据ID获取歌曲信息。
     * 此方法内部管理数据库连接。
     * @param id 歌曲ID
     * @return 歌曲对象，如果不存在则返回null
     */
    public Song getSongById(String id) { // NEW: 公共方法，内部管理连接
        String sql = "SELECT * FROM songs WHERE id = ?";
        try (Connection conn = DatabaseManager.getConnection(); // 内部获取连接
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, id);
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return mapResultSetToSong(rs);
                }
            }
        } catch (SQLException e) {
            System.err.println("根据ID获取歌曲失败: " + e.getMessage());
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 根据ID获取歌曲信息。
     * @param id 歌曲ID
     * @param conn 外部传入的数据库连接，用于事务或避免重复连接
     * @return 歌曲对象，如果不存在则返回null
     * @throws SQLException 如果读取数据失败
     */
    public Song getSongById(String id, Connection conn) throws SQLException { // 现有方法，保留用于内部/事务使用
        String sql = "SELECT * FROM songs WHERE id = ?";
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, id);
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return mapResultSetToSong(rs);
                }
            }
        }
        return null;
    }

    /**
     * 获取所有歌曲信息。
     * @return 歌曲列表
     */
    public List<Song> getAllSongs() {
        List<Song> songs = new ArrayList<>();
        String sql = "SELECT * FROM songs ORDER BY download_date DESC";
        try (Connection conn = DatabaseManager.getConnection();
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                songs.add(mapResultSetToSong(rs));
            }
        } catch (SQLException e) {
            System.err.println("从数据库获取所有歌曲失败: " + e.getMessage());
            e.printStackTrace();
        }
        return songs;
    }

    /**
     * 获取所有收藏的歌曲。
     * @return 收藏歌曲列表
     */
    public List<Song> getFavoriteSongs() {
        List<Song> songs = new ArrayList<>();
        String sql = "SELECT * FROM songs WHERE is_favorite = 1 ORDER BY download_date DESC";
        try (Connection conn = DatabaseManager.getConnection();
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                songs.add(mapResultSetToSong(rs));
            }
        } catch (SQLException e) {
            System.err.println("从数据库获取收藏歌曲失败: " + e.getMessage());
            e.printStackTrace();
        }
        return songs;
    }

    /**
     * 从 ResultSet 映射到 Song 对象。
     * @param rs 结果集
     * @return Song 对象
     * @throws SQLException 如果读取数据失败
     */
    public Song mapResultSetToSong(ResultSet rs) throws SQLException {
        String id = rs.getString("id");
        String title = rs.getString("title");
        String artist = rs.getString("artist");
        String bilibiliUrl = rs.getString("bilibili_url");
        String localFilePath = rs.getString("local_file_path");
        String coverUrl = rs.getString("cover_url");
        long durationSeconds = rs.getLong("duration_seconds");
        LocalDateTime downloadDate = LocalDateTime.parse(rs.getString("download_date"));
        boolean isFavorite = rs.getInt("is_favorite") == 1; // 读取 is_favorite

        return new Song(id, title, artist, bilibiliUrl, localFilePath, coverUrl, durationSeconds, downloadDate, isFavorite);
    }

    /**
     * 删除指定ID的歌曲。
     * @param id 歌曲ID
     * @return true 如果删除成功，false 如果失败
     */
    public boolean deleteSong(String id) {
        String sql = "DELETE FROM songs WHERE id = ?";
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, id);
            int rowsAffected = pstmt.executeUpdate();
            System.out.println("从数据库删除歌曲 (ID: " + id + "), 影响行数: " + rowsAffected);
            return rowsAffected > 0;
        } catch (SQLException e) {
            System.err.println("删除歌曲失败 (ID: " + id + "): " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }
}
