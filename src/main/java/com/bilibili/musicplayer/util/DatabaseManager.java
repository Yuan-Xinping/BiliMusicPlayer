// src/main/java/com/bilibili/musicplayer/util/DatabaseManager.java
package com.bilibili.musicplayer.util;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

public class DatabaseManager {

    private static final String DB_URL = "jdbc:sqlite:bili_music_player.db";

    // 静态代码块：在类加载时只执行一次，用于加载 JDBC 驱动
    static {
        try {
            Class.forName("org.sqlite.JDBC");
            System.out.println("SQLite JDBC Driver loaded.");
        } catch (ClassNotFoundException e) {
            System.err.println("错误: 无法加载 SQLite JDBC 驱动。请确保 'org.xerial:sqlite-jdbc' 依赖已添加。");
            e.printStackTrace();
            System.exit(1); // 致命错误，退出应用程序
        }
    }

    /**
     * 获取一个新的数据库连接。
     * 每次调用此方法都会返回一个独立的 Connection 对象。
     * 调用者有责任在使用完毕后关闭此连接（通常通过 try-with-resources）。
     *
     * @return 一个新的 Connection 对象。
     * @throws SQLException 如果发生数据库访问错误。
     */
    public static Connection getConnection() throws SQLException {
        Connection conn = DriverManager.getConnection(DB_URL);
        // 对于每个新连接，确保启用外键约束
        try (Statement stmt = conn.createStatement()) {
            stmt.execute("PRAGMA foreign_keys = ON;");
        }
        createTablesIfNotExist(conn); // 传入当前连接，避免再次获取
        return conn;
    }

    /**
     * 检查并创建数据库表（如果不存在）。
     * 此方法应在应用程序启动时调用一次，而不是每次获取连接时都调用。
     * 为了避免在 getConnection 中每次都创建 Statement，将其独立出来并传入连接。
     * @param conn 数据库连接
     * @throws SQLException 如果创建表失败
     */
    private static void createTablesIfNotExist(Connection conn) throws SQLException {
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

        try (Statement stmt = conn.createStatement()) { // 使用传入的连接
            stmt.execute(createSongsTableSQL);
            stmt.execute(createPlaylistsTableSQL);
            stmt.execute(createPlaylistSongsTableSQL);
        }
    }

}
