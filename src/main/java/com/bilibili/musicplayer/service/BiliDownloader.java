package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.util.AppConfig;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import javafx.concurrent.Task;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class BiliDownloader {

    private static File getDownloadDir() {
        File dir = new File(AppConfig.getDownloadPath());
        if (!dir.exists()) {
            boolean created = dir.mkdirs();
            System.out.println("BiliDownloader: 下载目录 '" + dir.getAbsolutePath() + "' 不存在，尝试创建: " + created);
        } else {
            System.out.println("BiliDownloader: 下载目录 '" + dir.getAbsolutePath() + "' 已存在。");
        }
        return dir;
    }

    private static final ObjectMapper objectMapper = new ObjectMapper();

    private static final Pattern PROGRESS_PATTERN = Pattern.compile("\\[download\\]\\s*(\\d+\\.\\d+)%");

    private static final Pattern INFO_JSON_PATH_PATTERN = Pattern.compile("^\\[info\\] Writing video metadata as JSON to: (.+)$");

    private static final Pattern FINAL_AUDIO_PATH_PATTERN = Pattern.compile("^\\[EmbedThumbnail\\] ffmpeg: Adding thumbnail to \"(.*?)\"$");


    /**
     * 用于异步下载的 Task 类
     */
    public static class DownloadTask extends Task<Song> {
        private final String bilibiliIdentifier;

        // 用于存储捕获到的 info.json 文件的路径 (临时路径)
        private String tempInfoJsonPath;
        // 用于存储最终下载的 MP3 文件的路径 (临时路径)
        private String tempDownloadedFilePath;

        // 存储视频的ID，用于构建临时文件名
        private String videoId;

        public DownloadTask(String bilibiliIdentifier) {
            this.bilibiliIdentifier = bilibiliIdentifier;
            updateMessage("准备下载...");
            updateProgress(0, 1);
            System.out.println("DownloadTask: 新建下载任务，标识符: " + bilibiliIdentifier);
        }

        @Override
        protected Song call() throws Exception {
            updateMessage("开始下载: " + bilibiliIdentifier);
            System.out.println("DownloadTask: call() 方法开始执行，下载标识符: " + bilibiliIdentifier);

            File ytDlpFile = new File(AppConfig.getYtDlpPath());
            File ffmpegFile = new File(AppConfig.getFfmpegPath());

            if (!ytDlpFile.exists() || !ytDlpFile.canExecute()) {
                String errorMsg = "yt-dlp 可执行文件不存在或不可执行: " + ytDlpFile.getAbsolutePath();
                System.err.println("DownloadTask Error: " + errorMsg);
                throw new IOException(errorMsg);
            }
            if (!ffmpegFile.exists() || !ffmpegFile.canExecute()) {
                String errorMsg = "ffmpeg 可执行文件不存在或不可执行: " + ffmpegFile.getAbsolutePath();
                System.err.println("DownloadTask Error: " + errorMsg);
                throw new IOException(errorMsg);
            }
            System.out.println("DownloadTask: yt-dlp 和 ffmpeg 路径检查通过。");

            if (bilibiliIdentifier.startsWith("BV")) {
                videoId = bilibiliIdentifier;
            } else {
                Pattern bilibiliIdPattern = Pattern.compile("(BV[0-9a-zA-Z]{10}|av\\d+)");
                Matcher matcher = bilibiliIdPattern.matcher(bilibiliIdentifier);
                if (matcher.find()) {
                    videoId = matcher.group(1);
                } else {
                    videoId = "unknown_id_" + System.currentTimeMillis(); // 回退方案
                }
            }
            System.out.println("DownloadTask: 视频ID (用于临时文件名): " + videoId);

            // 构建 yt-dlp 命令
            List<String> ytDlpArgs = new ArrayList<>();
            ytDlpArgs.add("--ffmpeg-location");
            ytDlpArgs.add("\"" + AppConfig.getFfmpegPath() + "\"");
            ytDlpArgs.add("-x"); // 提取音频
            ytDlpArgs.add("--audio-format");
            ytDlpArgs.add("mp3");
            ytDlpArgs.add("--audio-quality");
            ytDlpArgs.add("0");
            ytDlpArgs.add("--embed-thumbnail");
            ytDlpArgs.add("--write-info-json");

            String tempOutputTemplate = getDownloadDir().getAbsolutePath() + File.separator + "temp_" + videoId + ".%(ext)s";
            ytDlpArgs.add("--output");
            ytDlpArgs.add("\"" + tempOutputTemplate + "\"");

            String finalIdentifierForYtDlp = bilibiliIdentifier;
            if (finalIdentifierForYtDlp.startsWith("BV") && finalIdentifierForYtDlp.length() == 12 && !finalIdentifierForYtDlp.startsWith("http")) {
                finalIdentifierForYtDlp = "https://www.bilibili.com/video/" + finalIdentifierForYtDlp;
            }
            ytDlpArgs.add("\"" + finalIdentifierForYtDlp + "\"");

            String ytDlpCommandString = "\"" + AppConfig.getYtDlpPath() + "\" " + String.join(" ", ytDlpArgs);

            List<String> command = new ArrayList<>();
            command.add("cmd.exe");
            command.add("/c");
            command.add("chcp 65001 > nul && " + ytDlpCommandString); // 保持 chcp 65001 以便 yt-dlp 输出是 UTF-8

            System.out.println("DownloadTask: 完整的 cmd 命令: " + String.join(" ", command));

            ProcessBuilder processBuilder = new ProcessBuilder(command);
            processBuilder.redirectErrorStream(true);

            Process process = null;
            StringBuilder fullOutput = new StringBuilder();
            try {
                process = processBuilder.start();
                System.out.println("DownloadTask: yt-dlp 进程已启动。");

                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8));

                String line;
                while ((line = reader.readLine()) != null) {
                    if (isCancelled()) {
                        System.out.println("DownloadTask: 任务被取消，尝试终止 yt-dlp 进程。");
                        process.destroy();
                        updateMessage("下载已取消。");
                        return null;
                    }
                    System.out.println("yt-dlp Output: " + line);
                    fullOutput.append(line).append("\n");

                    Matcher progressMatcher = PROGRESS_PATTERN.matcher(line);
                    if (progressMatcher.find()) {
                        try {
                            double progress = Double.parseDouble(progressMatcher.group(1));
                            updateProgress(progress / 100.0, 1.0);
                            updateMessage("下载中: " + String.format("%.1f", progress) + "%");
                        } catch (NumberFormatException e) {
                            System.err.println("DownloadTask Error: 无法解析进度百分比: " + line);
                        }
                    }

                    // 捕获 info.json 文件的临时路径
                    Matcher infoJsonMatcher = INFO_JSON_PATH_PATTERN.matcher(line);
                    if (infoJsonMatcher.find()) {
                        tempInfoJsonPath = infoJsonMatcher.group(1).trim();
                        System.out.println("DownloadTask: 捕获到临时 info.json 路径: " + tempInfoJsonPath);
                    }

                    // 捕获最终下载的 MP3 文件的临时路径
                    Matcher finalAudioPathMatcher = FINAL_AUDIO_PATH_PATTERN.matcher(line);
                    if (finalAudioPathMatcher.find()) {
                        tempDownloadedFilePath = finalAudioPathMatcher.group(1).trim();
                        System.out.println("DownloadTask: 捕获到临时MP3文件路径 (从 EmbedThumbnail): " + tempDownloadedFilePath);
                    }
                }

                int exitCode = process.waitFor();
                reader.close();
                System.out.println("DownloadTask: yt-dlp 进程退出，退出码: " + exitCode);

                if (exitCode == 0) {
                    if (tempInfoJsonPath == null || tempInfoJsonPath.isEmpty()) {
                        String errorMsg = "yt-dlp 进程成功退出，但未能从其输出中捕获到临时 info.json 文件的路径。";
                        System.err.println("DownloadTask Error: " + errorMsg);
                        throw new IOException(errorMsg + "\n完整输出:\n" + fullOutput.toString());
                    }
                    if (tempDownloadedFilePath == null || tempDownloadedFilePath.isEmpty()) {
                        String errorMsg = "yt-dlp 进程成功退出，但未能从其输出中捕获到最终临时音频文件的路径。";
                        System.err.println("DownloadTask Error: " + errorMsg);
                        throw new IOException(errorMsg + "\n完整输出:\n" + fullOutput.toString());
                    }

                    updateMessage("下载完成，正在处理元数据并重命名文件...");
                    // 调用解析元数据和重命名文件的方法
                    return parseMetadataAndRenameFile(tempInfoJsonPath, tempDownloadedFilePath);
                } else {
                    System.err.println("DownloadTask Error: yt-dlp 下载失败，退出码: " + exitCode);
                    System.err.println("DownloadTask Error: yt-dlp 完整输出:\n" + fullOutput.toString());
                    throw new IOException("yt-dlp 下载失败，退出码: " + exitCode + "\n完整输出:\n" + fullOutput.toString());
                }

            } catch (IOException | InterruptedException e) {
                System.err.println("DownloadTask Error: 下载过程中发生异常: " + e.getMessage());
                System.err.println("DownloadTask Error: yt-dlp 完整输出 (异常前):\n" + fullOutput.toString());
                updateMessage("下载过程中发生错误: " + e.getMessage());
                throw new Exception("下载失败: " + e.getMessage(), e);
            } finally {
                if (process != null) {
                    process.destroy();
                    System.out.println("DownloadTask: yt-dlp 进程已销毁。");
                }
                // 无论成功失败，尝试删除临时 info.json 文件（如果存在）
                if (tempInfoJsonPath != null) {
                    File tempJsonFile = new File(tempInfoJsonPath);
                    if (tempJsonFile.exists()) {
                        try {
                            Files.delete(tempJsonFile.toPath());
                            System.out.println("DownloadTask: 临时 info.json 文件已清理: " + tempInfoJsonPath);
                        } catch (IOException e) {
                            System.err.println("DownloadTask Error: 无法删除临时 info.json 文件: " + tempInfoJsonPath + " - " + e.getMessage());
                        }
                    }
                }
            }
        }

        /**
         * 从指定的临时 .info.json 文件解析元数据，并使用传入的临时文件路径进行重命名。
         * @param tempInfoJsonPath 临时 .info.json 文件的完整路径
         * @param tempAudioFilePath 临时下载的音频文件的完整路径
         */
        private Song parseMetadataAndRenameFile(String tempInfoJsonPath, String tempAudioFilePath) throws IOException {
            System.out.println("DownloadTask: 开始解析元数据并重命名。临时 infoJsonPath: " + tempInfoJsonPath + ", 临时 tempAudioFilePath: " + tempAudioFilePath);

            File infoJsonFile = new File(tempInfoJsonPath);
            if (!infoJsonFile.exists()) {
                String errorMsg = "未找到临时元数据文件: " + infoJsonFile.getAbsolutePath();
                System.err.println("DownloadTask Error: " + errorMsg);
                throw new IOException(errorMsg);
            }
            System.out.println("DownloadTask: 找到临时 info.json 文件: " + infoJsonFile.getAbsolutePath());

            JsonNode rootNode = objectMapper.readTree(infoJsonFile);
            System.out.println("DownloadTask: 成功读取临时 info.json 内容。");

            String id = rootNode.has("id") ? rootNode.get("id").asText() : "N/A";
            String title = rootNode.has("title") ? rootNode.get("title").asText() : "未知标题";
            String artist = rootNode.has("uploader") ? rootNode.get("uploader").asText() : "未知艺术家";
            String url = rootNode.has("webpage_url") ? rootNode.get("webpage_url").asText() : bilibiliIdentifier;
            String coverUrl = rootNode.has("thumbnail") ? rootNode.get("thumbnail").asText() : null;
            long duration = rootNode.has("duration") ? rootNode.get("duration").asLong() : 0;

            System.out.println("DownloadTask: 解析元数据结果:");
            System.out.println("  ID: " + id);
            System.out.println("  Title: " + title);
            System.out.println("  Artist: " + artist);
            System.out.println("  URL: " + url);
            System.out.println("  Cover URL: " + coverUrl);
            System.out.println("  Duration: " + duration + " seconds");

            String sanitizedTitle = sanitizeFilename(title);
            String newFileName = sanitizedTitle + ".mp3";
            Path newFilePath = Paths.get(getDownloadDir().getAbsolutePath(), newFileName);

            Path originalTempPath = Paths.get(tempAudioFilePath);

            if (!Files.exists(originalTempPath)) {
                String errorMsg = "临时下载文件不存在，无法重命名: " + originalTempPath.toString();
                System.err.println("DownloadTask Error: " + errorMsg);
                throw new IOException(errorMsg);
            }

            System.out.println("DownloadTask: 尝试将临时文件 '" + originalTempPath + "' 重命名为 '" + newFilePath + "'");
            try {
                // 使用 Files.move 进行重命名，确保原子性操作
                Files.move(originalTempPath, newFilePath, StandardCopyOption.REPLACE_EXISTING);
                System.out.println("DownloadTask: 文件重命名成功。");
            } catch (IOException e) {
                String errorMsg = "文件重命名失败: " + e.getMessage();
                System.err.println("DownloadTask Error: " + errorMsg);
                throw new IOException(errorMsg);
            }

            updateMessage("元数据解析和文件重命名成功。");
            return new Song(id, title, artist, url, newFilePath.toString(), coverUrl, duration, LocalDateTime.now(),false);
        }

        /**
         * 清理字符串，使其符合 Windows 文件名规范。
         * 移除或替换不允许的字符。
         */
        private String sanitizeFilename(String filename) {
            String sanitized = filename.replaceAll("[<>:\"/\\\\|?*]", "_");
            sanitized = sanitized.replace(File.separatorChar, '_');
            return sanitized;
        }
    }
}
