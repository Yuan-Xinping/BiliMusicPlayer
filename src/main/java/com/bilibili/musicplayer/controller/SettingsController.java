package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.util.AppConfig;
import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.*;
import javafx.scene.layout.AnchorPane;
import javafx.stage.DirectoryChooser;
import javafx.stage.FileChooser;
import javafx.stage.Window;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.ResourceBundle;

public class SettingsController implements Initializable {

    private MainController mainController;

    @FXML private AnchorPane rootView;

    // 下载设置
    @FXML private TextField downloadPathField;
    @FXML private TextField ytDlpPathField;
    @FXML private TextField ffmpegPathField;

    // 关于部分
    @FXML private Label appVersionLabel;
    @FXML private Label ytDlpVersionLabel;
    @FXML private Label ffmpegVersionLabel;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("SettingsController initialized.");
        setupListeners();
        loadSettings();
        updateAboutInfo();
    }

    public void setMainController(MainController mainController) {
        this.mainController = mainController;
    }

    public AnchorPane getView() {
        return rootView;
    }

    /**
     * 加载并显示所有设置项的当前值。
     */
    private void loadSettings() {
        downloadPathField.setText(AppConfig.getDownloadPath());
        ytDlpPathField.setText(AppConfig.getYtDlpPath());
        ffmpegPathField.setText(AppConfig.getFfmpegPath());
    }

    /**
     * 设置所有UI控件的监听器，以便在值改变时自动保存设置。
     */
    private void setupListeners() {
        downloadPathField.textProperty().addListener((obs, oldVal, newVal) -> AppConfig.setDownloadPath(newVal));
        ytDlpPathField.textProperty().addListener((obs, oldVal, newVal) -> AppConfig.setYtDlpPath(newVal));
        ffmpegPathField.textProperty().addListener((obs, oldVal, newVal) -> AppConfig.setFfmpegPath(newVal));
    }

    /**
     * 处理选择下载目录按钮点击事件。
     */
    @FXML
    private void handleChooseDownloadPath() {
        DirectoryChooser directoryChooser = new DirectoryChooser();
        directoryChooser.setTitle("选择下载保存目录");
        File defaultDir = new File(downloadPathField.getText());
        if (defaultDir.exists() && defaultDir.isDirectory()) {
            directoryChooser.setInitialDirectory(defaultDir);
        } else {
            directoryChooser.setInitialDirectory(new File(System.getProperty("user.home")));
        }

        Window stage = rootView.getScene().getWindow();
        File selectedDirectory = directoryChooser.showDialog(stage);
        if (selectedDirectory != null) {
            downloadPathField.setText(selectedDirectory.getAbsolutePath());
            showAlert(Alert.AlertType.INFORMATION, "设置已保存", "下载目录已更新。");
        }
    }

    /**
     * 处理选择 yt-dlp 路径按钮点击事件。
     */
    @FXML
    private void handleChooseYtDlpPath() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("选择 yt-dlp 可执行文件");
        fileChooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("Executable Files", "*"));

        String currentPath = ytDlpPathField.getText();
        File initialDir = null;
        if (!currentPath.isEmpty()) {
            File currentFile = new File(currentPath);
            if (currentFile.exists()) {
                if (currentFile.isDirectory()) {
                    initialDir = currentFile;
                } else {
                    initialDir = currentFile.getParentFile();
                }
            }
        }
        if (initialDir == null || !initialDir.exists() || !initialDir.isDirectory()) {
            initialDir = new File(System.getProperty("user.home"));
        }
        fileChooser.setInitialDirectory(initialDir);

        Window stage = rootView.getScene().getWindow();
        File selectedFile = fileChooser.showOpenDialog(stage);
        if (selectedFile != null) {
            ytDlpPathField.setText(selectedFile.getAbsolutePath());
            showAlert(Alert.AlertType.INFORMATION, "设置已保存", "yt-dlp 路径已更新。");
        }
    }

    /**
     * 处理检测 yt-dlp 可用性按钮点击事件。
     */
    @FXML
    private void handleTestYtDlp() {
        String ytDlpPath = ytDlpPathField.getText();
        if (ytDlpPath.isEmpty()) {
            showAlert(Alert.AlertType.WARNING, "路径为空", "请输入 yt-dlp 路径。");
            return;
        }
        testTool(ytDlpPath, "yt-dlp", "--version");
    }

    /**
     * 处理选择 ffmpeg 路径按钮点击事件。
     */
    @FXML
    private void handleChooseFfmpegPath() {
        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("选择 ffmpeg 可执行文件");
        fileChooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("Executable Files", "*"));

        String currentPath = ffmpegPathField.getText();
        File initialDir = null;
        if (!currentPath.isEmpty()) {
            File currentFile = new File(currentPath);
            if (currentFile.exists()) {
                if (currentFile.isDirectory()) {
                    initialDir = currentFile;
                } else {
                    initialDir = currentFile.getParentFile();
                }
            }
        }
        if (initialDir == null || !initialDir.exists() || !initialDir.isDirectory()) {
            initialDir = new File(System.getProperty("user.home"));
        }
        fileChooser.setInitialDirectory(initialDir);

        Window stage = rootView.getScene().getWindow();
        File selectedFile = fileChooser.showOpenDialog(stage);
        if (selectedFile != null) {
            ffmpegPathField.setText(selectedFile.getAbsolutePath());
            showAlert(Alert.AlertType.INFORMATION, "设置已保存", "ffmpeg 路径已更新。");
        }
    }

    /**
     * 处理检测 ffmpeg 可用性按钮点击事件。
     */
    @FXML
    private void handleTestFfmpeg() {
        String ffmpegPath = ffmpegPathField.getText();
        if (ffmpegPath.isEmpty()) {
            showAlert(Alert.AlertType.WARNING, "路径为空", "请输入 ffmpeg 路径。");
            return;
        }
        testTool(ffmpegPath, "ffmpeg", "-version");
    }

    /**
     * 辅助方法：测试外部工具的可用性。
     */
    private void testTool(String toolPath, String toolName, String versionArg) {
        new Thread(() -> {
            try {
                ProcessBuilder pb;
                if (System.getProperty("os.name").toLowerCase().contains("win")) {
                    pb = new ProcessBuilder(toolPath, versionArg);
                } else {
                    pb = new ProcessBuilder(toolPath, versionArg);
                }

                pb.redirectErrorStream(true);
                Process process = pb.start();
                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
                StringBuilder output = new StringBuilder();
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }
                int exitCode = process.waitFor();

                Platform.runLater(() -> {
                    if (exitCode == 0) {
                        showAlert(Alert.AlertType.INFORMATION, "检测成功", toolName + " 运行正常。\n版本信息:\n" + output.toString().split("\n")[0]);
                    } else {
                        showAlert(Alert.AlertType.ERROR, "检测失败", toolName + " 运行失败，请检查路径或安装是否正确。\n错误信息:\n" + output.toString());
                    }
                });
            } catch (IOException | InterruptedException e) {
                Platform.runLater(() -> {
                    showAlert(Alert.AlertType.ERROR, "检测异常", "无法执行 " + toolName + "，请检查路径或权限。\n错误: " + e.getMessage());
                });
            }
        }).start();
    }

    /**
     * 更新“关于”页面中的版本信息。
     */
    private void updateAboutInfo() {
        appVersionLabel.setText("版本: 1.0.0 (Beta)"); // 你的应用版本
        getYtDlpAndFfmpegVersions();
    }

    private void getYtDlpAndFfmpegVersions() {
        new Thread(() -> {
            String ytDlpVer = getToolVersion(AppConfig.getYtDlpPath(), "--version");
            String ffmpegVer = getToolVersion(AppConfig.getFfmpegPath(), "-version");
            Platform.runLater(() -> {
                ytDlpVersionLabel.setText("yt-dlp 版本: " + (ytDlpVer.isEmpty() ? "未找到" : ytDlpVer));
                ffmpegVersionLabel.setText("ffmpeg 版本: " + (ffmpegVer.isEmpty() ? "未找到" : ffmpegVer));
            });
        }).start();
    }

    private String getToolVersion(String toolPath, String versionArg) {
        if (toolPath == null || toolPath.isEmpty()) {
            return "";
        }
        try {
            ProcessBuilder pb;
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                pb = new ProcessBuilder(toolPath, versionArg);
            } else {
                pb = new ProcessBuilder(toolPath, versionArg);
            }
            pb.redirectErrorStream(true);
            Process process = pb.start();
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            StringBuilder output = new StringBuilder();
            int limit = 0; // 只读取前几行
            while ((line = reader.readLine()) != null && limit < 5) {
                output.append(line).append("\n");
                limit++;
            }
            process.waitFor();
            reader.close();
            if (output.length() > 0) {
                String firstLine = output.toString().split("\n")[0];
                if (toolPath.toLowerCase().contains("yt-dlp")) {
                    return firstLine.replaceAll("yt-dlp ", "");
                } else if (toolPath.toLowerCase().contains("ffmpeg")) {
                    if (firstLine.contains("ffmpeg version")) {
                        return firstLine.substring(firstLine.indexOf("version") + 8).split(" ")[0];
                    }
                }
                return firstLine;
            }
        } catch (IOException | InterruptedException e) {
            System.err.println("Failed to get version for " + toolPath + ": " + e.getMessage());
        }
        return "";
    }


    /**
     * 处理检查更新按钮点击事件（占位）。
     */
    @FXML
    private void handleCheckForUpdates() {
        showAlert(Alert.AlertType.INFORMATION, "检查更新", "此功能尚未实现。请访问 GitHub 仓库获取最新版本。");
    }

    /**
     * 处理打开 GitHub 链接点击事件。
     */
    @FXML
    private void handleOpenGitHub() {
        try {
            java.awt.Desktop.getDesktop().browse(new java.net.URI("https://github.com/Yuan-Xinping/my-private-project")); // 替换为你的实际链接
        } catch (Exception e) {
            showAlert(Alert.AlertType.ERROR, "打开链接失败", "无法打开浏览器，请手动访问：https://github.com/Yuan-Xinping/my-private-project");
            e.printStackTrace();
        }
    }

    /**
     * 显示一个简单的提示框。
     */
    private void showAlert(Alert.AlertType type, String title, String message) {
        Platform.runLater(() -> {
            Alert alert = new Alert(type);
            alert.setTitle(title);
            alert.setHeaderText(null);
            alert.setContentText(message);
            alert.showAndWait();
        });
    }
}
