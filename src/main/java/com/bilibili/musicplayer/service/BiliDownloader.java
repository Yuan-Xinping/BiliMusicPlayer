package com.bilibili.musicplayer.service;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.util.AppConfig; // 导入 AppConfig
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

    // 从 AppConfig 获取下载目录
    private static File getDownloadDir() {
        File dir = new File(AppConfig.getDownloadPath());
        if (!dir.exists()) {
            dir.mkdirs();
        }
        return dir;
    }

    private static final ObjectMapper objectMapper = new ObjectMapper();

    // 正则表达式用于解析yt-dlp的进度输出
    private static final Pattern PROGRESS_PATTERN = Pattern.compile("\\[download\\]\\s*(\\d+\\.\\d+)%");
    // 正则表达式用于捕获 .info.json 文件的路径
    private static final Pattern INFO_JSON_PATH_PATTERN = Pattern.compile("Writing video metadata as JSON to: (.*)");
    // 新增：正则表达式用于捕获 [EmbedThumbnail] 行中的最终文件路径
    private static final Pattern EMBED_THUMBNAIL_PATH_PATTERN = Pattern.compile("\\[EmbedThumbnail\\] ffmpeg: Adding thumbnail to \"(.*?)\"");


    /**
     * 用于异步下载的 Task 类
     */
    public static class DownloadTask extends Task<Song> {
        private final String bilibiliIdentifier;

        // 用于存储捕获到的 info.json 文件的路径
        private String infoJsonPath;
        // 用于存储最终下载的 MP3 文件的路径
        private String finalDownloadedFilePath;

        public DownloadTask(String bilibiliIdentifier) {
            this.bilibiliIdentifier = bilibiliIdentifier;
            updateMessage("准备下载...");
            updateProgress(0, 1);
        }

        @Override
        protected Song call() throws Exception {
            updateMessage("开始下载: " + bilibiliIdentifier);

            List<String> command = new ArrayList<>();
            command.add(AppConfig.getYtDlpPath()); // 使用 AppConfig 获取 yt-dlp 路径
            command.add("--ffmpeg-location"); // 告诉 yt-dlp ffmpeg 的位置
            command.add(AppConfig.getFfmpegPath()); // 使用 AppConfig 获取 ffmpeg 路径
            command.add("-x"); // 提取音频

            // 移除音频格式选择，硬编码为 mp3
            command.add("--audio-format");
            command.add("mp3"); // 默认音频格式为 mp3

            // 移除音质选择，硬编码为最佳音质 (0)
            command.add("--audio-quality");
            command.add("0"); // 默认最佳音质

            // 移除嵌入封面选择，硬编码为始终嵌入
            command.add("--embed-thumbnail"); // 始终嵌入封面

            command.add("--write-info-json"); // 写入元数据json文件
            command.add("--output"); // 输出路径模板
            // %(title)s - 视频标题, %(ext)s - 扩展名, %(id)s - 视频ID
            command.add(getDownloadDir().getAbsolutePath() + File.separator + "%(title)s.%(ext)s");

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

                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream(),"GBK")); // 建议根据系统编码调整，或使用 Charset.defaultCharset()
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
                // 如果 EmbedThumbnail 路径未捕获到，尝试从 info.json 猜测
                // 这是一种回退方案，但强烈建议依赖 EmbedThumbnail 的输出
                String guessedFilePath = getDownloadDir().getAbsolutePath() + File.separator +
                        (rootNode.has("title") ? rootNode.get("title").asText() : "unknown") +
                        "." + (rootNode.has("ext") ? rootNode.get("ext").asText() : "mp3");
                File guessedFile = new File(guessedFilePath);
                if (guessedFile.exists()) {
                    finalAudioFilePath = guessedFile.getAbsolutePath();
                    System.out.println("Warning: finalDownloadedFilePath not captured from EmbedThumbnail, guessed: " + finalAudioFilePath);
                } else {
                    throw new IOException("无法从 yt-dlp 输出中获取最终下载文件的路径，且无法猜测其位置。");
                }
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
            return new Song(id, title, artist, url, finalAudioFilePath, coverUrl, duration, LocalDateTime.now(),false);
        }
    }
}
