// src/main/java/com/bilibili/musicplayer/controller/MainController.java
package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.service.MediaPlayerService; // 导入 MediaPlayerService
import com.bilibili.musicplayer.util.VlcjManager;         // 导入 VlcjManager

import javafx.application.Platform; // 导入 Platform
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;
import javafx.stage.Stage; // 导入 Stage

import java.net.URL;
import java.util.ResourceBundle;

public class MainController implements Initializable {

    @FXML private StackPane mainContentPane; // 用于显示不同视图的中心区域
    @FXML private DownloadController downloadViewController;
    @FXML private LibraryController libraryViewController;
    @FXML private LeftSidebarController leftSidebarController;
    @FXML private PlaybackController playbackBarController; // 底部播放控制条的控制器

    private MediaPlayerService mediaPlayerService; // MediaPlayerService 实例

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("MainController initialized.");

        VlcjManager.initialize(); // 确保 VLCJ 核心组件被初始化
        mediaPlayerService = new MediaPlayerService();


        leftSidebarController.setMainController(this);

        downloadViewController.setMainController(this);
        downloadViewController.setLibraryController(libraryViewController); // DownloadController 下载完成后通知 LibraryController

        libraryViewController.setMediaPlayerService(mediaPlayerService);
        playbackBarController.setMediaPlayerService(mediaPlayerService);


        // 3. 应用程序关闭时释放资源
        Platform.runLater(() -> {
            // 获取当前场景的窗口，并监听其关闭事件
            Stage stage = (Stage) mainContentPane.getScene().getWindow();
            if (stage != null) {
                stage.setOnHidden(event -> {
                    System.out.println("Application closing, releasing MediaPlayerService resources...");
                    mediaPlayerService.release();
                    VlcjManager.release(); // 确保 VlcjManager 持有的资源也被释放
                });
            }
        });

        System.out.println("MainController setup complete.");
    }

    /**
     * 切换主内容区域显示的视图。
     * @param viewName 要显示的视图名称 ("download" 或 "library")
     */
    public void showView(String viewName) {
        downloadViewController.getView().setVisible(false);
        libraryViewController.getView().setVisible(false);

        // 显示指定视图
        switch (viewName) {
            case "download":
                downloadViewController.getView().setVisible(true);
                break;
            case "library":
                libraryViewController.getView().setVisible(true);
                libraryViewController.refreshSongs(); // 刷新歌曲列表，确保最新数据
                break;
            // case "settings":
            //     // 如果有设置视图，将其设置为可见
            //     // settingsViewController.getView().setVisible(true);
            //     break;
            default:
                System.err.println("Unknown view name: " + viewName);
                break;
        }
    }

    // 提供给其他控制器访问的方法 (如果需要)
    public PlaybackController getPlaybackBarController() {
        return playbackBarController;
    }

    public LibraryController getLibraryViewController() {
        return libraryViewController;
    }

    public DownloadController getDownloadController() {
        return downloadViewController;
    }

    // 如果需要，可以提供 LeftSidebarController 的 getter
    public LeftSidebarController getLeftSidebarController() {
        return leftSidebarController;
    }
}
