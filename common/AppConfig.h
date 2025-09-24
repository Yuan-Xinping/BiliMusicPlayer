#pragma once
#include <QString>
#include <QFile>

class AppConfig {
public:
    static AppConfig& instance();

    bool loadConfig();
    bool saveConfig();

    QString getDownloadPath() const { return m_downloadPath; }
    QString getYtDlpPath() const { return m_ytDlpPath; }
    QString getFfmpegPath() const { return m_ffmpegPath; }

    void setDownloadPath(const QString& path);
    void setYtDlpPath(const QString& path);
    void setFfmpegPath(const QString& path);

    bool isYtDlpAvailable() const;
    bool isFfmpegAvailable() const;

    void refreshBinaryPaths();

    static QString getBundledBinaryPath(const QString& binaryName);

private:
    AppConfig();
    ~AppConfig() = default;
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    void setDefaultValues();
    QString getConfigFilePath() const;
    static QString getApplicationRoot();

    QString m_downloadPath;
    QString m_ytDlpPath;
    QString m_ffmpegPath;
};
