#include "DownloadConfig.h"
#include <QDebug>

QStringList DownloadOptions::toYtDlpArgs() const {
    QStringList args;

    // 基本选项
    if (extractAudioOnly) {
        args << "-x"; // 提取音频

        // 音频格式
        switch (audioFormat) {
        case AudioFormat::MP3:
            args << "--audio-format" << "mp3";
            break;
        case AudioFormat::M4A:
            args << "--audio-format" << "m4a";
            break;
        case AudioFormat::OPUS:
            args << "--audio-format" << "opus";
            break;
        case AudioFormat::FLAC:
            args << "--audio-format" << "flac";
            break;
        case AudioFormat::WAV:
            args << "--audio-format" << "wav";
            break;
        }

        // 音频质量
        args << "--audio-quality" << QString::number(static_cast<int>(audioQuality));
    }
    else {
        // 视频下载选项
        QString qualityStr;
        switch (videoQuality) {
        case VideoQuality::Best:
            qualityStr = "best";
            break;
        case VideoQuality::Worst:
            qualityStr = "worst";
            break;
        case VideoQuality::P1080:
            qualityStr = "best[height<=1080]";
            break;
        case VideoQuality::P720:
            qualityStr = "best[height<=720]";
            break;
        case VideoQuality::P480:
            qualityStr = "best[height<=480]";
            break;
        case VideoQuality::P360:
            qualityStr = "best[height<=360]";
            break;
        }
        args << "--format" << qualityStr;
    }

    // 元数据选项
    if (embedThumbnail) {
        args << "--embed-thumbnail";
    }

    if (writeInfoJson) {
        args << "--write-info-json";
    }

    if (writeDescription) {
        args << "--write-description";
    }

    if (writeSubtitles) {
        args << "--write-subs";
    }

    if (writeAutoSubtitles) {
        args << "--write-auto-subs";
    }

    // 文件名选项
    if (restrictFilenames) {
        args << "--restrict-filenames";
    }

    // 网络选项
    if (maxRetries > 0) {
        args << "--retries" << QString::number(maxRetries);
    }

    if (fragmentRetries > 0) {
        args << "--fragment-retries" << QString::number(fragmentRetries);
    }

    if (!proxy.isEmpty()) {
        args << "--proxy" << proxy;
    }

    if (!rateLimitKbps.isEmpty()) {
        args << "--limit-rate" << rateLimitKbps;
    }

    // SSL选项
    if (ignoreCertificateErrors) {
        args << "--ignore-certificate-errors";
    }

    if (noCheckCertificate) {
        args << "--no-check-certificate";
    }

    // 自定义参数
    args.append(customArgs);

    return args;
}

QString DownloadOptions::getFileExtension() const {
    if (extractAudioOnly) {
        switch (audioFormat) {
        case AudioFormat::MP3: return "mp3";
        case AudioFormat::M4A: return "m4a";
        case AudioFormat::OPUS: return "opus";
        case AudioFormat::FLAC: return "flac";
        case AudioFormat::WAV: return "wav";
        }
    }
    return videoFormat; // 视频格式
}

DownloadOptions DownloadOptions::createPreset(const QString& presetName) {
    DownloadOptions options;

    if (presetName == "high_quality_mp3") {
        options.audioQuality = AudioQuality::Best;
        options.audioFormat = AudioFormat::MP3;
        options.embedThumbnail = true;
        options.writeInfoJson = true;
    }
    else if (presetName == "medium_quality_mp3") {
        options.audioQuality = AudioQuality::High;
        options.audioFormat = AudioFormat::MP3;
        options.embedThumbnail = true;
        options.writeInfoJson = true;
    }
    else if (presetName == "lossless_flac") {
        options.audioQuality = AudioQuality::Best;
        options.audioFormat = AudioFormat::FLAC;
        options.embedThumbnail = true;
        options.writeInfoJson = true;
    }
    else if (presetName == "small_size_opus") {
        options.audioQuality = AudioQuality::Medium;
        options.audioFormat = AudioFormat::OPUS;
        options.embedThumbnail = true;
        options.writeInfoJson = true;
    }
    else if (presetName == "lossless_wav") {
        options.audioQuality = AudioQuality::Best;
        options.audioFormat = AudioFormat::WAV;
        options.embedThumbnail = false;
        options.writeInfoJson = true;
    }
    else if (presetName == "best_quality") {
        options.audioQuality = AudioQuality::Best;
        options.audioFormat = AudioFormat::FLAC;
        options.embedThumbnail = true;
        options.writeInfoJson = true;
    }
    else if (presetName == "video_720p") {
        options.extractAudioOnly = false;
        options.videoQuality = VideoQuality::P720;
        options.videoFormat = "mp4";
        options.embedThumbnail = false;
        options.writeInfoJson = true;
    }
    else if (presetName == "video_1080p") {
        options.extractAudioOnly = false;
        options.videoQuality = VideoQuality::P1080;
        options.videoFormat = "mp4";
        options.embedThumbnail = false;
        options.writeInfoJson = true;
    }

    return options;
}

QStringList DownloadOptions::getAvailablePresets() {
    return {
        "high_quality_mp3",
        "medium_quality_mp3",
        "lossless_flac",
        "small_size_opus",
        "lossless_wav",
        "best_quality",
        "video_720p",
        "video_1080p"
    };
}
