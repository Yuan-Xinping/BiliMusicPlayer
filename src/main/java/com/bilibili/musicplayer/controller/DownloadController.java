package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.service.BiliDownloader;
import com.bilibili.musicplayer.service.SongDAO;
import com.bilibili.musicplayer.service.BatchDownloadService;
import com.bilibili.musicplayer.util.AppConfig;

import javafx.application.Platform;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.*;
import javafx.scene.layout.HBox;
import javafx.stage.FileChooser;
import javafx.stage.Stage;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.core.type.TypeReference;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.ResourceBundle;
import java.util.Set;
import java.util.regex.Pattern;
import java.util.concurrent.atomic.AtomicInteger;

public class DownloadController implements Initializable {

    @FXML private HBox rootView;
    @FXML private TextField urlField;
    @FXML private Button downloadByUrlButton;
    @FXML private Button downloadByBvButton;
    @FXML private ProgressBar progressBar;
    @FXML private Label statusLabel;

    @FXML private Button btnImportFromFile;
    @FXML private Spinner<Integer> maxConcurrentDownloadsSpinner;
    @FXML private Button btnCancelBatchDownload;
    @FXML private ProgressBar batchOverallProgressBar;
    @FXML private Label batchOverallStatusLabel;
    @FXML private ListView<String> batchLogListView;

    private MainController mainController;
    private LibraryController libraryController;
    private SongDAO songDAO;
    private final ObjectMapper objectMapper = new ObjectMapper();

    private BatchDownloadService currentBatchDownloadService;

    private static final Pattern BV_ID_PATTERN = Pattern.compile("^BV[0-9a-zA-Z]{10}$");


    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("DownloadController: 初始化中...");
        progressBar.setProgress(0);
        statusLabel.setText("等待输入链接...");
        songDAO = new SongDAO();

        maxConcurrentDownloadsSpinner.setValueFactory(new SpinnerValueFactory.IntegerSpinnerValueFactory(1, 10, 3));

        btnCancelBatchDownload.setDisable(true);
        batchOverallProgressBar.setProgress(0);
        batchOverallStatusLabel.setText("无批量下载任务。");

        // 打印 AppConfig 中配置的路径，检查它们是否正确
        System.out.println("DownloadController: AppConfig.getYtDlpPath(): " + AppConfig.getYtDlpPath());
        System.out.println("DownloadController: AppConfig.getFfmpegPath(): " + AppConfig.getFfmpegPath());
        System.out.println("DownloadController: AppConfig.getDownloadPath(): " + AppConfig.getDownloadPath());
        System.out.println("DownloadController: 初始化完成。");
    }

    public void setMainController(MainController mainController) {
        this.mainController = mainController;
    }

    public void setLibraryController(LibraryController libraryController) {
        this.libraryController = libraryController;
    }

    /**
     * 处理“按链接下载”按钮点击事件
     */
    @FXML
    private void handleDownloadByUrlClick() {
        String input = urlField.getText().trim();
        System.out.println("DownloadController: '按链接下载'按钮被点击，输入: " + input);
        if (input.isEmpty()) {
            statusLabel.setText("请输入B站视频链接！");
            System.out.println("DownloadController: 输入为空，提示用户。");
            return;
        }

        if (!input.startsWith("http://") && !input.startsWith("https://")) {
            if (input.startsWith("www.bilibili.com/") || input.startsWith("m.bilibili.com/") || input.startsWith("b23.tv/")) {
                input = "https://" + input;
                System.out.println("DownloadController: 自动更正URL (添加 https://): " + input);
            } else {
                statusLabel.setText("请输入有效的B站链接 (以http://, https://, www.bilibili.com/, m.bilibili.com/ 或 b23.tv/ 开头)！");
                System.out.println("DownloadController: 无效的URL格式，提示用户。");
                return;
            }
        }

        startSingleDownload(input);
    }

    /**
     * 处理“按BV号下载”按钮点击事件
     */
    @FXML
    private void handleDownloadByBvClick() {
        String input = urlField.getText().trim();
        System.out.println("DownloadController: '按BV号下载'按钮被点击，输入: " + input);
        if (input.isEmpty()) {
            statusLabel.setText("请输入BV号！");
            System.out.println("DownloadController: BV号输入为空，提示用户。");
            return;
        }
        if (!BV_ID_PATTERN.matcher(input).matches()) {
            statusLabel.setText("请输入有效的BV号 (例如: BV1qD4y1U7fs)！");
            System.out.println("DownloadController: BV号格式无效，提示用户。");
            return;
        }
        startSingleDownload(input);
    }

    /**
     * 启动单个下载任务的通用方法
     * @param identifier 视频链接或BV号
     */
    private void startSingleDownload(String identifier) {
        System.out.println("DownloadController: 尝试启动单个下载任务，标识符: " + identifier);

        // 如果批量下载正在进行，阻止单个下载
        if (currentBatchDownloadService != null && currentBatchDownloadService.isRunning()) {
            Platform.runLater(() -> statusLabel.setText("请等待批量下载任务完成或手动取消。"));
            System.out.println("DownloadController: 批量下载正在进行，阻止单个下载。");
            return;
        }

        BiliDownloader.DownloadTask downloadTask = new BiliDownloader.DownloadTask(identifier);

        progressBar.progressProperty().bind(downloadTask.progressProperty());
        statusLabel.textProperty().bind(downloadTask.messageProperty());
        System.out.println("DownloadController: 绑定单个下载任务进度和状态到UI。");

        // 绑定单个下载UI控件到任务的运行状态
        downloadByUrlButton.disableProperty().bind(downloadTask.runningProperty());
        downloadByBvButton.disableProperty().bind(downloadTask.runningProperty());
        urlField.disableProperty().bind(downloadTask.runningProperty());
        System.out.println("DownloadController: 禁用单个下载相关UI控件。");

        // 手动禁用批量下载相关控件，因为它们不与单个任务绑定
        btnImportFromFile.setDisable(true);
        maxConcurrentDownloadsSpinner.setDisable(true);
        btnCancelBatchDownload.setDisable(true); // 单个下载时，批量取消按钮应禁用
        System.out.println("DownloadController: 禁用批量下载相关UI控件。");

        downloadTask.setOnSucceeded(event -> {
            System.out.println("DownloadController: 单个下载任务成功完成。");
            statusLabel.textProperty().unbind();
            progressBar.progressProperty().unbind();

            // 解除绑定并重新启用单个下载UI控件
            downloadByUrlButton.disableProperty().unbind();
            downloadByBvButton.disableProperty().unbind();
            urlField.disableProperty().unbind();
            downloadByUrlButton.setDisable(false);
            downloadByBvButton.setDisable(false);
            urlField.setDisable(false);
            System.out.println("DownloadController: 重新启用单个下载相关UI控件。");

            // 重新启用批量下载相关控件
            btnImportFromFile.setDisable(false);
            maxConcurrentDownloadsSpinner.setDisable(false);
            btnCancelBatchDownload.setDisable(true); // 确保在单个任务结束后取消按钮是禁用的
            System.out.println("DownloadController: 重新启用批量下载相关UI控件。");

            Song downloadedSong = downloadTask.getValue();
            if (downloadedSong != null) {
                System.out.println("DownloadController: 尝试保存下载的歌曲到数据库: " + downloadedSong.getTitle());
                if (songDAO.saveSong(downloadedSong)) {
                    statusLabel.setText("下载成功并已保存: " + downloadedSong.getTitle());
                    System.out.println("DownloadController: 下载完成的歌曲信息: " + downloadedSong);
                    if (libraryController != null) {
                        Platform.runLater(() -> libraryController.addSong(downloadedSong));
                        System.out.println("DownloadController: 通知LibraryController添加歌曲。");
                    } else {
                        System.err.println("DownloadController: LibraryController is not set. Cannot add song to library UI.");
                    }
                } else {
                    statusLabel.setText("下载成功，但保存到数据库失败: " + downloadedSong.getTitle());
                    System.err.println("DownloadController: 下载成功，但保存到数据库失败: " + downloadedSong.getTitle());
                }
            } else {
                statusLabel.setText("下载任务成功完成，但未返回歌曲信息。");
                System.err.println("DownloadController: 下载任务成功完成，但未返回歌曲信息 (downloadTask.getValue() is null)。");
            }
            progressBar.setProgress(1);
            urlField.setText("");
        });

        downloadTask.setOnFailed(event -> {
            System.err.println("DownloadController: 单个下载任务失败。");
            statusLabel.textProperty().unbind();
            progressBar.progressProperty().unbind();

            // 解除绑定并重新启用单个下载UI控件
            downloadByUrlButton.disableProperty().unbind();
            downloadByBvButton.disableProperty().unbind();
            urlField.disableProperty().unbind();
            downloadByUrlButton.setDisable(false);
            downloadByBvButton.setDisable(false);
            urlField.setDisable(false);
            System.out.println("DownloadController: 重新启用单个下载相关UI控件。");

            // 重新启用批量下载相关控件
            btnImportFromFile.setDisable(false);
            maxConcurrentDownloadsSpinner.setDisable(false);
            btnCancelBatchDownload.setDisable(true);
            System.out.println("DownloadController: 重新启用批量下载相关UI控件。");

            Throwable exception = downloadTask.getException();
            String errorMessage = "下载失败: " + (exception != null ? exception.getMessage() : "未知错误");
            statusLabel.setText(errorMessage);
            System.err.println("DownloadController: " + errorMessage);
            if (exception != null) {
                System.err.println("DownloadController: 异常堆栈跟踪:");
                exception.printStackTrace();
                Platform.runLater(() -> {
                    Alert alert = new Alert(Alert.AlertType.ERROR);
                    alert.setTitle("下载错误");
                    alert.setHeaderText("Bilibili 音频下载失败");
                    alert.setContentText(errorMessage + "\n\n请检查网络连接、Bilibili 链接是否有效，或确保 yt-dlp 和 ffmpeg 已正确安装并配置在系统 PATH 中或应用同级目录的bin/vlc文件夹中。");
                    alert.showAndWait();
                });
            }
            progressBar.setProgress(0);
        });

        downloadTask.setOnCancelled(event -> {
            System.out.println("DownloadController: 单个下载任务已取消。");
            statusLabel.textProperty().unbind();
            progressBar.progressProperty().unbind();

            // 解除绑定并重新启用单个下载UI控件
            downloadByUrlButton.disableProperty().unbind();
            downloadByBvButton.disableProperty().unbind();
            urlField.disableProperty().unbind();
            downloadByUrlButton.setDisable(false);
            downloadByBvButton.setDisable(false);
            urlField.setDisable(false);
            System.out.println("DownloadController: 重新启用单个下载相关UI控件。");

            // 重新启用批量下载相关控件
            btnImportFromFile.setDisable(false);
            maxConcurrentDownloadsSpinner.setDisable(false);
            btnCancelBatchDownload.setDisable(true);
            System.out.println("DownloadController: 重新启用批量下载相关UI控件。");

            statusLabel.setText("下载已取消。");
            progressBar.setProgress(0);
        });

        new Thread(downloadTask).start();
        System.out.println("DownloadController: 单个下载任务线程已启动。");
    }

    @FXML
    private void handleImportFromFile() {
        System.out.println("DownloadController: '从文件导入'按钮被点击。");
        // 如果单个下载正在进行，阻止启动批量下载
        if (progressBar.progressProperty().isBound()) {
            statusLabel.setText("请等待当前单个下载任务完成。");
            System.out.println("DownloadController: 检测到单个下载任务正在运行，阻止批量下载。");
            return;
        }

        // 避免启动多个批量下载任务
        if (currentBatchDownloadService != null && currentBatchDownloadService.isRunning()) {
            batchOverallStatusLabel.setText("已有批量下载任务正在进行中。");
            System.out.println("DownloadController: 检测到已有批量下载任务正在运行，阻止新的批量下载。");
            return;
        }

        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("选择导入的JSON文件");
        fileChooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("JSON Files", "*.json"));

        Stage stage = (Stage) rootView.getScene().getWindow();
        File file = fileChooser.showOpenDialog(stage);

        if (file != null) {
            System.out.println("DownloadController: 选择了导入文件: " + file.getAbsolutePath());
            batchLogListView.getItems().clear();
            batchOverallProgressBar.setProgress(0);
            batchOverallStatusLabel.setText("正在解析文件...");
            batchLogListView.getItems().add("正在解析导入文件: " + file.getName() + "...");

            // 禁用所有下载相关的UI控件
            setAllDownloadControlsDisabled(true);
            System.out.println("DownloadController: 禁用所有下载UI控件。");

            new Thread(() -> {
                try {
                    List<Map<String, String>> importedData = objectMapper.readValue(file, new TypeReference<List<Map<String, String>>>(){});
                    System.out.println("DownloadController: 成功读取JSON文件，共 " + importedData.size() + " 条数据。");

                    Set<String> uniqueBvIdentifiers = new HashSet<>();
                    final AtomicInteger invalidEntries = new AtomicInteger(0);

                    for (Map<String, String> entry : importedData) {
                        String id = entry.get("id");
                        if (id != null && BV_ID_PATTERN.matcher(id).matches()) {
                            uniqueBvIdentifiers.add(id);
                        } else {
                            invalidEntries.incrementAndGet();
                            Platform.runLater(() -> batchLogListView.getItems().add("跳过无效条目: " + (id != null ? id : "空ID或格式无效")));
                        }
                    }
                    System.out.println("DownloadController: 解析出 " + uniqueBvIdentifiers.size() + " 个有效BV号。");

                    if (uniqueBvIdentifiers.isEmpty()) {
                        Platform.runLater(() -> {
                            batchOverallStatusLabel.setText("导入文件中未找到有效的BV号。");
                            batchLogListView.getItems().add("错误: 导入文件中未找到有效的BV号。");
                            setAllDownloadControlsDisabled(false); // 重新启用控件
                        });
                        System.out.println("DownloadController: 未找到有效BV号，重新启用UI控件并返回。");
                        return;
                    }

                    List<String> bvListForDownload = new ArrayList<>(uniqueBvIdentifiers);
                    int maxConcurrent = maxConcurrentDownloadsSpinner.getValue();
                    System.out.println("DownloadController: 批量下载任务将处理 " + bvListForDownload.size() + " 个BV号，最大并发数: " + maxConcurrent);


                    Platform.runLater(() -> {
                        batchOverallStatusLabel.setText("已解析 " + bvListForDownload.size() + " 个有效BV号，开始批量下载...");
                        batchLogListView.getItems().add("已解析 " + bvListForDownload.size() + " 个有效BV号，开始批量下载...");
                        if (invalidEntries.get() > 0) { // 获取计数器的值
                            batchLogListView.getItems().add("注意: 文件中包含 " + invalidEntries.get() + " 个无效或重复的条目已跳过。");
                        }
                        btnCancelBatchDownload.setDisable(false);
                    });

                    currentBatchDownloadService = new BatchDownloadService(bvListForDownload, songDAO, maxConcurrent);
                    System.out.println("DownloadController: BatchDownloadService 实例创建成功。");

                    // 绑定批量服务属性到 UI
                    Platform.runLater(() -> {
                        batchOverallProgressBar.progressProperty().bind(currentBatchDownloadService.progressProperty());
                        batchOverallStatusLabel.textProperty().bind(currentBatchDownloadService.messageProperty());
                        batchLogListView.setItems(currentBatchDownloadService.getLogMessages()); // 使用 setItems 设置 ObservableList
                        System.out.println("DownloadController: 批量下载服务属性已绑定到UI。");
                    });


                    currentBatchDownloadService.setOnSucceeded(e -> {
                        Platform.runLater(() -> {
                            System.out.println("DownloadController: 批量下载任务成功完成。");
                            batchOverallStatusLabel.textProperty().unbind();
                            batchOverallProgressBar.progressProperty().unbind();
                            batchLogListView.itemsProperty().unbind(); // 解除绑定
                            batchOverallProgressBar.setProgress(1.0);
                            batchOverallStatusLabel.setText(currentBatchDownloadService.getMessage() + " 批量下载完成。");
                            setAllDownloadControlsDisabled(false); // 重新启用所有控件
                            btnCancelBatchDownload.setDisable(true); // 禁用取消按钮
                            // 批量下载完成后刷新音乐库
                            if (libraryController != null) {
                                libraryController.refreshSongs();
                                System.out.println("DownloadController: 通知LibraryController刷新歌曲。");
                            }
                            currentBatchDownloadService = null; // 清除引用
                        });
                    });

                    currentBatchDownloadService.setOnFailed(e -> {
                        Platform.runLater(() -> {
                            System.err.println("DownloadController: 批量下载任务失败。");
                            batchOverallStatusLabel.textProperty().unbind();
                            batchOverallProgressBar.progressProperty().unbind();
                            batchLogListView.itemsProperty().unbind(); // 解除绑定
                            batchOverallProgressBar.setProgress(0.0);
                            Throwable exception = currentBatchDownloadService.getException();
                            String errMsg = (exception != null ? exception.getMessage() : "未知错误");
                            batchOverallStatusLabel.setText("批量下载失败: " + errMsg);
                            System.err.println("DownloadController: 批量下载失败: " + errMsg);
                            if (exception != null) {
                                System.err.println("DownloadController: 批量下载异常堆栈跟踪:");
                                exception.printStackTrace();
                            }
                            setAllDownloadControlsDisabled(false); // 重新启用所有控件
                            btnCancelBatchDownload.setDisable(true); // 禁用取消按钮
                            currentBatchDownloadService = null; // 清除引用
                        });
                    });

                    currentBatchDownloadService.setOnCancelled(e -> {
                        Platform.runLater(() -> {
                            System.out.println("DownloadController: 批量下载任务已取消。");
                            batchOverallStatusLabel.textProperty().unbind();
                            batchOverallProgressBar.progressProperty().unbind();
                            batchLogListView.itemsProperty().unbind(); // 解除绑定
                            batchOverallProgressBar.setProgress(0.0);
                            batchOverallStatusLabel.setText("批量下载已取消。");
                            setAllDownloadControlsDisabled(false); // 重新启用所有控件
                            btnCancelBatchDownload.setDisable(true); // 禁用取消按钮
                            currentBatchDownloadService = null; // 清除引用
                        });
                    });

                    new Thread(currentBatchDownloadService).start();
                    System.out.println("DownloadController: 批量下载服务线程已启动。");

                } catch (IOException e) {
                    Platform.runLater(() -> {
                        batchOverallStatusLabel.setText("读取或解析JSON文件失败: " + e.getMessage());
                        batchLogListView.getItems().add("错误: 读取或解析JSON文件失败: " + e.getMessage());
                        System.err.println("DownloadController: 读取或解析JSON文件失败:");
                        e.printStackTrace();
                        setAllDownloadControlsDisabled(false); // 重新启用控件
                    });
                }
            }).start();
        } else {
            batchOverallStatusLabel.setText("文件导入操作已取消。");
            System.out.println("DownloadController: 文件选择器已取消。");
            setAllDownloadControlsDisabled(false); // 确保取消后控件恢复
        }
    }

    @FXML
    private void handleCancelBatchDownload() {
        System.out.println("DownloadController: '取消批量下载'按钮被点击。");
        if (currentBatchDownloadService != null && currentBatchDownloadService.isRunning()) {
            currentBatchDownloadService.cancel();
            btnCancelBatchDownload.setDisable(true); // 立即禁用取消按钮
            System.out.println("DownloadController: 已请求取消批量下载任务。");
        } else {
            System.out.println("DownloadController: 无批量下载任务正在运行，无需取消。");
        }
    }

    /**
     * 辅助方法：禁用/启用所有下载相关的UI控件。
     * 此方法主要用于批量下载任务的开始和结束时，统一管理UI状态。
     * 单个下载任务的UI控件禁用状态由其自身的 `runningProperty` 绑定管理。
     * @param disabled true 为禁用，false 为启用
     */
    private void setAllDownloadControlsDisabled(boolean disabled) {
        System.out.println("DownloadController: setAllDownloadControlsDisabled(" + disabled + ") called.");
        // 单个下载控件 (在批量下载时需要禁用)
        downloadByUrlButton.setDisable(disabled);
        downloadByBvButton.setDisable(disabled);
        urlField.setDisable(disabled);

        // 批量下载控件
        btnImportFromFile.setDisable(disabled);
        maxConcurrentDownloadsSpinner.setDisable(disabled);
    }

    public HBox getView() {
        return rootView;
    }
}
