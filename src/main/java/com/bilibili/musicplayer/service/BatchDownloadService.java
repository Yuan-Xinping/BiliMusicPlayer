package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Song;
import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.concurrent.Task;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.regex.Pattern;

public class BatchDownloadService extends Task<Void> {

    private final List<String> bilibiliIdentifiers;
    private final SongDAO songDAO;
    private final int maxConcurrentDownloads;

    private ExecutorService downloadExecutor;
    private final ObservableList<String> logMessages = FXCollections.observableArrayList();

    // 用于跟踪任务状态
    private final AtomicInteger totalTasks = new AtomicInteger(0);
    private final AtomicInteger completedTasks = new AtomicInteger(0); // 成功下载并保存
    private final AtomicInteger skippedTasks = new AtomicInteger(0);   // 已存在或被取消
    private final AtomicInteger failedTasks = new AtomicInteger(0);    // 下载失败或保存失败

    // 用于基本 BV 号验证的正则表达式
    private static final Pattern BV_ID_PATTERN = Pattern.compile("^BV[0-9a-zA-Z]{10}$");


    public BatchDownloadService(List<String> bilibiliIdentifiers, SongDAO songDAO, int maxConcurrentDownloads) {
        this.bilibiliIdentifiers = bilibiliIdentifiers;
        this.songDAO = songDAO;
        this.maxConcurrentDownloads = Math.max(1, maxConcurrentDownloads); // 至少1个并发下载
        updateMessage("准备批量下载...");
        updateProgress(0, 1);
        Platform.runLater(() -> logMessages.add("批量下载任务已启动，最大并发数: " + this.maxConcurrentDownloads));
        System.out.println("BatchDownloadService: 批量下载服务实例创建，总 BV 号: " + bilibiliIdentifiers.size() + ", 并发数: " + this.maxConcurrentDownloads);
    }

    // 暴露日志信息给 UI
    public ObservableList<String> getLogMessages() {
        return logMessages;
    }

    @Override
    protected Void call() throws Exception {
        totalTasks.set(bilibiliIdentifiers.size());
        updateMessage(String.format("总计 %d 个任务待处理...", totalTasks.get()));
        updateProgress(0, totalTasks.get());
        System.out.println("BatchDownloadService: call() 方法开始执行，总任务数: " + totalTasks.get());

        // 使用固定大小的线程池来限制并发下载数
        downloadExecutor = Executors.newFixedThreadPool(maxConcurrentDownloads);
        System.out.println("BatchDownloadService: 线程池创建成功，大小: " + maxConcurrentDownloads);

        for (String identifier : bilibiliIdentifiers) {
            if (isCancelled()) {
                System.out.println("BatchDownloadService: 任务被取消，停止处理新的下载请求。");
                updateMessage("批量下载已取消。");
                break;
            }
            System.out.println("BatchDownloadService: 处理 BV 号: " + identifier);

            // 基本 BV 号格式验证
            if (!BV_ID_PATTERN.matcher(identifier).matches()) {
                Platform.runLater(() -> logMessages.add(String.format("BV号 %s: 格式无效，已跳过。", identifier)));
                skippedTasks.incrementAndGet();
                updateOverallProgress();
                System.out.println("BatchDownloadService: BV号格式无效，跳过: " + identifier);
                continue;
            }

            // 1. 数据库去重检查 (在提交任务前进行，避免不必要的下载)
            Song existingSong = songDAO.getSongById(identifier); // 使用 SongDAO 中新的公共方法
            if (existingSong != null) {
                Platform.runLater(() -> logMessages.add(String.format("BV号 %s: 已跳过 (歌曲 '%s' 已存在于音乐库)", identifier, existingSong.getTitle())));
                skippedTasks.incrementAndGet();
                updateOverallProgress();
                System.out.println("BatchDownloadService: 歌曲已存在于数据库，跳过: " + identifier + " - " + existingSong.getTitle());
                continue; // 跳过此任务
            }
            System.out.println("BatchDownloadService: BV号 " + identifier + " 不在数据库中，准备下载。");

            // 2. 创建并提交下载任务
            BiliDownloader.DownloadTask downloadTask = new BiliDownloader.DownloadTask(identifier);

            // 监听单个下载任务的进度和状态，并更新批量任务的日志
            downloadTask.messageProperty().addListener((obs, oldVal, newVal) ->
                    Platform.runLater(() -> logMessages.add(String.format("BV号 %s: %s", identifier, newVal)))
            );

            downloadTask.setOnSucceeded(e -> {
                Song downloadedSong = downloadTask.getValue();
                if (downloadedSong != null) {
                    System.out.println("BatchDownloadService: BV号 " + identifier + " 下载成功，尝试保存到数据库。");
                    if (songDAO.saveSong(downloadedSong)) {
                        Platform.runLater(() -> {
                            logMessages.add(String.format("BV号 %s: 下载成功并保存 - '%s'", identifier, downloadedSong.getTitle()));
                            // 这里不直接通知 LibraryController，而是由 DownloadController 在批量任务结束后统一刷新
                        });
                        System.out.println("BatchDownloadService: BV号 " + identifier + " 成功保存到数据库。");
                    } else {
                        Platform.runLater(() -> logMessages.add(String.format("BV号 %s: 下载成功但保存到数据库失败 - '%s'", identifier, downloadedSong.getTitle())));
                        failedTasks.incrementAndGet();
                        System.err.println("BatchDownloadService Error: BV号 " + identifier + " 下载成功但保存到数据库失败。");
                    }
                } else {
                    Platform.runLater(() -> logMessages.add(String.format("BV号 %s: 下载任务成功完成，但未返回歌曲信息。", identifier)));
                    failedTasks.incrementAndGet();
                    System.err.println("BatchDownloadService Error: BV号 " + identifier + " 下载任务成功完成，但未返回歌曲信息 (downloadTask.getValue() is null)。");
                }
                completedTasks.incrementAndGet();
                updateOverallProgress();
            });

            downloadTask.setOnFailed(e -> {
                Throwable ex = downloadTask.getException();
                String errMsg = (ex != null ? ex.getMessage() : "未知错误");
                Platform.runLater(() -> logMessages.add(String.format("BV号 %s: 下载失败 - %s", identifier, errMsg)));
                failedTasks.incrementAndGet();
                updateOverallProgress();
                System.err.println("BatchDownloadService Error: BV号 " + identifier + " 下载失败: " + errMsg);
                if (ex != null) ex.printStackTrace();
            });

            downloadTask.setOnCancelled(e -> {
                Platform.runLater(() -> logMessages.add(String.format("BV号 %s: 下载已取消。", identifier)));
                skippedTasks.incrementAndGet(); // 视为跳过
                updateOverallProgress();
                System.out.println("BatchDownloadService: BV号 " + identifier + " 下载已取消。");
            });

            downloadExecutor.submit(downloadTask);
            System.out.println("BatchDownloadService: 已提交 BV 号 " + identifier + " 到下载队列。");
        }

        // 关闭线程池，不再接受新任务
        downloadExecutor.shutdown();
        System.out.println("BatchDownloadService: 下载线程池已关闭，等待所有任务完成...");
        try {
            // 等待所有已提交的任务完成，设置超时时间
            if (!downloadExecutor.awaitTermination(2, TimeUnit.HOURS)) { // 增加超时时间以应对长时间下载
                Platform.runLater(() -> logMessages.add("部分下载任务未能及时完成，可能仍在后台运行。"));
                updateMessage("批量下载完成 (部分任务超时)。");
                System.out.println("BatchDownloadService: 线程池等待超时。");
            } else {
                updateMessage("所有批量下载任务已完成。");
                System.out.println("BatchDownloadService: 所有下载任务已完成。");
            }
        } catch (InterruptedException e) {
            Platform.runLater(() -> logMessages.add("批量下载等待被中断。"));
            updateMessage("批量下载被中断。");
            System.err.println("BatchDownloadService Error: 批量下载等待被中断。");
            Thread.currentThread().interrupt(); // 重新设置中断标志
        }

        updateOverallProgress(); // 确保最终进度更新
        return null;
    }

    private void updateOverallProgress() {
        double currentProcessed = completedTasks.get() + skippedTasks.get() + failedTasks.get();
        double total = totalTasks.get();
        // 避免除以零
        double progressValue = (total > 0) ? currentProcessed / total : 0.0;

        updateProgress(progressValue, 1.0); // 进度条范围是0到1
        updateMessage(String.format("总计 %d 个任务 | 完成: %d | 跳过: %d | 失败: %d",
                totalTasks.get(), completedTasks.get(), skippedTasks.get(), failedTasks.get()));
        System.out.println(String.format("BatchDownloadService Progress: 已处理 %d/%d (完成: %d, 跳过: %d, 失败: %d)",
                (int)currentProcessed, totalTasks.get(), completedTasks.get(), skippedTasks.get(), failedTasks.get()));
    }

    @Override
    protected void cancelled() {
        super.cancelled();
        System.out.println("BatchDownloadService: 批量下载任务被取消回调。");
        if (downloadExecutor != null) {
            downloadExecutor.shutdownNow(); // 尝试立即停止所有正在运行的任务
            Platform.runLater(() -> logMessages.add("批量下载已请求取消，正在停止所有进行中的任务。"));
            System.out.println("BatchDownloadService: 线程池 shutdownNow() 已调用。");
        }
        updateMessage("批量下载已取消。");
        updateOverallProgress();
    }
}
