// src/main/java/com/bilibili/musicplayer/util/VlcjManager.java
package com.bilibili.musicplayer.util;

import uk.co.caprica.vlcj.player.MediaPlayerFactory;
import uk.co.caprica.vlcj.player.embedded.EmbeddedMediaPlayer;
import uk.co.caprica.vlcj.runtime.RuntimeUtil;
import com.sun.jna.NativeLibrary;

import java.io.File;
import java.nio.file.Paths;

public class VlcjManager {

    private static MediaPlayerFactory mediaPlayerFactory;
    private static EmbeddedMediaPlayer mediaPlayer;
    private static volatile boolean initialized = false;

    public static synchronized boolean initialize() {
        if (initialized) {
            return true;
        }

        String vlcLibPath = null;
        String vlcPluginPath = null;

        // 1. 优先检查 JVM 参数是否已设置 jna.library.path (由 Launch4j 设置)
        String jvmJnaLibraryPath = System.getProperty("jna.library.path");
        if (jvmJnaLibraryPath != null && !jvmJnaLibraryPath.isEmpty()) {
            vlcLibPath = jvmJnaLibraryPath;
            System.out.println("VlcjManager: Using VLC library path from JVM argument: " + vlcLibPath);
        } else {
            // 2. 如果 JVM 参数未设置，尝试查找应用程序同级目录下的 'vlc' 文件夹
            String appDir = System.getProperty("user.dir"); // 获取当前工作目录 (exe所在目录)
            File bundledVlcDir = new File(appDir, "vlc");
            if (bundledVlcDir.exists() && bundledVlcDir.isDirectory()) {
                vlcLibPath = bundledVlcDir.getAbsolutePath();
                System.out.println("VlcjManager: Using bundled VLC library path: " + vlcLibPath);
            } else {
                // 3. 如果捆绑的VLC也未找到，回退到系统路径查找 (主要用于开发环境)
                System.err.println("VlcjManager: Warning: Neither JVM argument for jna.library.path nor bundled 'vlc' directory found. Attempting system-wide VLC discovery...");
                vlcLibPath = findSystemVlcPath(); // 调用辅助方法查找系统VLC
                if (vlcLibPath != null) {
                    System.out.println("VlcjManager: Using system-wide VLC library path: " + vlcLibPath);
                } else {
                    System.err.println("VlcjManager: Error: VLC installation directory not found automatically. " +
                            "Please ensure VLC is installed or bundled correctly.");
                    return false;
                }
            }
        }

        // 设置 jna.library.path 和 VLC_PLUGIN_PATH
        if (vlcLibPath != null) {
            // 确保 jna.library.path 已经被设置到 NativeLibrary
            NativeLibrary.addSearchPath(RuntimeUtil.getLibVlcLibraryName(), vlcLibPath);
            // 推断 VLC_PLUGIN_PATH
            if (RuntimeUtil.isWindows()) {
                // Windows 下，插件路径通常是 libvlc.dll 所在目录的子目录 "plugins"
                vlcPluginPath = Paths.get(vlcLibPath, "plugins").toAbsolutePath().toString();
                if (new File(vlcPluginPath).exists()) {
                    System.setProperty("VLC_PLUGIN_PATH", vlcPluginPath);
                    System.out.println("VlcjManager: Set VLC_PLUGIN_PATH to: " + vlcPluginPath);
                } else {
                    System.err.println("VlcjManager: Warning: VLC_PLUGIN_PATH could not be inferred from " + vlcLibPath + " because 'plugins' directory does not exist.");
                }
            } else {
                System.out.println("VlcjManager: For non-Windows, VLCclea_PLUGIN_PATH usually not explicitly set or handled by VLC itself.");
            }
        } else {
            System.err.println("VlcjManager: Critical Error: VLC library path could not be determined.");
            return false;
        }


        String[] vlcArgs = {
                "--no-video",
                "--ignore-config",
                "--quiet",
                "--no-osd",
                "--no-spu",
                "--no-stats",
                "--no-metadata-network-access",
                "--file-caching=200",
                //"--no-lua",   //vlc3才需要这个,vlc2不用
                //"--verbose=3" // 调试时可以打开
        };

        try {
            // 确保 NativeLibrary 已经设置了搜索路径，然后创建 MediaPlayerFactory
            mediaPlayerFactory = new MediaPlayerFactory(vlcArgs);
            mediaPlayer = mediaPlayerFactory.newEmbeddedMediaPlayer();
            initialized = true;
            System.out.println("VlcjManager: vlcj MediaPlayerFactory and EmbeddedMediaPlayer initialized successfully.");
            return true;
        } catch (Throwable t) {
            System.err.println("VlcjManager: Error initializing vlcj: " + t.getMessage());
            t.printStackTrace();
            release();
            return false;
        }
    }

    // 辅助方法：查找系统安装的 VLC 路径
    private static String findSystemVlcPath() {
        if (RuntimeUtil.isWindows()) {
            String[] possiblePaths = {
                    "C:\\Program Files\\VideoLAN\\VLC",
                    "C:\\Program Files (x86)\\VideoLAN\\VLC"
            };
            for (String path : possiblePaths) {
                // 在Windows上，vlcj通常需要的是VLC安装目录本身，而不是bin或lib子目录
                if (new File(path).exists() && new File(path, "libvlc.dll").exists()) {
                    return path;
                }
            }
        } else if (RuntimeUtil.isMac()) {
            String[] possiblePaths = {
                    "/Applications/VLC.app/Contents/MacOS/lib", // VLC 3.x+
                    "/Applications/VLC.app/Contents/MacOS"      // Older VLC or specific setups
            };
            for (String path : possiblePaths) {
                if (new File(path).exists()) {
                    return path;
                }
            }
        } else { // Linux
            String[] possiblePaths = {
                    "/usr/lib/vlc",
                    "/usr/lib/x86_64-linux-gnu/vlc",
                    "/usr/local/lib/vlc",
                    "/usr/lib" // 通用库路径，可能包含vlc相关symlink
            };
            for (String path : possiblePaths) {
                if (new File(path).exists()) {
                    return path;
                }
            }
        }
        return null;
    }

    public static EmbeddedMediaPlayer getMediaPlayer() {
        if (!initialized || mediaPlayer == null) {
            throw new IllegalStateException("VlcjManager has not been initialized successfully. Call initialize() first and check its return value.");
        }
        return mediaPlayer;
    }

    public static synchronized void release() {
        if (mediaPlayer != null) {
            mediaPlayer.release();
            mediaPlayer = null;
            System.out.println("VlcjManager: vlcj EmbeddedMediaPlayer released.");
        }
        if (mediaPlayerFactory != null) {
            mediaPlayerFactory.release();
            mediaPlayerFactory = null;
            System.out.println("VlcjManager: vlcj MediaPlayerFactory released.");
        }
        initialized = false;
    }

    public static boolean isInitialized() {
        return initialized;
    }
}
