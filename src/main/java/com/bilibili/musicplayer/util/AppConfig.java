package com.bilibili.musicplayer.util;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.SerializationFeature;

import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;
import java.net.URL;
import com.bilibili.musicplayer.MainApplication;

public class AppConfig {

    private static final ObjectMapper objectMapper = new ObjectMapper();
    static {
        objectMapper.enable(SerializationFeature.INDENT_OUTPUT);
    }

    private static final String CONFIG_DIR_NAME = "BiliMusicPlayer";
    private static final String CONFIG_FILE_NAME = "config.json";
    private static final File CONFIG_FILE;

    private static AppConfigData configData;

    static {
        File configDir = new File(System.getProperty("user.home"), CONFIG_DIR_NAME);
        if (!configDir.exists()) {
            configDir.mkdirs();
        }
        CONFIG_FILE = new File(configDir, CONFIG_FILE_NAME);
        loadConfig();
    }

    private static void loadConfig() {
        if (CONFIG_FILE.exists()) {
            try {
                configData = objectMapper.readValue(CONFIG_FILE, AppConfigData.class);
                System.out.println("AppConfig loaded from: " + CONFIG_FILE.getAbsolutePath());
            } catch (IOException e) {
                System.err.println("Error loading config from " + CONFIG_FILE.getAbsolutePath() + ": " + e.getMessage());
                configData = new AppConfigData(); // Fallback to default if load fails
                System.out.println("Falling back to default config.");
                saveConfig(); // Save newly created default config
            }
        } else {
            configData = new AppConfigData(); // Create default if file not found
            System.out.println("Config file not found, creating default config.");
            saveConfig(); // Save newly created default config
        }
    }

    public static void saveConfig() {
        try {
            objectMapper.writeValue(CONFIG_FILE, configData);
            System.out.println("AppConfig saved to: " + CONFIG_FILE.getAbsolutePath());
        } catch (IOException e) {
            System.err.println("Error saving config to " + CONFIG_FILE.getAbsolutePath() + ": " + e.getMessage());
        }
    }

    public static String getDownloadPath() {
        return configData.getDownloadPath();
    }

    public static String getYtDlpPath() {
        return configData.getYtDlpPath();
    }

    public static String getFfmpegPath() {
        return configData.getFfmpegPath();
    }

    public static void setDownloadPath(String path) {
        configData.setDownloadPath(path);
        saveConfig();
    }

    public static void setYtDlpPath(String path) {
        configData.setYtDlpPath(path);
        saveConfig();
    }

    public static void setFfmpegPath(String path) {
        configData.setFfmpegPath(path);
        saveConfig();
    }

    /**
     * 尝试获取应用程序的根目录。
     * 区分 IDE 运行和打包运行。
     * @return 应用程序根目录的 File 对象。
     */
    private static File getApplicationRoot() {
        try {
            URL location = MainApplication.class.getProtectionDomain().getCodeSource().getLocation();
            File codeSourceFile = new File(location.toURI());

            // 调试打印：
            System.out.println("DEBUG: Code source location URI: " + location.toURI());
            System.out.println("DEBUG: Code source file: " + codeSourceFile.getAbsolutePath());

            // 如果是从 JAR 包运行 (打包后的应用)
            if (codeSourceFile.isFile() && codeSourceFile.getName().toLowerCase().endsWith(".jar")) {
                // JAR 包在 BiliMusicPlayer_Release/BiliMusicPlayer-1.0-SNAPSHOT.jar
                // 它的父目录就是 BiliMusicPlayer_Release，这正是我们需要的根目录
                File appRoot = codeSourceFile.getParentFile();
                System.out.println("DEBUG: Detected app root (from JAR): " + appRoot.getAbsolutePath());
                return appRoot;
            }

            // 如果是从 IDE 运行 (例如 target/classes)
            if (codeSourceFile.isDirectory() && codeSourceFile.getName().equals("classes")) {
                // 从 target/classes 向上两级到项目根目录
                File appRoot = codeSourceFile.getParentFile().getParentFile();
                System.out.println("DEBUG: Detected app root (from IDE classes): " + appRoot.getAbsolutePath());
                return appRoot;
            }

            // 其他情况的默认回退，可能直接就是项目根目录
            System.out.println("DEBUG: Detected app root (fallback to code source): " + codeSourceFile.getAbsolutePath());
            return codeSourceFile;

        } catch (URISyntaxException e) {
            System.err.println("Error resolving application root URI: " + e.getMessage());
        }
        // 最终回退：如果以上方法都失败，使用用户当前工作目录（对于打包的 EXE，通常是 EXE 所在的目录）
        System.err.println("Could not determine application root reliably. Falling back to user.dir.");
        File userDir = new File(System.getProperty("user.dir"));
        System.out.println("DEBUG: Detected app root (fallback to user.dir): " + userDir.getAbsolutePath());
        return userDir;
    }

    /**
     * 尝试获取捆绑在应用程序安装目录中的二进制文件的路径。
     * 该方法会根据运行环境（IDE或打包应用）尝试不同的查找路径。
     *
     * @param binaryName 二进制文件的名称 (例如 "yt-dlp.exe" 或 "ffmpeg.exe")
     * @return 捆绑二进制文件的绝对路径，如果找不到则返回空字符串。
     */
    public static String getBundledBinaryPath(String binaryName) {
        File appRoot = getApplicationRoot(); // 获取应用程序的根目录

        // 1. 优先检查 appRoot/bin/binaryName (适用于打包后的应用)
        File packagedBinaryPath = new File(appRoot, "bin" + File.separator + binaryName);
        if (packagedBinaryPath.exists() && packagedBinaryPath.isFile()) {
            System.out.println("Found bundled binary (appRoot/bin): " + packagedBinaryPath.getAbsolutePath());
            return packagedBinaryPath.getAbsolutePath();
        }

        // 2. 检查 appRoot/target/bin/binaryName (适用于 Maven 开发环境，如果 appRoot 是项目根目录)
        File targetBinaryPath = new File(appRoot, "target" + File.separator + "bin" + File.separator + binaryName);
        if (targetBinaryPath.exists() && targetBinaryPath.isFile()) {
            System.out.println("Found bundled binary (appRoot/target/bin): " + targetBinaryPath.getAbsolutePath());
            return targetBinaryPath.getAbsolutePath();
        }

        // 3. 检查 appRoot/external_binaries/win/binaryName (适用于 IDE 开发环境，如果 appRoot 是项目根目录)
        File sourceBinaryPath = new File(appRoot, "external_binaries" + File.separator + "win" + File.separator + binaryName);
        if (sourceBinaryPath.exists() && sourceBinaryPath.isFile()) {
            System.out.println("Found bundled binary (appRoot/external_binaries): " + sourceBinaryPath.getAbsolutePath());
            return sourceBinaryPath.getAbsolutePath();
        }

        System.err.println("Bundled binary NOT found at any expected location for: " + binaryName + ". App Root was: " + appRoot.getAbsolutePath());
        return ""; // 所有尝试都失败，返回空字符串
    }
}
