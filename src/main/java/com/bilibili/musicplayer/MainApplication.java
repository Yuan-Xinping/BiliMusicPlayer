// src/main/java/com/bilibili/musicplayer/MainApplication.java
package com.bilibili.musicplayer;

import com.bilibili.musicplayer.util.VlcjManager;
import com.bilibili.musicplayer.util.AppConfig; // 导入 AppConfig

import javafx.application.Application;
import javafx.application.Platform;
import javafx.concurrent.Task;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Label;
import javafx.scene.layout.StackPane;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

import java.io.IOException;


public class MainApplication extends Application {

    private Stage primaryStage;
    private Stage splashStage;

    @Override
    public void start(Stage stage) throws IOException {
        this.primaryStage = stage;

        // 1. 显示一个简单的加载界面（Splash Screen）
        showSplashScreen();

        // 2. 在后台线程执行耗时的初始化任务
        Task<Boolean> initializationTask = new Task<Boolean>() {
            @Override
            protected Boolean call() throws Exception {
                System.out.println("Starting vlcj initialization on background thread...");
                // 在后台线程调用 VlcjManager 的初始化方法
                return VlcjManager.initialize();
            }
        };

        // 任务成功完成时
        initializationTask.setOnSucceeded(event -> {
            boolean success = initializationTask.getValue();
            if (success) {
                System.out.println("vlcj initialization completed successfully.");
                // 3. 初始化成功后，在 JavaFX UI 线程上切换到主界面
                Platform.runLater(() -> {
                    try {
                        showMainApplication();
                    } catch (IOException e) {
                        System.err.println("Failed to load main application UI: " + e.getMessage());
                        e.printStackTrace();
                        Platform.exit(); // 如果主界面加载失败，则退出应用
                    }
                });
            } else {
                System.err.println("vlcj initialization failed. Exiting application.");
                // 3. 初始化失败，在 JavaFX UI 线程上显示错误信息并退出
                Platform.runLater(() -> {
                    Alert alert = new Alert(Alert.AlertType.ERROR);
                    alert.setTitle("初始化错误");
                    alert.setHeaderText("VLC 媒体播放器初始化失败");
                    alert.setContentText("请确保 VLC Media Player 已安装，并且其库文件可访问。\n" +
                            "请查看控制台获取更多详细信息，或尝试设置 -Djna.library.path JVM 参数。");
                    alert.showAndWait();
                    Platform.exit();
                });
            }
        });

        // 任务失败（例如，抛出未捕获的异常）时
        initializationTask.setOnFailed(event -> {
            System.err.println("vlcj initialization task failed unexpectedly: " + initializationTask.getException().getMessage());
            initializationTask.getException().printStackTrace();
            Platform.runLater(() -> {
                Alert alert = new Alert(Alert.AlertType.ERROR);
                alert.setTitle("启动错误");
                alert.setHeaderText("应用程序启动时发生意外错误");
                alert.setContentText("错误详情: " + initializationTask.getException().getMessage());
                alert.showAndWait();
                Platform.exit();
            });
        });

        // 启动后台任务
        new Thread(initializationTask).start();
    }

    /**
     * 显示一个简单的加载界面。
     */
    private void showSplashScreen() {
        splashStage = new Stage();
        splashStage.initStyle(StageStyle.UNDECORATED); // 无边框，更像一个加载窗口
        splashStage.setTitle("加载中...");

        StackPane pane = new StackPane(new Label("正在加载 Bili Music Player..."));
        pane.setPrefSize(400, 200);
        // 简单的样式，可以根据你的应用主题进行调整
        pane.setStyle("-fx-background-color: #2c2f33; -fx-text-fill: white; -fx-font-size: 18px; -fx-alignment: center;");
        Label loadingLabel = (Label) pane.getChildren().get(0);
        loadingLabel.setStyle("-fx-text-fill: white;");

        Scene splashScene = new Scene(pane);
        splashStage.setScene(splashScene);
        splashStage.show();
    }

    /**
     * 加载并显示主应用程序界面。
     * @throws IOException 如果 FXML 文件加载失败
     */
    private void showMainApplication() throws IOException {
        // 关闭加载界面
        if (splashStage != null) {
            splashStage.hide();
        }

        // FXML 文件路径调整为 /com/bilibili/musicplayer/mainwindow.fxml
        FXMLLoader loader = new FXMLLoader(getClass().getResource("/com/bilibili/musicplayer/mainwindow.fxml"));
        Parent root = loader.load();

        Scene scene = new Scene(root, 1200, 800); // 使用 1200x800 尺寸
        // CSS 文件路径调整为 /css/style.css
        scene.getStylesheets().add(getClass().getResource("/css/style.css").toExternalForm());

        primaryStage.setTitle("Bili Music Player"); // 设置窗口标题
        primaryStage.setScene(scene); // 设置场景
        primaryStage.show(); // 显示窗口
    }

    @Override
    public void stop() throws Exception {
        super.stop();
        // 确保在应用程序关闭时释放 VLCJ 资源
        VlcjManager.release();
        System.out.println("Application stopped. VLCJ resources released.");
    }

    public static void main(String[] args) {
        launch(args);
    }
}
