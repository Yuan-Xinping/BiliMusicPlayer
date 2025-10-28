#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include "entities/Song.h"
#include "../infra/DownloadConfig.h"

class AppConfig : public QObject {
    Q_OBJECT

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

    // 播放相关（持久化）
    int  getPlayerVolume() const;        // 0-100
    int  getPlayerPlaybackMode() const;  // 0 顺序、1 随机、2 单曲、3 列表
    void setPlayerVolume(int volume);
    void setPlayerPlaybackMode(int mode);

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
    // 会话恢复
    bool getResumeOnStartup() const;
    void setResumeOnStartup(bool enabled);
    QString getLastSongId() const;
    void setLastSongId(const QString& id);
    qint64 getLastPositionMs() const;
    void setLastPositionMs(qint64 ms);
    QStringList getLastPlaylistIds() const;
    void setLastPlaylistIds(const QStringList& ids);
    QStringList getLastQueueIds() const;
    void setLastQueueIds(const QStringList& ids);

    bool isValidTheme(const QString& theme) const;
    static QStringList availableThemes();

    QString getConfigFilePath() const;

signals:
    void themeChanged(const QString& theme);

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

    static QStringList s_validThemes;
    static AudioFormat formatForPreset(const QString& preset); // 映射方法

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

    // 播放持久化
    int m_playerVolume = 70;         // 0-100
    int m_playerPlaybackMode = 0;    // 0 顺序
    bool m_resumeOnStartup = true;
    QString m_lastSongId;
    qint64 m_lastPositionMs = 0;
    QStringList m_lastPlaylistIds;
    QStringList m_lastQueueIds;
    
};
