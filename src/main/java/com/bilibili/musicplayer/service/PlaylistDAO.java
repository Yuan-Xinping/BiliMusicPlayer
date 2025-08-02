// src/main/java/com/bilibili/musicplayer/service/PlaylistDAO.java
package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Playlist;
import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.util.DatabaseManager;

import java.sql.*;
import java.util.ArrayList;
import java.util.List;

public class PlaylistDAO {

    private final SongDAO songDAO; // 用于获取歌曲详情

    public PlaylistDAO() {
        this.songDAO = new SongDAO();
    }

    /**
     * 创建一个新的播放列表。
     * @param playlist 要创建的播放列表对象
     * @return true 如果创建成功，false 如果失败
     */
    public boolean createPlaylist(Playlist playlist) {
        String sql = "INSERT INTO playlists (id, name, description) VALUES (?, ?, ?)";
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, playlist.getId());
            pstmt.setString(2, playlist.getName());
            pstmt.setString(3, playlist.getDescription());
            int rowsAffected = pstmt.executeUpdate();
            System.out.println("创建播放列表: " + playlist.getName() + ", 影响行数: " + rowsAffected);
            return rowsAffected > 0;
        } catch (SQLException e) {
            System.err.println("创建播放列表失败: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }

    /**
     * 获取所有播放列表。
     * @return 播放列表列表
     */
    public List<Playlist> getAllPlaylists() {
        List<Playlist> playlists = new ArrayList<>();
        String sql = "SELECT * FROM playlists ORDER BY name ASC";
        try (Connection conn = DatabaseManager.getConnection();
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            while (rs.next()) {
                playlists.add(new Playlist(
                        rs.getString("id"),
                        rs.getString("name"),
                        rs.getString("description")
                ));
            }
        } catch (SQLException e) {
            System.err.println("获取所有播放列表失败: " + e.getMessage());
            e.printStackTrace();
        }
        return playlists;
    }

    /**
     * 根据ID获取播放列表。
     * @param id 播放列表ID
     * @return 播放列表对象，如果不存在则返回null
     */
    public Playlist getPlaylistById(String id) {
        String sql = "SELECT * FROM playlists WHERE id = ?";
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, id);
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return new Playlist(
                            rs.getString("id"),
                            rs.getString("name"),
                            rs.getString("description")
                    );
                }
            }
        } catch (SQLException e) {
            System.err.println("根据ID获取播放列表失败: " + e.getMessage());
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 更新播放列表信息（目前只支持名称和描述）。
     * @param playlist 要更新的播放列表对象
     * @return true 如果更新成功，false 如果失败
     */
    public boolean updatePlaylist(Playlist playlist) {
        String sql = "UPDATE playlists SET name = ?, description = ? WHERE id = ?";
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, playlist.getName());
            pstmt.setString(2, playlist.getDescription());
            pstmt.setString(3, playlist.getId());
            int rowsAffected = pstmt.executeUpdate();
            System.out.println("更新播放列表: " + playlist.getName() + ", 影响行数: " + rowsAffected);
            return rowsAffected > 0;
        } catch (SQLException e) {
            System.err.println("更新播放列表失败: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }

    /**
     * 删除播放列表及其所有关联歌曲。
     * @param id 播放列表ID
     * @return true 如果删除成功，false 如果失败
     */
    public boolean deletePlaylist(String id) {
        // 首先删除 playlist_songs 表中与该播放列表关联的记录
        String deleteSongsSql = "DELETE FROM playlist_songs WHERE playlist_id = ?";
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmtSongs = conn.prepareStatement(deleteSongsSql)) {
            pstmtSongs.setString(1, id);
            pstmtSongs.executeUpdate(); // 不关心影响行数，因为可能没有关联歌曲
            System.out.println("删除播放列表 (ID: " + id + ") 关联歌曲。");

            // 然后删除 playlists 表中的记录
            String deletePlaylistSql = "DELETE FROM playlists WHERE id = ?";
            try (PreparedStatement pstmtPlaylist = conn.prepareStatement(deletePlaylistSql)) {
                pstmtPlaylist.setString(1, id);
                int rowsAffected = pstmtPlaylist.executeUpdate();
                System.out.println("删除播放列表 (ID: " + id + "), 影响行数: " + rowsAffected);
                return rowsAffected > 0;
            }
        } catch (SQLException e) {
            System.err.println("删除播放列表失败: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }

    /**
     * 将歌曲添加到播放列表。
     * @param playlistId 播放列表ID
     * @param songId 歌曲ID
     * @return true 如果添加成功，false 如果失败（如已存在）
     */
    public boolean addSongToPlaylist(String playlistId, String songId) {
        String sql = "INSERT OR IGNORE INTO playlist_songs (playlist_id, song_id) VALUES (?, ?)"; // IGNORE 避免重复插入
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, playlistId);
            pstmt.setString(2, songId);
            int rowsAffected = pstmt.executeUpdate();
            if (rowsAffected > 0) {
                System.out.println("歌曲 " + songId + " 已添加到播放列表 " + playlistId);
            } else {
                System.out.println("歌曲 " + songId + " 已存在于播放列表 " + playlistId + " (未重复添加)");
            }
            return rowsAffected > 0;
        } catch (SQLException e) {
            System.err.println("添加歌曲到播放列表失败: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }

    /**
     * 从播放列表移除歌曲。
     * @param playlistId 播放列表ID
     * @param songId 歌曲ID
     * @return true 如果移除成功，false 如果失败
     */
    public boolean removeSongFromPlaylist(String playlistId, String songId) {
        String sql = "DELETE FROM playlist_songs WHERE playlist_id = ? AND song_id = ?";
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, playlistId);
            pstmt.setString(2, songId);
            int rowsAffected = pstmt.executeUpdate();
            System.out.println("从播放列表 " + playlistId + " 移除歌曲 " + songId + ", 影响行数: " + rowsAffected);
            return rowsAffected > 0;
        } catch (SQLException e) {
            System.err.println("从播放列表移除歌曲失败: " + e.getMessage());
            e.printStackTrace();
            return false;
        }
    }

    /**
     * 检查歌曲是否已存在于指定播放列表中。
     * @param playlistId 播放列表ID
     * @param songId 歌曲ID
     * @return true 如果歌曲已存在于播放列表中，否则为 false
     */
    public boolean isSongInPlaylist(String playlistId, String songId) {
        String sql = "SELECT COUNT(*) FROM playlist_songs WHERE playlist_id = ? AND song_id = ?";
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, playlistId);
            pstmt.setString(2, songId);
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return rs.getInt(1) > 0; // 如果计数大于0，则表示歌曲存在
                }
            }
        } catch (SQLException e) {
            System.err.println("检查歌曲是否在播放列表中失败: " + e.getMessage());
            e.printStackTrace();
        }
        return false; // 发生异常或未找到，都返回false
    }

    /**
     * 获取指定播放列表中的所有歌曲。
     * @param playlistId 播放列表ID
     * @return 歌曲列表
     */
    public List<Song> getSongsInPlaylist(String playlistId) {
        List<Song> songs = new ArrayList<>();
        String sql = "SELECT s.* FROM songs s " +
                "JOIN playlist_songs ps ON s.id = ps.song_id " +
                "WHERE ps.playlist_id = ? ORDER BY s.title ASC"; // 按标题排序
        try (Connection conn = DatabaseManager.getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, playlistId);
            try (ResultSet rs = pstmt.executeQuery()) {
                while (rs.next()) {
                    songs.add(songDAO.mapResultSetToSong(rs)); // 使用 SongDAO 的映射方法
                }
            }
        } catch (SQLException e) {
            System.err.println("获取播放列表歌曲失败: " + e.getMessage());
            e.printStackTrace();
        }
        return songs;
    }
}
