// src/main/java/com/bilibili/musicplayer/util/DatabaseManager.java
package com.bilibili.musicplayer.util;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

public class DatabaseManager {

    private static final String DATABASE_NAME = "bili_music_player.db";
    private static final String JDBC_URL = "jdbc:sqlite:" + DATABASE_NAME;

    private static Connection connection = null;

    public static Connection getConnection() throws SQLException {
        if (connection == null || connection.isClosed()) {
            try {
                Class.forName("org.sqlite.JDBC");
                connection = DriverManager.getConnection(JDBC_URL);
                System.out.println("数据库连接成功: " + JDBC_URL);
                createTables(); // 确保表存在
            } catch (ClassNotFoundException e) {
                System.err.println("SQLite JDBC 驱动未找到。请检查 pom.xml 配置。");
                throw new SQLException("SQLite JDBC Driver not found.", e);
            } catch (SQLException e) {
                System.err.println("数据库连接失败: " + e.getMessage());
                throw e;
            }
        }
        return connection;
    }

    private static void createTables() throws SQLException {
        // 更新 songs 表：添加 is_favorite 字段
        String createSongsTableSQL = "CREATE TABLE IF NOT EXISTS songs (" +
                "id TEXT PRIMARY KEY," +
                "title TEXT NOT NULL," +
                "artist TEXT," +
                "bilibili_url TEXT NOT NULL," +
                "local_file_path TEXT NOT NULL," +
                "cover_url TEXT," +
                "duration_seconds INTEGER," +
                "download_date TEXT NOT NULL," +
                "is_favorite INTEGER DEFAULT 0" + // 新增字段，0为否，1为是
                ");";

        // 新增 playlists 表
        String createPlaylistsTableSQL = "CREATE TABLE IF NOT EXISTS playlists (" +
                "id TEXT PRIMARY KEY," +
                "name TEXT NOT NULL UNIQUE," + // 播放列表名称唯一
                "description TEXT" +
                ");";

        // 新增 playlist_songs 关联表（多对多关系）
        String createPlaylistSongsTableSQL = "CREATE TABLE IF NOT EXISTS playlist_songs (" +
                "playlist_id TEXT NOT NULL," +
                "song_id TEXT NOT NULL," +
                "PRIMARY KEY (playlist_id, song_id)," + // 复合主键
                "FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE," + // 级联删除
                "FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE" + // 级联删除
                ");";

        try (Statement stmt = connection.createStatement()) {
            stmt.execute(createSongsTableSQL);
            stmt.execute(createPlaylistsTableSQL);
            stmt.execute(createPlaylistSongsTableSQL);
            System.out.println("所有数据库表检查或创建成功。");
        } catch (SQLException e) {
            System.err.println("创建数据库表失败: " + e.getMessage());
            throw e;
        }
    }

    public static void closeConnection() {
        if (connection != null) {
            try {
                connection.close();
                System.out.println("数据库连接已关闭。");
            } catch (SQLException e) {
                System.err.println("关闭数据库连接失败: " + e.getMessage());
            }
        }
    }
}
