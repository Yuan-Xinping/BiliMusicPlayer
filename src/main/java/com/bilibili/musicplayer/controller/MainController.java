package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.service.MediaPlayerService;
import com.bilibili.musicplayer.util.VlcjManager;

import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Alert;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

import java.net.URL;
import java.util.ResourceBundle;

public class MainController implements Initializable {

    @FXML private StackPane mainContentPane;
    @FXML private DownloadController downloadViewController;
    @FXML private LibraryController libraryViewController;
    @FXML private LeftSidebarController leftSidebarController;
    @FXML private PlaybackController playbackBarController;
    @FXML private SettingsController settingsViewController;

    private MediaPlayerService mediaPlayerService;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("MainController initialized.");

        boolean vlcInitSuccess = VlcjManager.isInitialized(); // 检查是否已初始化
        if (!vlcInitSuccess) {

            vlcInitSuccess = VlcjManager.initialize();
            if (!vlcInitSuccess) {
                Platform.runLater(() -> {
                    Alert alert = new Alert(Alert.AlertType.ERROR);
                    alert.setTitle("错误");
                    alert.setHeaderText("VLC 媒体播放器初始化失败");
                    // 修改提示信息，因为已移除 VLC 路径设置
                    alert.setContentText("请确保已安装 VLC 播放器。\n应用将退出。");
                    alert.showAndWait();
                    Platform.exit();
                    System.exit(1);
                });
                return; // 阻止后续初始化
            }
        }

        mediaPlayerService = new MediaPlayerService();

        leftSidebarController.setMainController(this);

        downloadViewController.setMainController(this);
        downloadViewController.setLibraryController(libraryViewController);

        libraryViewController.setMediaPlayerService(mediaPlayerService);
        playbackBarController.setMediaPlayerService(mediaPlayerService);

        // 设置 SettingsController 的 MainController
        settingsViewController.setMainController(this);

        // 应用程序关闭时释放资源
        Platform.runLater(() -> {
            Stage stage = (Stage) mainContentPane.getScene().getWindow();
            if (stage != null) {
                stage.setOnHidden(event -> {
                    System.out.println("Application closing, releasing MediaPlayerService and VlcjManager resources...");
                    mediaPlayerService.release(); // MediaPlayerService 会调用 VlcjManager.release()
                    VlcjManager.release(); // 显式调用 VlcjManager.release() 确保资源释放
                });
            }
        });

        System.out.println("MainController setup complete.");
    }

    /**
     * 切换主内容区域显示的视图。
     * @param viewName 要显示的视图名称 ("download", "library", "settings")
     */
    public void showView(String viewName) {
        downloadViewController.getView().setVisible(false);
        libraryViewController.getView().setVisible(false);
        settingsViewController.getView().setVisible(false); // 隐藏设置视图

        // 显示指定视图
        switch (viewName) {
            case "download":
                downloadViewController.getView().setVisible(true);
                break;
            case "library":
                libraryViewController.getView().setVisible(true);
                libraryViewController.refreshSongs();
                break;
            case "settings":
                settingsViewController.getView().setVisible(true);
                break;
            default:
                System.err.println("Unknown view name: " + viewName);
                break;
        }
    }

    public PlaybackController getPlaybackBarController() {
        return playbackBarController;
    }

    public LibraryController getLibraryViewController() {
        return libraryViewController;
    }

    public DownloadController getDownloadController() {
        return downloadViewController;
    }

    public LeftSidebarController getLeftSidebarController() {
        return leftSidebarController;
    }

    // 如果需要，可以提供 SettingsController 的 getter
    public SettingsController getSettingsViewController() {
        return settingsViewController;
    }
}
