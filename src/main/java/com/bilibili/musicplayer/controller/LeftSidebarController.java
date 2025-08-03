package com.bilibili.musicplayer.controller;

import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.layout.VBox;

import java.net.URL;
import java.util.ResourceBundle;

public class LeftSidebarController implements Initializable {

    private MainController mainController;

    @FXML private VBox rootView;

    @FXML private Button libraryButton;
    @FXML private Button downloadButton;
    @FXML private Button settingsButton;

    private Button currentActiveButton;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("LeftSidebarController initialized.");
    }

    /**
     * 设置 MainController 实例，并初始化侧边栏的选中状态。
     * @param mainController MainController 实例
     */
    public void setMainController(MainController mainController) {
        this.mainController = mainController;

        setActiveButton(libraryButton);
        if (mainController != null) {
            mainController.showView("library");
            // 已删除：rootView.setVisible(!AppConfig.isHideSidebar());
            // 侧边栏的可见性现在将由 FXML 默认或 CSS 控制，不再通过 AppConfig 设置。
        }
    }

    /**
     * 设置当前激活的侧边栏按钮的样式。
     * 移除之前激活按钮的 'sidebar-button-active' 类，并为新激活的按钮添加该类。
     * @param button 要激活的按钮
     */
    public void setActiveButton(Button button) {
        if (currentActiveButton != null) {
            currentActiveButton.getStyleClass().remove("sidebar-button-active");
        }
        if (button != null) {
            button.getStyleClass().add("sidebar-button-active");
        }
        currentActiveButton = button;
    }

    // 提供 getter 方法，如果 MainController 或其他地方需要直接访问这些按钮实例
    public Button getLibraryButton() { return libraryButton; }
    public Button getDownloadButton() { return downloadButton; }
    public Button getSettingsButton() { return settingsButton; }


    @FXML
    private void handleLibraryClick() {
        setActiveButton(libraryButton);
        if (mainController != null) {
            mainController.showView("library");
        }
    }

    @FXML
    private void handleDownloadClick() {
        setActiveButton(downloadButton);
        if (mainController != null) {
            mainController.showView("download");
        }
    }

    @FXML
    private void handleSettingsClick() {
        setActiveButton(settingsButton);
        if (mainController != null) {
            System.out.println("Settings clicked.");
            mainController.showView("settings"); // 现在可以调用了
        }
    }

    public VBox getView() {
        return rootView;
    }
}
