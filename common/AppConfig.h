#pragma once

#include <QString>
#include "entities/Song.h"
#include "../infra/DownloadConfig.h"

class AppConfig {
public:
    static AppConfig& instance();

    bool load();
    bool save();

    // Getters
    QString getDownloadPath() const;
    QString getYtDlpPath() const;
    QString getFfmpegPath() const;
    QString getDefaultQualityPreset() const;
    AudioFormat getDefaultAudioFormat() const;
    int getMaxConcurrentDownloads() const;
    QString getDatabasePath() const;
    QString getTheme() const;
    int getFontSize() const;
    bool getProxyEnabled() const;
    QString getProxyUrl() const;

    // Setters
    void setDownloadPath(const QString& path);
    void setYtDlpPath(const QString& path);
    void setFfmpegPath(const QString& path);
    void setDefaultQualityPreset(const QString& preset);
    void setDefaultAudioFormat(AudioFormat format);
    void setMaxConcurrentDownloads(int count);
    void setDatabasePath(const QString& path);
    void setTheme(const QString& theme);
    void setFontSize(int size);
    void setProxyEnabled(bool enabled);
    void setProxyUrl(const QString& url);

    QString getConfigFilePath() const;

private:
    AppConfig();
    ~AppConfig() = default;
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    void setDefaultValues();
    void loadFromJson(const class QJsonObject& json);
    void saveToJson(class QJsonObject& json) const;
    QString getBundledBinaryPath(const QString& binaryName) const;
    void ensureDatabaseDirectoryExists();

    // 下载设置
    QString m_downloadPath;
    QString m_ytDlpPath;
    QString m_ffmpegPath;
    QString m_defaultQualityPreset;
    AudioFormat m_defaultAudioFormat = AudioFormat::MP3;
    int m_maxConcurrentDownloads = 3;

    // 数据库设置
    QString m_databasePath;

    // 界面设置
    QString m_theme;
    int m_fontSize = 13;

    // 高级设置
    bool m_proxyEnabled = false;
    QString m_proxyUrl;
};
