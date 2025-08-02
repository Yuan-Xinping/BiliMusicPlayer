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

    /**
     * 获取数据库连接。如果连接不存在或已关闭，则创建一个新连接。
     * @return 数据库连接对象
     * @throws SQLException 如果连接失败
     */
    public static Connection getConnection() throws SQLException {
        if (connection == null || connection.isClosed()) {
            try {
                // 加载SQLite JDBC驱动
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

    /**
     * 创建数据库表（如果不存在）。
     * 目前只创建 songs 表。
     * @throws SQLException 如果创建表失败
     */
    private static void createTables() throws SQLException {
        String createSongsTableSQL = "CREATE TABLE IF NOT EXISTS songs (" +
                "id TEXT PRIMARY KEY," + // BVID 或 yt-dlp ID
                "title TEXT NOT NULL," +
                "artist TEXT," +
                "bilibili_url TEXT NOT NULL," +
                "local_file_path TEXT NOT NULL," +
                "cover_url TEXT," +
                "duration_seconds INTEGER," +
                "download_date TEXT NOT NULL" + // 存储为 ISO 8601 格式字符串
                ");";

        try (Statement stmt = connection.createStatement()) {
            stmt.execute(createSongsTableSQL);
            System.out.println("表 'songs' 检查或创建成功。");
        } catch (SQLException e) {
            System.err.println("创建表 'songs' 失败: " + e.getMessage());
            throw e;
        }
    }

    /**
     * 关闭数据库连接。
     */
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
