// src/main/java/com/bilibili/musicplayer/service/MediaPlayerService.java
package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.util.VlcjManager;
import javafx.application.Platform;
import javafx.beans.property.*;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.concurrent.Task;

import uk.co.caprica.vlcj.player.MediaPlayer;
import uk.co.caprica.vlcj.player.MediaPlayerEventAdapter;
import uk.co.caprica.vlcj.player.embedded.EmbeddedMediaPlayer;

import java.io.File;
import java.util.List;
import java.util.Random;

public class MediaPlayerService {

    private final EmbeddedMediaPlayer mediaPlayer;

    // UI 绑定属性
    private final BooleanProperty playing = new SimpleBooleanProperty(false);
    private final DoubleProperty progress = new SimpleDoubleProperty(0.0); // 0.0 to 1.0
    private final StringProperty currentTimeText = new SimpleStringProperty("00:00");
    private final StringProperty totalTimeText = new SimpleStringProperty("00:00");
    private final IntegerProperty volume = new SimpleIntegerProperty(100); // 0-200
    private final ObjectProperty<Song> currentSong = new SimpleObjectProperty<>(null);

    private final ObservableList<Song> currentPlaylist = FXCollections.observableArrayList();
    private final IntegerProperty currentSongIndex = new SimpleIntegerProperty(-1);

    private final BooleanProperty loopMode = new SimpleBooleanProperty(false);
    private final BooleanProperty shuffleMode = new SimpleBooleanProperty(false);
    private final Random random = new Random();

    // 预加载相关
    private Song preloadedSong = null;
    private boolean isPreloading = false;
    private int originalVolume = 100; // 存储预加载前的音量，以便恢复

    // 计时器变量
    private long playMediaCallTime = 0;

    public MediaPlayerService() {
        this.mediaPlayer = VlcjManager.getMediaPlayer();

        mediaPlayer.addMediaPlayerEventListener(new MediaPlayerEventAdapter() {
            @Override
            public void playing(MediaPlayer mp) {
                if (isPreloading) {
                    System.out.println("MediaPlayerService: Preloading playing event (suppressed UI update).");
                    Platform.runLater(() -> {
                        mp.pause();
                        mp.setVolume(0); // 确保静音
                        System.out.println("MediaPlayerService: Preloading paused and muted.");
                    });
                } else {
                    Platform.runLater(() -> {
                        playing.set(true);
                        long endTime = System.nanoTime();
                        long durationMs = (endTime - playMediaCallTime) / 1_000_000;
                        System.out.println("MediaPlayerService: Playing started. Load time from playMedia call to playing event: " + durationMs + " ms");
                    });
                    System.out.println("MediaPlayerService: Playing started.");
                }
            }

            @Override
            public void paused(MediaPlayer mp) {
                if (isPreloading) {
                    System.out.println("MediaPlayerService: Preloading paused event (suppressed UI update).");
                } else {
                    Platform.runLater(() -> {
                        playing.set(false);
                    });
                    System.out.println("MediaPlayerService: Paused.");
                }
            }

            @Override
            public void stopped(MediaPlayer mp) {
                Platform.runLater(() -> {
                    playing.set(false);
                    progress.set(0.0);
                    currentTimeText.set("00:00");
                });
                System.out.println("MediaPlayerService: Stopped.");
                preloadedSong = null;
                isPreloading = false;
            }

            @Override
            public void finished(MediaPlayer mp) {
                Platform.runLater(() -> {
                    System.out.println("MediaPlayerService: Song finished. Current loopMode: " + loopMode.get());
                    if (loopMode.get()) { // 单曲循环
                        Song songToReplay = currentSong.get();
                        if (songToReplay != null) {
                            playSongInternal(songToReplay); // <-- 修改这里
                        } else {
                            stop();
                        }
                    } else { // 列表循环或随机播放
                        playNext();
                    }
                });
            }

            @Override
            public void error(MediaPlayer mp) {
                Platform.runLater(() -> {
                    playing.set(false);
                });
                System.err.println("MediaPlayerService: Playback error.");
            }

            @Override
            public void timeChanged(MediaPlayer mp, long newTime) {
                if (!isPreloading) {
                    Platform.runLater(() -> {
                        currentTimeText.set(formatTime(newTime));
                        long totalTime = mp.getLength();
                        if (totalTime > 0) {
                            progress.set((double) newTime / totalTime);
                        }
                    });
                }
            }

            @Override
            public void lengthChanged(MediaPlayer mp, long newLength) {
                if (!isPreloading) {
                    Platform.runLater(() -> {
                        totalTimeText.set(formatTime(newLength));
                    });
                }
            }
        });

        volume.addListener((obs, oldVal, newVal) -> {
            originalVolume = newVal.intValue();
            if (!isPreloading) {
                mediaPlayer.setVolume(newVal.intValue());
            }
        });
        mediaPlayer.setVolume(volume.get());
    }

    /**
     * 播放指定歌曲的公共入口，处理文件存在性检查并在后台线程执行。
     * 外部（如 Controller）应调用此方法。
     * @param song 要播放的歌曲对象
     */
    public void playSong(Song song) {
        Task<Void> playTask = new Task<Void>() {
            @Override // <-- 确保这里有 @Override
            protected Void call() throws Exception {
                // 在后台线程执行文件存在性检查
                if (song == null || song.getLocalFilePath() == null) {
                    Platform.runLater(() -> {
                        System.err.println("MediaPlayerService: Invalid song or file path is null: " + (song != null ? song.getLocalFilePath() : "null"));
                        // 可以在这里显示一个 UI 提示
                    });
                    return null;
                }
                File mediaFile = new File(song.getLocalFilePath());
                if (!mediaFile.exists()) {
                    Platform.runLater(() -> {
                        System.err.println("MediaPlayerService: File not found: " + song.getLocalFilePath());
                        // 可以在这里显示一个 UI 提示
                    });
                    return null;
                }

                // 文件存在，切换回 JavaFX Application Thread 执行实际播放
                Platform.runLater(() -> {
                    playSongInternal(song);
                });
                return null;
            }
        };
        new Thread(playTask).start(); // <-- 确保这里是 java.lang.Thread
    }

    /**
     * 实际执行 VLCJ 播放操作的方法，应在 JavaFX Application Thread 上调用。
     * @param song 要播放的歌曲对象
     */
    private void playSongInternal(Song song) {
        // 如果要播放的歌曲就是当前预加载的歌曲
        if (preloadedSong != null && preloadedSong.equals(song) && isPreloading) {
            System.out.println("MediaPlayerService: Playing preloaded song: " + song.getTitle());
            isPreloading = false; // 切换到非预加载模式
            mediaPlayer.setVolume(originalVolume); // 恢复音量
            mediaPlayer.play(); // 直接播放
            Platform.runLater(() -> {
                currentSong.set(song);
                int index = currentPlaylist.indexOf(song);
                if (index != -1) {
                    currentSongIndex.set(index);
                } else {
                    currentPlaylist.add(song);
                    currentSongIndex.set(currentPlaylist.size() - 1);
                }
            });
        } else {
            // 停止当前播放或预加载
            mediaPlayer.stop(); // 这会触发 stopped 事件，清除 isPreloading 和 preloadedSong

            playMediaCallTime = System.nanoTime(); // 在调用 playMedia 前记录时间

            // 播放新媒体
            boolean success = mediaPlayer.playMedia(song.getLocalFilePath());
            if (success) {
                Platform.runLater(() -> {
                    currentSong.set(song); // 更新当前播放歌曲对象
                    int index = currentPlaylist.indexOf(song);
                    if (index != -1) {
                        currentSongIndex.set(index);
                    } else {
                        currentPlaylist.add(song);
                        currentSongIndex.set(currentPlaylist.size() - 1);
                    }
                });
                System.out.println("MediaPlayerService: Playing " + song.getTitle() + " from " + song.getLocalFilePath());
            } else {
                System.err.println("MediaPlayerService: Failed to start media " + song.getLocalFilePath());
            }
        }
    }

    public void togglePlayPause() {
        if (mediaPlayer.isPlaying()) {
            mediaPlayer.pause();
        } else {
            if (currentSong.get() == null && !currentPlaylist.isEmpty()) {
                playSongInternal(currentPlaylist.get(0)); // <-- 修改这里
            } else if (currentSong.get() != null) {
                mediaPlayer.play();
            } else {
                System.out.println("MediaPlayerService: No song to play.");
            }
        }
    }

    public void stop() {
        mediaPlayer.stop();
        Platform.runLater(() -> {
            currentSong.set(null);
            currentSongIndex.set(-1);
        });
    }

    /**
     * 播放下一首歌曲
     */
    public void playNext() {
        if (currentPlaylist.isEmpty()) {
            System.out.println("MediaPlayerService: Playlist is empty. Cannot play next song.");
            stop();
            return;
        }

        int nextIndex;
        if (shuffleMode.get()) {
            nextIndex = random.nextInt(currentPlaylist.size());
            if (currentPlaylist.size() > 1 && nextIndex == currentSongIndex.get()) {
                nextIndex = (nextIndex + 1) % currentPlaylist.size();
            }
        } else {
            nextIndex = currentSongIndex.get() + 1;
            if (nextIndex >= currentPlaylist.size()) {
                nextIndex = 0;
            }
        }

        if (nextIndex >= 0 && nextIndex < currentPlaylist.size()) {
            Song nextSong = currentPlaylist.get(nextIndex);
            playSongInternal(nextSong); // <-- 修改这里
            // 在这里，你可以考虑预加载再下一首歌曲
            if (currentPlaylist.size() > 1) {
                int nextNextIndex = (nextIndex + 1) % currentPlaylist.size();
                preloadSong(currentPlaylist.get(nextNextIndex));
            }
        } else {
            stop();
        }
    }

    /**
     * 播放上一首歌曲
     */
    public void playPrevious() {
        if (currentPlaylist.isEmpty()) {
            System.out.println("MediaPlayerService: Playlist is empty. Cannot play previous song.");
            stop();
            return;
        }

        int prevIndex;
        if (shuffleMode.get()) {
            prevIndex = random.nextInt(currentPlaylist.size());
            if (currentPlaylist.size() > 1 && prevIndex == currentSongIndex.get()) {
                prevIndex = (prevIndex - 1 + currentPlaylist.size()) % currentPlaylist.size();
            }
        } else {
            prevIndex = currentSongIndex.get() - 1;
            if (prevIndex < 0) {
                prevIndex = currentPlaylist.size() - 1;
            }
        }

        if (prevIndex >= 0 && prevIndex < currentPlaylist.size()) {
            playSongInternal(currentPlaylist.get(prevIndex)); // <-- 修改这里
        } else {
            stop();
        }
    }

    /**
     * 预加载下一首歌曲。
     * 应该在当前歌曲播放时，或者在播放列表切换时调用。
     * @param songToPreload 要预加载的歌曲对象
     */
    public void preloadSong(Song songToPreload) {
        // 预加载也可能涉及文件检查，为了避免阻塞 UI，这里也将其放在 Task 中
        Task<Void> preloadTask = new Task<Void>() {
            @Override
            protected Void call() throws Exception {
                if (songToPreload == null || songToPreload.getLocalFilePath() == null) {
                    System.err.println("MediaPlayerService: Invalid song for preloading or file not found: " + (songToPreload != null ? songToPreload.getLocalFilePath() : "null"));
                    return null;
                }
                // 如果当前正在播放，或者已经预加载了这首歌，或者预加载的是当前正在播放的歌，则不进行预加载
                if (mediaPlayer.isPlaying() || (preloadedSong != null && preloadedSong.equals(songToPreload)) || (currentSong.get() != null && currentSong.get().equals(songToPreload))) {
                    System.out.println("MediaPlayerService: Skipping preload, already playing or preloaded: " + songToPreload.getTitle());
                    return null;
                }

                File mediaFile = new File(songToPreload.getLocalFilePath());
                if (!mediaFile.exists()) {
                    System.err.println("MediaPlayerService: Preload failed, file not found: " + songToPreload.getLocalFilePath());
                    return null;
                }

                System.out.println("MediaPlayerService: Attempting to preload: " + songToPreload.getTitle());

                // 切换回 JavaFX Application Thread 执行实际的 VLCJ 预加载操作
                Platform.runLater(() -> {
                    // 停止当前可能存在的预加载或播放
                    mediaPlayer.stop(); // 这会清除 isPreloading 和 preloadedSong

                    // 设置预加载标志
                    isPreloading = true;
                    preloadedSong = songToPreload;

                    // 静音并开始播放，然后立即暂停，以触发缓冲
                    mediaPlayer.setVolume(0); // 预加载时静音
                    boolean success = mediaPlayer.playMedia(songToPreload.getLocalFilePath());

                    if (!success) {
                        System.err.println("MediaPlayerService: Failed to start preloading media " + songToPreload.getLocalFilePath());
                        isPreloading = false;
                        preloadedSong = null;
                        mediaPlayer.setVolume(originalVolume); // 恢复音量
                    }
                });
                return null;
            }
        };
        new Thread(preloadTask).start();
    }

    /**
     * 跳转到指定位置。
     * @param position 0.0 (开头) 到 1.0 (结尾)
     */
    public void seek(float position) {
        mediaPlayer.setPosition(position);
    }

    // --- Getters for JavaFX Properties ---
    public BooleanProperty playingProperty() {
        return playing;
    }

    public DoubleProperty progressProperty() {
        return progress;
    }

    public StringProperty currentTimeTextProperty() {
        return currentTimeText;
    }

    public StringProperty totalTimeTextProperty() {
        return totalTimeText;
    }

    public IntegerProperty volumeProperty() {
        return volume;
    }

    public ObjectProperty<Song> currentSongProperty() {
        return currentSong;
    }

    public void setLoopMode(boolean loop) {
        this.loopMode.set(loop);
        if (loop) { // 如果开启单曲循环，关闭随机播放
            this.shuffleMode.set(false);
        }
    }
    public void setShuffleMode(boolean shuffle) {
        this.shuffleMode.set(shuffle);
        if (shuffle) { // 如果开启随机播放，关闭单曲循环
            this.loopMode.set(false);
        }
    }
    public long getVlcjMediaPlayerLength() {
        return mediaPlayer.getLength();
    }
    /**
     * 格式化毫秒为 MM:SS 字符串。
     */
    private String formatTime(long millis) {
        long totalSeconds = millis / 1000;
        long minutes = totalSeconds / 60;
        long seconds = totalSeconds % 60;
        return String.format("%02d:%02d", minutes, seconds);
    }

    // 应用程序关闭时释放VLCJ资源
    public void release() {
        if (mediaPlayer != null) {
            mediaPlayer.release();
        }
        VlcjManager.release(); // 确保释放VlcjManager持有的资源
    }
}
