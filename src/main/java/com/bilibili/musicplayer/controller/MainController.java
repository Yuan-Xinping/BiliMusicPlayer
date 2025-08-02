// src/main/java/com/bilibili/musicplayer/controller/MainController.java
package com.bilibili.musicplayer.controller;

import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.layout.StackPane;
import javafx.scene.layout.VBox;

import java.net.URL;
import java.util.ResourceBundle;

public class MainController implements Initializable {

    @FXML private StackPane mainContentPane;
    @FXML private VBox leftSidebar;

    @FXML private DownloadController downloadViewController;
    @FXML private LibraryController libraryViewController;
    @FXML private LeftSidebarController leftSidebarController;
    @FXML private PlaybackController playbackBarController;


    @Override
    public void initialize(URL location, ResourceBundle resources) {
        leftSidebarController.setMainController(this);
        downloadViewController.setMainController(this);
        libraryViewController.setMainController(this);

        downloadViewController.getView().setVisible(true);
        libraryViewController.getView().setVisible(false);

        System.out.println("MainController initialized.");
    }

    public void showView(String viewName) {
        downloadViewController.getView().setVisible(false);
        libraryViewController.getView().setVisible(false);

        switch (viewName) {
            case "download":
                downloadViewController.getView().setVisible(true);
                break;
            case "library":
                libraryViewController.getView().setVisible(true);
                libraryViewController.refreshSongs(); // 刷新歌曲列表
                break;
        }
    }

    public PlaybackController getPlaybackBarController() {
        return playbackBarController;
    }

    public LibraryController getLibraryViewController() {
        return libraryViewController;
    }
}
