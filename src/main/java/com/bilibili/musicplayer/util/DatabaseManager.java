// src/main/java/com/bilibili/musicplayer/util/DatabaseManager.java
package com.bilibili.musicplayer.util;

import java.io.File;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;

public class DatabaseManager {

    private static final String DB_FILE_NAME = "bili_music_player.db";
    private static String DB_URL;

    static {
        try {
            Class.forName("org.sqlite.JDBC");
            System.out.println("SQLite JDBC Driver loaded.");

            String appDataDir;
            String os = System.getProperty("os.name").toLowerCase();

            if (os.contains("win")) {

                appDataDir = System.getenv("APPDATA");
                if (appDataDir == null) {
                    appDataDir = System.getProperty("user.home") + File.separator + "AppData" + File.separator + "Roaming";
                }
            } else if (os.contains("mac")) {
                appDataDir = System.getProperty("user.home") + File.separator + "Library" + File.separator + "Application Support";
            } else {
                appDataDir = System.getProperty("user.home") + File.separator + ".config"; // XDG Base Directory Specification
            }

            File appSpecificDir = new File(appDataDir, "BiliMusicPlayer");
            if (!appSpecificDir.exists()) {
                boolean created = appSpecificDir.mkdirs(); // 创建目录（包括父目录）
                if (created) {
                    System.out.println("Created application data directory: " + appSpecificDir.getAbsolutePath());
                } else {
                    System.err.println("Warning: Failed to create application data directory: " + appSpecificDir.getAbsolutePath());
                }
            }

            File dbFile = new File(appSpecificDir, DB_FILE_NAME);
            DB_URL = "jdbc:sqlite:" + dbFile.getAbsolutePath();
            System.out.println("Database path set to: " + dbFile.getAbsolutePath());

        } catch (ClassNotFoundException e) {
            System.err.println("错误: 无法加载 SQLite JDBC 驱动。请确保 'org.xerial:sqlite-jdbc' 依赖已添加。");
            e.printStackTrace();
            throw new RuntimeException("Failed to load SQLite JDBC driver.", e);
        } catch (Exception e) { // 捕获其他可能的异常，如目录创建失败
            System.err.println("错误: 初始化数据库路径失败。");
            e.printStackTrace();
            throw new RuntimeException("Failed to initialize database path.", e);
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
                "is_favorite INTEGER DEFAULT 0" +
                ");";

        // 新增 playlists 表
        String createPlaylistsTableSQL = "CREATE TABLE IF NOT EXISTS playlists (" +
                "id TEXT PRIMARY KEY," +
                "name TEXT NOT NULL UNIQUE," +
                "description TEXT" +
                ");";

        String createPlaylistSongsTableSQL = "CREATE TABLE IF NOT EXISTS playlist_songs (" +
                "playlist_id TEXT NOT NULL," +
                "song_id TEXT NOT NULL," +
                "PRIMARY KEY (playlist_id, song_id)," +
                "FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE," +
                "FOREIGN KEY (song_id) REFERENCES songs(id) ON DELETE CASCADE" +
                ");";

        try (Statement stmt = conn.createStatement()) {
            stmt.execute(createSongsTableSQL);
            stmt.execute(createPlaylistsTableSQL);
            stmt.execute(createPlaylistSongsTableSQL);


            try {
                stmt.execute("ALTER TABLE songs ADD COLUMN is_favorite INTEGER DEFAULT 0;");
                System.out.println("Added 'is_favorite' column to 'songs' table.");
            } catch (SQLException e) {
                if (!e.getMessage().contains("duplicate column name")) {
                    System.err.println("Error adding 'is_favorite' column: " + e.getMessage());
                }
            }

        }
    }

    // 主方法用于测试数据库路径和连接
    public static void main(String[] args) {
        try {
            System.out.println("Attempting to get database connection...");
            Connection conn = DatabaseManager.getConnection();
            if (conn != null) {
                System.out.println("Successfully connected to the database.");
                conn.close();
                System.out.println("Connection closed.");
            }
        } catch (SQLException e) {
            System.err.println("Failed to connect to the database: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
