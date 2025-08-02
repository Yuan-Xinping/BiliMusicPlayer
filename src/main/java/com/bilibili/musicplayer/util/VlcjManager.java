// src/main/java/com/bilibili/musicplayer/util/VlcjManager.java
package com.bilibili.musicplayer.util;

import uk.co.caprica.vlcj.player.MediaPlayerFactory;
import uk.co.caprica.vlcj.player.embedded.EmbeddedMediaPlayer;
import uk.co.caprica.vlcj.runtime.RuntimeUtil;

import java.io.File;

public class VlcjManager {

    private static MediaPlayerFactory mediaPlayerFactory;
    private static EmbeddedMediaPlayer mediaPlayer;
    private static volatile boolean initialized = false;

    public static synchronized boolean initialize() {
        if (initialized) {
            return true;
        }

        String jnaLibraryPath = System.getProperty("jna.library.path");
        String vlcPluginPath = System.getProperty("VLC_PLUGIN_PATH");

        if (jnaLibraryPath == null || jnaLibraryPath.isEmpty()) {
            String vlcInstallDir = null;

            if (RuntimeUtil.isWindows()) {
                String[] possiblePaths = {
                        "C:\\Program Files\\VideoLAN\\VLC",
                        "C:\\Program Files (x86)\\VideoLAN\\VLC"
                };
                for (String path : possiblePaths) {
                    if (new File(path).exists()) {
                        vlcInstallDir = path;
                        break;
                    }
                }
            } else if (RuntimeUtil.isMac()) {
                String[] possiblePaths = {
                        "/Applications/VLC.app/Contents/MacOS/lib",
                        "/Applications/VLC.app/Contents/MacOS"
                };
                for (String path : possiblePaths) {
                    if (new File(path).exists()) {
                        vlcInstallDir = path;
                        break;
                    }
                }
            } else { // Linux
                String[] possiblePaths = {
                        "/usr/lib/vlc",
                        "/usr/lib/x86_64-linux-gnu/vlc",
                        "/usr/local/lib/vlc",
                        "/usr/lib"
                };
                for (String path : possiblePaths) {
                    if (new File(path).exists()) {
                        vlcInstallDir = path;
                        break;
                    }
                }
            }

            if (vlcInstallDir != null) {
                System.setProperty("jna.library.path", vlcInstallDir);
                if (RuntimeUtil.isWindows()) {
                    System.setProperty("VLC_PLUGIN_PATH", vlcInstallDir + "\\plugins");
                }
            } else {
                System.err.println("Warning: VLC installation directory not found automatically. " +
                        "Please ensure VLC is installed and consider setting -Djna.library.path JVM option.");
                return false;
            }
        } else {
            if (RuntimeUtil.isWindows() && (vlcPluginPath == null || vlcPluginPath.isEmpty())) {
                String inferredPluginPath = jnaLibraryPath + "\\plugins";
                if (new File(inferredPluginPath).exists()) {
                    System.setProperty("VLC_PLUGIN_PATH", inferredPluginPath);
                } else {
                    System.err.println("Warning: VLC_PLUGIN_PATH not set and could not be inferred from jna.library.path. " +
                            "Ensure plugins are in " + jnaLibraryPath + "\\plugins or set VLC_PLUGIN_PATH explicitly.");
                }
            }
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
                "--no-lua",
                //"--verbose=3"
        };



        try {
            mediaPlayerFactory = new MediaPlayerFactory(vlcArgs);
            mediaPlayer = mediaPlayerFactory.newEmbeddedMediaPlayer();
            initialized = true;
            System.out.println("vlcj MediaPlayerFactory and EmbeddedMediaPlayer initialized.");
            return true;
        } catch (Throwable t) {
            System.err.println("Error initializing vlcj: " + t.getMessage());
            t.printStackTrace();
            release();
            return false;
        }
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
            System.out.println("vlcj EmbeddedMediaPlayer released.");
        }
        if (mediaPlayerFactory != null) {
            mediaPlayerFactory.release();
            mediaPlayerFactory = null;
            System.out.println("vlcj MediaPlayerFactory released.");
        }
        initialized = false;
    }

    public static boolean isInitialized() {
        return initialized;
    }
}

