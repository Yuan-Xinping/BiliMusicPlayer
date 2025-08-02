// src/main/java/com/bilibili/musicplayer/service/BiliDownloader.java
package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Song;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import javafx.concurrent.Task;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class BiliDownloader {

    // 默认下载目录：用户主目录下的 BiliMusicPlayer_Downloads 文件夹
    private static final String DOWNLOAD_DIR_PATH = System.getProperty("user.home") + File.separator + "BiliMusicPlayer_Downloads";
    private static final File DOWNLOAD_DIR = new File(DOWNLOAD_DIR_PATH);

    // FFMPEG 路径
    private static final String FFMPEG_EXECUTABLE_PATH = "C:\\ffmpeg\\ffmpeg-7.1.1-essentials_build\\bin";
    private static final ObjectMapper objectMapper = new ObjectMapper();

    // 确保下载目录存在
    static {
        if (!DOWNLOAD_DIR.exists()) {
            DOWNLOAD_DIR.mkdirs();
        }
    }

    /**
     * 用于异步下载的 Task 类
     */
    public static class DownloadTask extends Task<Song> {
        private final String bilibiliIdentifier;

        // 用于存储捕获到的 info.json 文件的路径
        private String infoJsonPath;
        // 用于存储最终下载的 MP3 文件的路径
        private String finalDownloadedFilePath;

        // 正则表达式用于解析yt-dlp的进度输出
        private static final Pattern PROGRESS_PATTERN = Pattern.compile("\\[download\\]\\s*(\\d+\\.\\d+)%");
        // 正则表达式用于捕获 .info.json 文件的路径
        private static final Pattern INFO_JSON_PATH_PATTERN = Pattern.compile("Writing video metadata as JSON to: (.*)");
        // 新增：正则表达式用于捕获 [EmbedThumbnail] 行中的最终文件路径
        private static final Pattern EMBED_THUMBNAIL_PATH_PATTERN = Pattern.compile("\\[EmbedThumbnail\\] ffmpeg: Adding thumbnail to \"(.*?)\"");


        public DownloadTask(String bilibiliIdentifier) {
            this.bilibiliIdentifier = bilibiliIdentifier;
            updateMessage("准备下载...");
            updateProgress(0, 1);
        }

        @Override
        protected Song call() throws Exception {
            updateMessage("开始下载: " + bilibiliIdentifier);

            List<String> command = new ArrayList<>();
            command.add("yt-dlp");
            command.add("--ffmpeg-location"); // 告诉 yt-dlp ffmpeg 的位置
            command.add(FFMPEG_EXECUTABLE_PATH); // 使用你定义的常量
            command.add("-x"); // 提取音频
            command.add("--audio-format"); // 音频格式
            command.add("mp3"); // 转换为mp3
            command.add("--embed-thumbnail"); // 嵌入封面
            command.add("--write-info-json"); // 写入元数据json文件
            command.add("--output"); // 输出路径模板
            // %(title)s - 视频标题, %(ext)s - 扩展名, %(id)s - 视频ID
            command.add(DOWNLOAD_DIR_PATH + File.separator + "%(title)s.%(ext)s");

            String finalIdentifierForYtDlp = bilibiliIdentifier;
            if (finalIdentifierForYtDlp.startsWith("BV") && finalIdentifierForYtDlp.length() == 12 && !finalIdentifierForYtDlp.startsWith("http")) {
                finalIdentifierForYtDlp = "https://www.bilibili.com/video/" + finalIdentifierForYtDlp;
                System.out.println("Processed BV identifier for yt-dlp: " + finalIdentifierForYtDlp); // 调试信息
            }
            command.add(finalIdentifierForYtDlp); // 使用处理后的标识符

            ProcessBuilder processBuilder = new ProcessBuilder(command);
            processBuilder.redirectErrorStream(true); // 将错误流合并到标准输出流，方便统一处理进度和信息

            Process process = null;
            StringBuilder fullOutput = new StringBuilder(); // 用于收集所有输出，方便调试
            try {
                process = processBuilder.start();

                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream(),"GBK"));
                String line;
                while ((line = reader.readLine()) != null) {
                    if (isCancelled()) {
                        process.destroy(); // 尝试终止进程
                        updateMessage("下载已取消。");
                        return null;
                    }
                    System.out.println("yt-dlp: " + line); // 打印所有输出到控制台
                    fullOutput.append(line).append("\n"); // 收集所有输出

                    Matcher progressMatcher = PROGRESS_PATTERN.matcher(line);
                    if (progressMatcher.find()) {
                        double progress = Double.parseDouble(progressMatcher.group(1));
                        updateProgress(progress / 100.0, 1.0);
                        updateMessage("下载中: " + String.format("%.1f", progress) + "%");
                    }

                    // 捕获 info.json 文件的路径
                    Matcher infoJsonMatcher = INFO_JSON_PATH_PATTERN.matcher(line);
                    if (infoJsonMatcher.find()) {
                        infoJsonPath = infoJsonMatcher.group(1);
                        System.out.println("Captured info.json path: " + infoJsonPath); // 调试信息
                    }

                    // 捕获最终下载的 MP3 文件路径（从 EmbedThumbnail 行）
                    Matcher embedThumbnailPathMatcher = EMBED_THUMBNAIL_PATH_PATTERN.matcher(line);
                    if (embedThumbnailPathMatcher.find()) {
                        finalDownloadedFilePath = embedThumbnailPathMatcher.group(1);
                        System.out.println("Captured final downloaded MP3 path from EmbedThumbnail: " + finalDownloadedFilePath); // 调试信息
                    }
                }

                int exitCode = process.waitFor();
                reader.close();

                if (exitCode == 0) {
                    updateMessage("下载完成，正在处理元数据...");
                    // 调用解析元数据的方法，并传入捕获到的 info.json 路径和最终文件路径
                    return parseMetadataAndCreateSong(infoJsonPath, finalDownloadedFilePath);
                } else {
                    // 如果 yt-dlp 失败，打印所有收集到的输出
                    throw new IOException("yt-dlp 下载失败，退出码: " + exitCode + "\n完整输出:\n" + fullOutput.toString());
                }

            } catch (IOException | InterruptedException e) {
                updateMessage("下载过程中发生错误: " + e.getMessage());
                throw new Exception("下载失败: " + e.getMessage(), e);
            } finally {
                if (process != null) {
                    process.destroy(); // 确保进程被终止
                }
            }
        }

        /**
         * 从指定的.info.json文件解析元数据，并使用传入的最终文件路径
         * @param infoJsonPath .info.json 文件的完整路径
         * @param finalAudioFilePath 最终下载的音频文件的完整路径
         */
        private Song parseMetadataAndCreateSong(String infoJsonPath, String finalAudioFilePath) throws IOException {
            if (infoJsonPath == null || infoJsonPath.isEmpty()) {
                throw new IOException("无法获取 info.json 文件的路径，无法解析元数据。");
            }

            File infoJsonFile = new File(infoJsonPath);

            if (!infoJsonFile.exists()) {
                throw new IOException("未找到元数据文件: " + infoJsonFile.getAbsolutePath());
            }

            JsonNode rootNode = objectMapper.readTree(infoJsonFile);

            // 直接使用从 yt-dlp 输出中捕获到的最终文件路径
            if (finalAudioFilePath == null || finalAudioFilePath.isEmpty()) {
                throw new IOException("无法从 yt-dlp 输出中获取最终下载文件的路径（EmbedThumbnail 行未找到或路径为空）。");
            }

            String id = rootNode.has("id") ? rootNode.get("id").asText() : "N/A";
            String title = rootNode.has("title") ? rootNode.get("title").asText() : "未知标题";
            String artist = rootNode.has("uploader") ? rootNode.get("uploader").asText() : "未知艺术家";
            // webpage_url 字段在 info.json 中总是完整的B站链接，无论你输入的是URL还是BV号
            String url = rootNode.has("webpage_url") ? rootNode.get("webpage_url").asText() : bilibiliIdentifier;
            String coverUrl = rootNode.has("thumbnail") ? rootNode.get("thumbnail").asText() : null;
            long duration = rootNode.has("duration") ? rootNode.get("duration").asLong() : 0;

            // 清理info.json文件
            infoJsonFile.delete();

            updateMessage("元数据解析成功。");
            return new Song(id, title, artist, url, finalAudioFilePath, coverUrl, duration, LocalDateTime.now());
        }
    }
}
