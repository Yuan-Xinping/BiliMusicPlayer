package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.service.MediaPlayerService;

import javafx.application.Platform;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.Slider;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.MouseButton;
import javafx.scene.layout.HBox;

import java.io.InputStream;
import java.net.URL;
import java.util.ResourceBundle;
// import java.text.DecimalFormat; // 此导入已不再需要，因为timeFormat字段被移除了

public class PlaybackController implements Initializable {

    @FXML private HBox rootView;
    @FXML private Button playPauseButton;
    @FXML private Button stopButton;
    @FXML private Label currentSongLabel;
    @FXML private Label currentTimeLabel;
    @FXML private Slider progressBar;
    @FXML private Label totalTimeLabel;
    @FXML private Slider volumeSlider;

    @FXML private Button prevButton;
    @FXML private Button nextButton;
    @FXML private Button playbackModeButton;

    private MediaPlayerService mediaPlayerService;

    // 图标资源，统一命名为不带 _icon 后缀
    private Image playIcon;
    private Image pauseIcon;
    private Image normalModeIcon; // 用于顺序播放模式
    private Image repeatAllModeIcon;
    private Image repeatOneModeIcon;
    private Image shuffleModeIcon;

    private boolean isDragging = false;
    // private final DecimalFormat timeFormat = new DecimalFormat("00"); // 此字段未在代码中使用，可以移除

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("PlaybackController initialized.");

        // 加载图标
        try {
            playIcon = loadImage("/icons/play.png");
            pauseIcon = loadImage("/icons/pause.png");
            normalModeIcon = loadImage("/icons/normal.png");
            repeatAllModeIcon = loadImage("/icons/repeat_all.png");
            repeatOneModeIcon = loadImage("/icons/repeat_one.png");
            shuffleModeIcon = loadImage("/icons/shuffle.png");

            // 初始设置播放图标
            if (playPauseButton.getGraphic() instanceof ImageView) {
                ((ImageView) playPauseButton.getGraphic()).setImage(playIcon);
            } else {
                playPauseButton.setText("▶"); // 备用文本
            }

            // 初始设置播放模式图标 (假设默认是NORMAL)
            if (playbackModeButton != null && playbackModeButton.getGraphic() instanceof ImageView) {
                ((ImageView) playbackModeButton.getGraphic()).setImage(normalModeIcon);
            } else if (playbackModeButton != null) {
                playbackModeButton.setText("顺序"); // 备用文本
            }

            // 确保其他按钮也有文本或图标作为备用
            if (stopButton.getGraphic() == null) stopButton.setText("■");
            if (prevButton != null && prevButton.getGraphic() == null) prevButton.setText("⏮");
            if (nextButton != null && nextButton.getGraphic() == null) nextButton.setText("⏭");

        } catch (Exception e) {
            System.err.println("Failed to load playback icons: " + e.getMessage() + ". Using text fallback.");
            e.printStackTrace();
            playPauseButton.setText("▶");
            stopButton.setText("■");
            if (prevButton != null) prevButton.setText("⏮");
            if (nextButton != null) nextButton.setText("⏭");
            if (playbackModeButton != null) playbackModeButton.setText("模式");
        }

        // --- 进度条拖动事件处理 (修改部分) ---
        progressBar.setOnMousePressed(event -> {
            isDragging = true; // 用户开始交互 (点击或拖动)
            // 用户开始拖动时，解除进度条的绑定，使其可以自由移动
            progressBar.valueProperty().unbind();
            // 同时，解除 currentTimeLabel 的绑定，以便手动更新其文本
            currentTimeLabel.textProperty().unbind();

            // 立即根据点击位置更新滑块的值和时间标签，实现“点击跳转”的即时反馈
            double clickX = event.getX();
            double sliderWidth = progressBar.getWidth();
            double position = clickX / sliderWidth;

            // 将百分比限制在0.0到1.0之间 (假设 Slider 的 min=0.0, max=1.0)
            position = Math.max(0.0, Math.min(1.0, position));
            progressBar.setValue(position); // 手动设置滑块的值

            // 更新当前时间标签以显示预览时间
            if (mediaPlayerService != null) {
                long totalLengthMillis = mediaPlayerService.getVlcjMediaPlayerLength();
                if (totalLengthMillis > 0) {
                    long previewTimeMillis = (long) (progressBar.getValue() * totalLengthMillis);
                    currentTimeLabel.setText(formatTime(previewTimeMillis));
                }
            }
        });

        progressBar.setOnMouseDragged(event -> {
            // 在拖动时，实时更新滑块的值和当前时间标签，提供视觉反馈
            if (isDragging) { // 确保是在拖动状态下
                double dragX = event.getX();
                double sliderWidth = progressBar.getWidth();
                double position = dragX / sliderWidth;

                position = Math.max(0.0, Math.min(1.0, position));
                progressBar.setValue(position); // **关键：手动设置滑块的值**

                if (mediaPlayerService != null) {
                    long totalLengthMillis = mediaPlayerService.getVlcjMediaPlayerLength();
                    if (totalLengthMillis > 0) {
                        // 根据滑块的当前值（用户拖动到的位置）计算预览时间
                        long previewTimeMillis = (long) (progressBar.getValue() * totalLengthMillis);
                        currentTimeLabel.setText(formatTime(previewTimeMillis));
                    }
                }
            }
        });

        progressBar.setOnMouseReleased(event -> {
            isDragging = false; // 用户释放鼠标，交互结束
            if (mediaPlayerService != null) {
                final float targetPosition = (float) progressBar.getValue(); // 保存目标位置
                mediaPlayerService.seek(targetPosition);

                // 创建一个临时的ChangeListener来监听progressProperty
                // 当progressProperty更新到接近目标位置时，再重新绑定
                ChangeListener<Number> progressListener = new ChangeListener<Number>() {
                    @Override
                    public void changed(ObservableValue<? extends Number> observable, Number oldValue, Number newValue) {
                        // 使用一个小的容差值来判断是否到达目标位置，因为浮点数比较不精确
                        if (Math.abs(newValue.floatValue() - targetPosition) < 0.005f) { // 0.5% 的容差
                            Platform.runLater(() -> {
                                // 重新绑定进度条和时间标签
                                progressBar.valueProperty().bind(mediaPlayerService.progressProperty());
                                currentTimeLabel.textProperty().bind(mediaPlayerService.currentTimeTextProperty());
                                // 移除这个临时的监听器，避免重复触发和内存泄漏
                                mediaPlayerService.progressProperty().removeListener(this);
                                System.out.println("Progress bar re-bound after seek completion.");
                            });
                        }
                    }
                };
                // 添加临时的监听器
                mediaPlayerService.progressProperty().addListener(progressListener);
            }
        });

        // 初始设置音量 (在 setMediaPlayerService 后会重新绑定)
        volumeSlider.setValue(100); // 默认值
    }

    /**
     * 新增的辅助方法，用于加载图片并提供调试信息。
     * @param path 资源路径，例如 "/icons/play.png"
     * @return 加载成功的 Image 对象，如果失败则抛出异常
     * @throws IllegalArgumentException 如果输入流为 null (资源未找到)
     * @throws RuntimeException 如果 Image 构造失败
     */
    private Image loadImage(String path) {
        System.out.println("DEBUG: Attempting to load image from path: " + path);
        InputStream is = null;
        try {
            is = getClass().getResourceAsStream(path);

            if (is == null) {
                System.err.println("ERROR: Input stream is NULL for path: " + path + ". Resource not found on classpath.");
                throw new IllegalArgumentException("Input stream must not be null for path: " + path);
            }

            Image image = new Image(is);
            System.out.println("DEBUG: Successfully loaded image from path: " + path);
            return image;
        } catch (IllegalArgumentException e) {
            System.err.println("ERROR: Failed to load image from path (IllegalArgumentException): " + path + ". Message: " + e.getMessage());
            throw e;
        } catch (Exception e) {
            System.err.println("ERROR: Failed to load image from path (General Exception): " + path + ". Message: " + e.getMessage());
            throw new RuntimeException("Error loading image from " + path, e);
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (java.io.IOException e) {
                    System.err.println("WARNING: Failed to close input stream for path: " + path + ". Error: " + e.getMessage());
                    e.printStackTrace();
                }
            }
        }
    }


    /**
     * 新增方法：设置 MediaPlayerService 实例。
     * 必须在 MainController 中调用此方法来注入 MediaPlayerService。
     * @param service MediaPlayerService 实例
     */
    public void setMediaPlayerService(MediaPlayerService service) {
        this.mediaPlayerService = service;

        // 只有当 mediaPlayerService 被设置后，才进行属性绑定
        if (this.mediaPlayerService == null) {
            System.err.println("Error: MediaPlayerService not set in PlaybackController.");
            return;
        }

        // 初始绑定，但在拖动时会解除
        progressBar.valueProperty().bind(mediaPlayerService.progressProperty());
        currentTimeLabel.textProperty().bind(mediaPlayerService.currentTimeTextProperty());
        totalTimeLabel.textProperty().bind(mediaPlayerService.totalTimeTextProperty());
        volumeSlider.valueProperty().bindBidirectional(mediaPlayerService.volumeProperty());

        // 监听播放状态，切换播放/暂停按钮图标
        mediaPlayerService.playingProperty().addListener((obs, oldVal, newVal) -> {
            Platform.runLater(() -> { // 确保在 JavaFX 线程更新 UI
                if (newVal) {
                    if (playPauseButton.getGraphic() instanceof ImageView) {
                        ((ImageView) playPauseButton.getGraphic()).setImage(pauseIcon);
                    } else {
                        playPauseButton.setText("❚❚"); // 暂停文本
                    }
                } else {
                    if (playPauseButton.getGraphic() instanceof ImageView) {
                        ((ImageView) playPauseButton.getGraphic()).setImage(playIcon);
                    } else {
                        playPauseButton.setText("▶"); // 播放文本
                    }
                }
            });
        });

        // 监听当前播放歌曲，更新歌曲信息标签
        mediaPlayerService.currentSongProperty().addListener((obs, oldSong, newSong) -> {
            Platform.runLater(() -> { // 确保在 JavaFX 线程更新 UI
                if (newSong != null) {
                    currentSongLabel.setText(newSong.getTitle() + (newSong.getArtist() != null && !newSong.getArtist().isEmpty() ? " - " + newSong.getArtist() : ""));
                } else {
                    currentSongLabel.setText("未选择歌曲");
                }
            });
        });

        // 监听播放模式属性变化，更新按钮图标或文本
        mediaPlayerService.playbackModeProperty().addListener((obs, oldMode, newMode) -> {
            Platform.runLater(() -> {
                updatePlaybackModeButtonUI(newMode);
            });
        });
        // 初始设置播放模式按钮UI
        updatePlaybackModeButtonUI(mediaPlayerService.playbackModeProperty().get());

        // 初始设置音量
        mediaPlayerService.volumeProperty().set((int)volumeSlider.getValue());
    }

    // --- FXML 绑定的事件处理方法 ---
    @FXML
    private void handlePlayPause() {
        if (mediaPlayerService != null) {
            mediaPlayerService.togglePlayPause();
            System.out.println("PlaybackController: Play/Pause toggled.");
        }
    }

    @FXML
    private void handleStop() {
        if (mediaPlayerService != null) {
            mediaPlayerService.stop();
            System.out.println("PlaybackController: Stop button clicked.");
        }
    }

    @FXML
    private void handlePrevious() {
        if (mediaPlayerService != null) {
            mediaPlayerService.playPrevious();
            System.out.println("PlaybackController: Play previous song.");
        }
    }

    @FXML
    private void handleNext() {
        if (mediaPlayerService != null) {
            mediaPlayerService.playNext();
            System.out.println("PlaybackController: Play next song.");
        }
        // 修正：这里应该是nextButton
    }

    @FXML
    private void handleTogglePlaybackMode() {
        if (mediaPlayerService != null) {
            MediaPlayerService.PlaybackMode currentMode = mediaPlayerService.playbackModeProperty().get();
            MediaPlayerService.PlaybackMode nextMode;
            switch (currentMode) {
                case NORMAL:
                    nextMode = MediaPlayerService.PlaybackMode.REPEAT_ALL;
                    break;
                case REPEAT_ALL:
                    nextMode = MediaPlayerService.PlaybackMode.REPEAT_ONE;
                    break;
                case REPEAT_ONE:
                    nextMode = MediaPlayerService.PlaybackMode.SHUFFLE;
                    break;
                case SHUFFLE:
                default:
                    nextMode = MediaPlayerService.PlaybackMode.NORMAL;
                    break;
            }
            mediaPlayerService.setPlaybackMode(nextMode);
            System.out.println("PlaybackController: Playback mode toggled to " + nextMode);
        }
    }

    /**
     * 供外部控制器调用，播放指定歌曲。
     * 此方法现在是冗余的，外部应直接调用 MediaPlayerService.playSong()。
     * @param song 要播放的歌曲对象
     */
    public void playSong(Song song) {
        if (mediaPlayerService != null) {
            mediaPlayerService.playSong(song);
            System.out.println("PlaybackController.playSong() called. Song: " + (song != null ? song.getTitle() : "null"));
        } else {
            System.err.println("PlaybackController: MediaPlayerService is null, cannot play song.");
        }
    }

    public HBox getView() {
        return rootView;
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

    /**
     * 根据播放模式更新播放模式按钮的UI。
     * @param mode 当前播放模式
     */
    private void updatePlaybackModeButtonUI(MediaPlayerService.PlaybackMode mode) {
        if (playbackModeButton == null) return;

        ImageView iconView = null;
        String text = ""; // 备用文本

        switch (mode) {
            case NORMAL:
                iconView = (normalModeIcon != null) ? new ImageView(normalModeIcon) : null;
                text = "顺序";
                break;
            case REPEAT_ALL:
                iconView = (repeatAllModeIcon != null) ? new ImageView(repeatAllModeIcon) : null;
                text = "循环";
                break;
            case REPEAT_ONE:
                iconView = (repeatOneModeIcon != null) ? new ImageView(repeatOneModeIcon) : null; // 修正：这里应该是repeatOneModeIcon
                text = "单曲";
                break;
            case SHUFFLE:
                iconView = (shuffleModeIcon != null) ? new ImageView(shuffleModeIcon) : null;
                text = "随机";
                break;
        }

        if (iconView != null) {
            iconView.setFitWidth(24); // 设置图标大小
            iconView.setFitHeight(24);
            playbackModeButton.setGraphic(iconView);
            playbackModeButton.setText(""); // 如果有图标，清空文本
        } else {
            playbackModeButton.setGraphic(null); // 清空图标
            playbackModeButton.setText(text); // 显示文本
        }
        System.out.println("Playback Mode Button UI updated to: " + mode);
    }
}
