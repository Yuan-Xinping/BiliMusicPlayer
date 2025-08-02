package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.service.BiliDownloader;
import com.bilibili.musicplayer.service.SongDAO;

import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ProgressBar;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;

import java.net.URL;
import java.util.ResourceBundle;

public class DownloadController implements Initializable {

    @FXML private VBox rootView;
    @FXML private TextField urlField;
    @FXML private Button downloadByUrlButton; // 对应FXML中的按链接下载按钮
    @FXML private Button downloadByBvButton;  // 对应FXML中的按BV号下载按钮
    @FXML private ProgressBar progressBar;
    @FXML private Label statusLabel;

    private MainController mainController;
    private SongDAO songDAO;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("DownloadController initialized.");
        progressBar.setProgress(0); // 初始进度为0
        statusLabel.setText("等待输入链接...");
        songDAO = new SongDAO();
    }

    public void setMainController(MainController mainController) {
        this.mainController = mainController;
    }

    /**
     * 处理“按链接下载”按钮点击事件
     */
    @FXML
    private void handleDownloadByUrlClick() {
        String input = urlField.getText().trim();
        if (input.isEmpty()) {
            statusLabel.setText("请输入B站视频链接！");
            return;
        }

        if (!input.startsWith("http://") && !input.startsWith("https://")) {
            if (input.startsWith("www.bilibili.com/") || input.startsWith("m.bilibili.com/") || input.startsWith("b23.tv/")) {
                input = "https://" + input; // 自动添加https://
                System.out.println("Automatically corrected URL (added https://): " + input); // 调试信息
            } else {
                statusLabel.setText("请输入有效的B站链接 (以http://, https://, www.bilibili.com/, m.bilibili.com/ 或 b23.tv/ 开头)！");
                return;
            }
        }

        startDownload(input);
    }

    /**
     * 处理“按BV号下载”按钮点击事件
     */
    @FXML
    private void handleDownloadByBvClick() {
        String input = urlField.getText().trim();
        if (input.isEmpty()) {
            statusLabel.setText("请输入BV号！");
            return;
        }
        // 简单的BV号格式检查 (BV开头，后面是数字和字母)
        if (!input.startsWith("BV") || input.length() != 12) {
            statusLabel.setText("请输入有效的BV号 (例如: BV1qD4y1U7fs)！");
            return;
        }
        startDownload(input);
    }

    /**
     * 启动下载任务的通用方法
     * @param identifier 视频链接或BV号
     */
    private void startDownload(String identifier) {
        BiliDownloader.DownloadTask downloadTask = new BiliDownloader.DownloadTask(identifier);

        progressBar.progressProperty().bind(downloadTask.progressProperty());
        statusLabel.textProperty().bind(downloadTask.messageProperty());

        downloadByUrlButton.disableProperty().bind(downloadTask.runningProperty());
        downloadByBvButton.disableProperty().bind(downloadTask.runningProperty());
        urlField.disableProperty().bind(downloadTask.runningProperty());

        downloadTask.setOnSucceeded(event -> {
            statusLabel.textProperty().unbind();
            progressBar.progressProperty().unbind();

            Song downloadedSong = downloadTask.getValue();
            if (downloadedSong != null) {
                if (songDAO.saveSong(downloadedSong)) {
                    statusLabel.setText("下载成功并已保存: " + downloadedSong.getTitle());
                    System.out.println("下载完成的歌曲信息: " + downloadedSong);
                    if (mainController != null && mainController.getLibraryViewController() != null) {
                        Platform.runLater(() -> {
                            mainController.getLibraryViewController().addSong(downloadedSong);
                            mainController.getLibraryViewController().refreshSongs();
                        });
                    }
                } else {
                    statusLabel.setText("下载成功，但保存到数据库失败: " + downloadedSong.getTitle());
                }
            } else {
                statusLabel.setText("下载任务成功完成，但未返回歌曲信息。");
            }
            progressBar.setProgress(1);
        });

        downloadTask.setOnFailed(event -> {
            statusLabel.textProperty().unbind();
            progressBar.progressProperty().unbind();

            Throwable exception = downloadTask.getException();
            String errorMessage = "下载失败: " + (exception != null ? exception.getMessage() : "未知错误");
            statusLabel.setText(errorMessage);
            System.err.println(errorMessage);
            if (exception != null) {
                exception.printStackTrace();
            }
            progressBar.setProgress(0);
        });

        downloadTask.setOnCancelled(event -> {
            statusLabel.textProperty().unbind();
            progressBar.progressProperty().unbind();

            statusLabel.setText("下载已取消。");
            progressBar.setProgress(0);
        });

        new Thread(downloadTask).start();
    }

    public VBox getView() {
        return rootView;
    }
}
