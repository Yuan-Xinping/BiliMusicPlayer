// src/main/java/com/bilibili/musicplayer/controller/LeftSidebarController.java
package com.bilibili.musicplayer.controller;

import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.layout.VBox;

import java.net.URL;
import java.util.ResourceBundle;

public class LeftSidebarController implements Initializable {

    private MainController mainController;

    @FXML private VBox rootView; // FXML的根节点，用于getView()

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("LeftSidebarController initialized.");
    }

    public void setMainController(MainController mainController) {
        this.mainController = mainController;
    }

    @FXML
    private void handleLibraryClick() {
        if (mainController != null) {
            mainController.showView("library");
        }
    }

    @FXML
    private void handleDownloadClick() {
        if (mainController != null) {
            mainController.showView("download");
        }
    }

    @FXML
    private void handleSettingsClick() {
        if (mainController != null) {
            System.out.println("Settings clicked (not yet implemented).");
        }
    }

    public VBox getView() {
        return rootView;
    }
}
