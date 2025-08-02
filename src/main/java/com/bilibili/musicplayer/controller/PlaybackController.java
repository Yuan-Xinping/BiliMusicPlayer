// src/main/java/com/bilibili/musicplayer/controller/PlaybackController.java
package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.service.MediaPlayerService;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.Slider;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.MouseButton;
import javafx.scene.layout.HBox;

import java.net.URL;
import java.util.ResourceBundle;

public class PlaybackController implements Initializable {

    @FXML private HBox rootView;
    @FXML private Button playPauseButton;
    @FXML private Button stopButton;
    @FXML private Label currentSongLabel;
    @FXML private Label currentTimeLabel;
    @FXML private Slider progressBar;
    @FXML private Label totalTimeLabel;
    @FXML private Slider volumeSlider;

    private MediaPlayerService mediaPlayerService;

    private Image playIcon;
    private Image pauseIcon;

    private boolean isDragging = false; // 新增：用于区分拖动和点击

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("PlaybackController initialized.");

        mediaPlayerService = new MediaPlayerService();

        // 加载图标
        try {
            playIcon = new Image(getClass().getResourceAsStream("/icons/play_icon.png"));
            pauseIcon = new Image(getClass().getResourceAsStream("/icons/pause_icon.png"));
            // 初始设置播放图标
            ((ImageView) playPauseButton.getGraphic()).setImage(playIcon);
        } catch (Exception e) {
            System.err.println("Failed to load playback icons: " + e.getMessage());
            playPauseButton.setText("▶");
            stopButton.setText("■");
        }

        // 进度条只进行单向绑定，由 MediaPlayerService 更新 UI
        progressBar.valueProperty().bind(mediaPlayerService.progressProperty());
        // 绑定 currentTimeLabel 和 totalTimeLabel
        currentTimeLabel.textProperty().bind(mediaPlayerService.currentTimeTextProperty());
        totalTimeLabel.textProperty().bind(mediaPlayerService.totalTimeTextProperty());
        volumeSlider.valueProperty().bindBidirectional(mediaPlayerService.volumeProperty());

        // 监听播放状态，切换播放/暂停按钮图标
        mediaPlayerService.playingProperty().addListener((obs, oldVal, newVal) -> {
            if (newVal) {
                ((ImageView) playPauseButton.getGraphic()).setImage(pauseIcon);
            } else {
                ((ImageView) playPauseButton.getGraphic()).setImage(playIcon);
            }
        });

        progressBar.setOnMousePressed(event -> {
            isDragging = true; // 用户开始拖动
            // 用户开始拖动时，解除进度条的绑定，使其可以自由移动
            progressBar.valueProperty().unbind();
            // 同时，解除 currentTimeLabel 的绑定，以便手动更新其文本
            currentTimeLabel.textProperty().unbind();
        });

        progressBar.setOnMouseDragged(event -> {
            // 在拖动时，实时更新当前时间标签，提供视觉反馈
            long totalLengthMillis = mediaPlayerService.getVlcjMediaPlayerLength();
            if (totalLengthMillis > 0) {
                // 根据滑块的当前值（用户拖动到的位置）计算预览时间
                long previewTimeMillis = (long) (progressBar.getValue() * totalLengthMillis);
                currentTimeLabel.setText(formatTime(previewTimeMillis));
            }
        });

        progressBar.setOnMouseReleased(event -> {
            isDragging = false; // 用户释放鼠标，拖动结束
            // 用户释放鼠标时，进行实际的跳转
            mediaPlayerService.seek((float) progressBar.getValue());
            // 重新绑定进度条，使其恢复由播放器更新
            progressBar.valueProperty().bind(mediaPlayerService.progressProperty());
            // 重新绑定 currentTimeLabel
            currentTimeLabel.textProperty().bind(mediaPlayerService.currentTimeTextProperty());
        });
        progressBar.setOnMouseClicked(event -> {
            if (event.getButton() == MouseButton.PRIMARY && !isDragging) {
                double clickX = event.getX();
                double sliderWidth = progressBar.getWidth();
                double position = clickX / sliderWidth;

                // 将百分比限制在0.0到1.0之间
                position = Math.max(0.0, Math.min(1.0, position));

                // 执行跳转
                mediaPlayerService.seek((float) position);

            }
        });
        // 初始设置音量
        mediaPlayerService.volumeProperty().set((int)volumeSlider.getValue());
    }

    @FXML
    private void handlePlayPause() {
        mediaPlayerService.togglePlayPause();
    }

    @FXML
    private void handleStop() {
        mediaPlayerService.stop();
    }

    /**
     * 供外部控制器调用，播放指定歌曲。
     * @param song 要播放的歌曲对象
     */
    public void playSong(Song song) {
        mediaPlayerService.playSong(song);
        // 手动设置 currentSongLabel 的文本，以确保显示格式
        if (song != null) {
            currentSongLabel.setText(song.getTitle() + (song.getArtist() != null && !song.getArtist().isEmpty() ? " - " + song.getArtist() : ""));
        } else {
            currentSongLabel.setText("未选择歌曲");
        }
    }

    public HBox getView() {
        return rootView;
    }

    /**
     * 格式化毫秒为 MM:SS 字符串。
     * (在 PlaybackController 中也提供一个，方便拖动时实时显示)
     */
    private String formatTime(long millis) {
        long totalSeconds = millis / 1000;
        long minutes = totalSeconds / 60;
        long seconds = totalSeconds % 60;
        return String.format("%02d:%02d", minutes, seconds);
    }
}
