package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.util.VlcjManager;
import javafx.application.Platform;
import javafx.beans.property.*;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.concurrent.Task;
import javafx.scene.control.Alert;

import uk.co.caprica.vlcj.player.MediaPlayer;
import uk.co.caprica.vlcj.player.MediaPlayerEventAdapter;
import uk.co.caprica.vlcj.player.embedded.EmbeddedMediaPlayer;

import java.io.File;
import java.util.Collections;
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

    // 播放模式枚举
    public enum PlaybackMode {
        REPEAT_ALL,
        REPEAT_ONE,
        SHUFFLE
    }
    private final ObjectProperty<PlaybackMode> playbackMode = new SimpleObjectProperty<>(PlaybackMode.REPEAT_ALL);
    private final Random random = new Random();

    private long playMediaCallTime = 0;

    public MediaPlayerService() {
        this.mediaPlayer = VlcjManager.getMediaPlayer();

        mediaPlayer.addMediaPlayerEventListener(new MediaPlayerEventAdapter() {
            @Override
            public void playing(MediaPlayer mp) {
                Platform.runLater(() -> {
                    playing.set(true);
                    long endTime = System.nanoTime();
                    long durationMs = (endTime - playMediaCallTime) / 1_000_000;
                    System.out.println("MediaPlayerService: Playing started. Load time from playMedia call to playing event: " + durationMs + " ms");
                });
                System.out.println("MediaPlayerService: Playing started.");
            }

            @Override
            public void paused(MediaPlayer mp) {
                Platform.runLater(() -> {
                    playing.set(false);
                });
                System.out.println("MediaPlayerService: Paused.");
            }

            @Override
            public void stopped(MediaPlayer mp) {
                Platform.runLater(() -> {
                    playing.set(false);
                    progress.set(0.0);
                    currentTimeText.set("00:00");
                });
                System.out.println("MediaPlayerService: Stopped.");
            }

            @Override
            public void finished(MediaPlayer mp) {
                Platform.runLater(() -> {
                    System.out.println("MediaPlayerService: Song finished. Current playbackMode: " + playbackMode.get());
                    playNext();
                });
            }

            @Override
            public void error(MediaPlayer mp) {
                Platform.runLater(() -> {
                    playing.set(false);
                    Alert alert = new Alert(Alert.AlertType.ERROR);
                    alert.setTitle("Playback Error");
                    alert.setHeaderText("An error occurred during playback.");
                    alert.setContentText("The media file might be corrupted or unplayable. Please try another song.");
                    alert.showAndWait();
                });
                System.err.println("MediaPlayerService: Playback error.");
            }

            @Override
            public void timeChanged(MediaPlayer mp, long newTime) {
                Platform.runLater(() -> {
                    currentTimeText.set(formatTime(newTime));
                    long totalTime = mp.getLength();
                    if (totalTime > 0) {
                        progress.set((double) newTime / totalTime);
                    }
                });
            }

            @Override
            public void lengthChanged(MediaPlayer mp, long newLength) {
                Platform.runLater(() -> {
                    totalTimeText.set(formatTime(newLength));
                });
            }
        });

        // 监听音量属性变化，更新播放器音量
        volume.addListener((obs, oldVal, newVal) -> {
            mediaPlayer.setVolume(newVal.intValue());
        });
        mediaPlayer.setVolume(volume.get()); // 初始设置播放器音量
    }

    /**
     * 设置播放列表。会清空当前列表并添加新歌曲。
     * @param songs 新的歌曲列表
     */
    public void setPlaylist(List<Song> songs) {
        Platform.runLater(() -> {
            currentPlaylist.clear();
            if (songs != null) {
                currentPlaylist.addAll(songs);
            }
            currentSongIndex.set(-1); // 重置当前播放歌曲索引
            System.out.println("MediaPlayerService: Playlist updated with " + currentPlaylist.size() + " songs.");
        });
    }

    /**
     * 播放指定歌曲的公共入口，处理文件存在性检查并在后台线程执行。
     * 外部（如 Controller）应调用此方法。
     * @param song 要播放的歌曲对象
     */
    public void playSong(Song song) {
        Task<Void> playTask = new Task<Void>() {
            @Override
            protected Void call() throws Exception {
                if (song == null || song.getLocalFilePath() == null) {
                    Platform.runLater(() -> {
                        Alert alert = new Alert(Alert.AlertType.ERROR);
                        alert.setTitle("Playback Error");
                        alert.setHeaderText("Invalid Song Information");
                        alert.setContentText("The selected song has no valid file path.");
                        alert.showAndWait();
                    });
                    return null;
                }
                File mediaFile = new File(song.getLocalFilePath());
                if (!mediaFile.exists()) {
                    Platform.runLater(() -> {
                        Alert alert = new Alert(Alert.AlertType.ERROR);
                        alert.setTitle("Playback Error");
                        alert.setHeaderText("File Not Found");
                        alert.setContentText("The song file could not be found at: \n" + song.getLocalFilePath() + "\n它可能已被移动或删除。");
                        alert.showAndWait();
                    });
                    return null;
                }

                // 文件存在，切换回 JavaFX Application Thread 执行实际播放
                Platform.runLater(() -> {
                    int index = currentPlaylist.indexOf(song);
                    if (index == -1) {
                        currentPlaylist.add(song);
                        currentSongIndex.set(currentPlaylist.size() - 1);
                    } else {
                        currentSongIndex.set(index);
                    }
                    playSongInternal(song);
                });
                return null;
            }
        };
        new Thread(playTask).start();
    }


    /**
     * 实际执行 VLCJ 播放操作的方法，应在 JavaFX Application Thread 上调用。
     * @param song 要播放的歌曲对象
     */
    private void playSongInternal(Song song) {
        mediaPlayer.stop();

        playMediaCallTime = System.nanoTime();

        boolean success = mediaPlayer.playMedia(song.getLocalFilePath());
        if (success) {
            currentSong.set(song);
            System.out.println("MediaPlayerService: Playing " + song.getTitle() + " from " + song.getLocalFilePath());
        } else {
            System.err.println("MediaPlayerService: Failed to start media " + song.getLocalFilePath());
            currentSong.set(null);
            currentSongIndex.set(-1);
            Alert alert = new Alert(Alert.AlertType.ERROR);
            alert.setTitle("Playback Error");
            alert.setHeaderText("Failed to start media playback.");
            alert.setContentText("Could not play: " + song.getTitle() + ".\n确保文件未损坏且 VLC 配置正确。");
            alert.showAndWait();
        }
    }

    public void togglePlayPause() {
        if (mediaPlayer.isPlaying()) {
            mediaPlayer.pause();
        } else {
            if (currentSong.get() == null && !currentPlaylist.isEmpty()) {
                playSong(currentPlaylist.get(0));
            } else if (currentSong.get() != null) {
                mediaPlayer.play();
            } else {
                System.out.println("MediaPlayerService: No song to play and playlist is empty.");
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

        int nextIndex = getNextSongIndex();
        if (nextIndex != -1) {
            currentSongIndex.set(nextIndex);
            Song nextSong = currentPlaylist.get(nextIndex);
            playSong(nextSong);
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

        int prevIndex = getPreviousSongIndex();
        if (prevIndex != -1) {
            currentSongIndex.set(prevIndex);
            Song prevSong = currentPlaylist.get(prevIndex);
            playSong(prevSong);
        } else {
            stop();
        }
    }

    /**
     * 根据当前播放模式获取下一首歌曲的索引。
     * @return 下一首歌曲的索引，如果当前模式下没有下一首则返回 -1。
     */
    private int getNextSongIndex() {
        if (currentPlaylist.isEmpty()) return -1;

        int currentIndex = currentSongIndex.get();
        if (currentIndex == -1) {
            return 0;
        }

        switch (playbackMode.get()) {
            case REPEAT_ONE:
                return currentIndex;
            case SHUFFLE:
                if (currentPlaylist.size() <= 1) return currentIndex;
                int newIndex;
                do {
                    newIndex = random.nextInt(currentPlaylist.size());
                } while (newIndex == currentIndex && currentPlaylist.size() > 1);
                return newIndex;
            case REPEAT_ALL:
            default:
                return (currentIndex + 1) % currentPlaylist.size();
        }
    }

    /**
     * 根据当前播放模式获取上一首歌曲的索引。
     * @return 上一首歌曲的索引，如果当前模式下没有上一首则返回 -1。
     */
    private int getPreviousSongIndex() {
        if (currentPlaylist.isEmpty()) return -1;

        int currentIndex = currentSongIndex.get();
        if (currentIndex == -1) {
            return 0;
        }

        switch (playbackMode.get()) {
            case REPEAT_ONE:
                return currentIndex;
            case SHUFFLE:
                if (currentPlaylist.size() <= 1) return currentIndex;
                int newIndex;
                do {
                    newIndex = random.nextInt(currentPlaylist.size());
                } while (newIndex == currentIndex && currentPlaylist.size() > 1);
                return newIndex;
            case REPEAT_ALL:
            default:
                return (currentIndex - 1 + currentPlaylist.size()) % currentPlaylist.size();
        }
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

    public ObjectProperty<PlaybackMode> playbackModeProperty() {
        return playbackMode;
    }

    /**
     * 设置播放模式。
     * @param mode 新的播放模式
     */
    public void setPlaybackMode(PlaybackMode mode) {
        this.playbackMode.set(mode);
        System.out.println("MediaPlayerService: Playback mode set to " + mode);
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
        VlcjManager.release();
    }
}
