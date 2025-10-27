// AppConfig.cpp
#include "AppConfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

QStringList AppConfig::s_validThemes = { "dark", "light" };

static inline bool presetEq(const QString& a, const char* b) {
    return a.compare(QLatin1String(b), Qt::CaseInsensitive) == 0;
}

AudioFormat AppConfig::formatForPreset(const QString& preset) {
    if (presetEq(preset, "lossless_wav"))       return AudioFormat::WAV;
    if (presetEq(preset, "lossless_flac"))      return AudioFormat::FLAC;
    if (presetEq(preset, "small_size_opus"))    return AudioFormat::OPUS;
    if (presetEq(preset, "high_quality_mp3"))   return AudioFormat::MP3;
    if (presetEq(preset, "medium_quality_mp3")) return AudioFormat::MP3;
    if (presetEq(preset, "best_quality"))       return AudioFormat::FLAC;
    return AudioFormat::MP3; // 兜底
}

AppConfig& AppConfig::instance() {
    static AppConfig instance;
    return instance;
}

AppConfig::AppConfig() : QObject(nullptr) {
    setDefaultValues();
}

bool AppConfig::load() {
    QString configPath = getConfigFilePath();
    QFile file(configPath);

    if (!file.exists()) {
        qDebug() << "配置文件不存在，使用默认值:" << configPath;
        setDefaultValues();
        return save();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << configPath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "配置文件格式错误";
        return false;
    }

    QJsonObject json = doc.object();
    loadFromJson(json);

    qDebug() << "配置加载成功:" << configPath;

    if (m_ytDlpPath.isEmpty() || !QFileInfo::exists(m_ytDlpPath)) {
        qWarning() << "⚠️ yt-dlp 路径无效，重新检测...";
        m_ytDlpPath = getBundledBinaryPath("yt-dlp.exe");
        if (m_ytDlpPath.isEmpty()) {
            m_ytDlpPath = QStandardPaths::findExecutable("yt-dlp");
        }
        save();
    }

    if (m_ffmpegPath.isEmpty() || !QFileInfo::exists(m_ffmpegPath)) {
        qWarning() << "⚠️ ffmpeg 路径无效，重新检测...";
        m_ffmpegPath = getBundledBinaryPath("ffmpeg.exe");
        if (m_ffmpegPath.isEmpty()) {
            m_ffmpegPath = QStandardPaths::findExecutable("ffmpeg");
        }
        save();
    }

    qDebug() << "✅ 最终 yt-dlp 路径:" << m_ytDlpPath;
    qDebug() << "✅ 最终 ffmpeg 路径:" << m_ffmpegPath;

    return true;
}

void AppConfig::ensureDatabaseDirectoryExists() {
    QFileInfo dbFileInfo(m_databasePath);
    QDir dbDir = dbFileInfo.dir();

    if (!dbDir.exists()) {
        qDebug() << "📁 数据库目录不存在，正在创建:" << dbDir.absolutePath();

        if (dbDir.mkpath(".")) {
            qDebug() << "✅ 数据库目录创建成功:" << dbDir.absolutePath();
        }
        else {
            qCritical() << "❌ 数据库目录创建失败:" << dbDir.absolutePath();
        }
    }
    else {
        qDebug() << "📁 数据库目录已存在:" << dbDir.absolutePath();
    }
}

bool AppConfig::save() {
    QString configPath = getConfigFilePath();
    QFileInfo fileInfo(configPath);
    QDir dir = fileInfo.dir();

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "无法创建配置目录:" << dir.absolutePath();
            return false;
        }
    }

    QJsonObject json;
    saveToJson(json);

    QJsonDocument doc(json);
    QFile file(configPath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入配置文件:" << configPath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "配置保存成功:" << configPath;
    return true;
}

void AppConfig::setDefaultValues() {
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    // 下载路径
    m_downloadPath = homeDir + "/BiliMusicPlayer_Downloads";

    // 工具路径
    m_ytDlpPath = getBundledBinaryPath("yt-dlp.exe");
    m_ffmpegPath = getBundledBinaryPath("ffmpeg.exe");

    if (m_ytDlpPath.isEmpty()) {
        qWarning() << "⚠️ 未找到内置的 yt-dlp.exe";
    }
    else {
        qDebug() << "✅ yt-dlp 路径:" << m_ytDlpPath;
    }

    if (m_ffmpegPath.isEmpty()) {
        qWarning() << "⚠️ 未找到内置的 ffmpeg.exe";
    }
    else {
        qDebug() << "✅ ffmpeg 路径:" << m_ffmpegPath;
    }

    // 下载设置
    m_defaultQualityPreset = "high_quality_mp3";
    m_defaultAudioFormat = formatForPreset(m_defaultQualityPreset); // 由预设推导
    m_maxConcurrentDownloads = 3;

    // 数据库路径
    m_databasePath = homeDir + "/BiliMusicPlayer/bili_music_player.db";
    qDebug() << "📊 数据库路径设置为:" << m_databasePath;

    ensureDatabaseDirectoryExists();

    // 界面设置
    m_theme = "dark";
    m_fontSize = 13;

    // 高级设置
    m_proxyEnabled = false;
    m_proxyUrl = "";
}

QString AppConfig::getConfigFilePath() const {
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return homeDir + "/BiliMusicPlayer/config.json";
}

QString AppConfig::getBundledBinaryPath(const QString& binaryName) const {
    QString appDir = QCoreApplication::applicationDirPath();

    QStringList candidatePaths = {
        appDir + "/bin/" + binaryName,
        appDir + "/../bin/" + binaryName,
        appDir + "/../../bin/" + binaryName,
        appDir + "/" + binaryName
    };

    for (const QString& path : candidatePaths) {
        QString cleanPath = QDir::cleanPath(path);
        if (QFileInfo::exists(cleanPath)) {
            qDebug() << "✅ 找到二进制文件:" << cleanPath;
            return cleanPath;
        }
    }

    qWarning() << "❌ 未找到二进制文件:" << binaryName;
    return QString();
}

void AppConfig::loadFromJson(const QJsonObject& json) {
    if (json.contains("downloadPath")) {
        m_downloadPath = json["downloadPath"].toString();
    }

    if (json.contains("ytDlpPath")) {
        m_ytDlpPath = json["ytDlpPath"].toString();
    }

    if (json.contains("ffmpegPath")) {
        m_ffmpegPath = json["ffmpegPath"].toString();
    }

    if (json.contains("defaultQualityPreset")) {
        m_defaultQualityPreset = json["defaultQualityPreset"].toString();
    }

    // —— 关键：用预设推导应有的格式，并在存在不一致时修正 —— //
    const AudioFormat expected = formatForPreset(m_defaultQualityPreset);
    if (json.contains("defaultAudioFormat")) {
        m_defaultAudioFormat = static_cast<AudioFormat>(json["defaultAudioFormat"].toInt());
        if (m_defaultAudioFormat != expected) {
            qWarning() << "AppConfig: format mismatch with preset, fixing. preset="
                << m_defaultQualityPreset
                << "fileFormat=" << static_cast<int>(m_defaultAudioFormat)
                << "expected=" << static_cast<int>(expected);
            m_defaultAudioFormat = expected;
        }
    }
    else {
        m_defaultAudioFormat = expected;
    }

    if (json.contains("maxConcurrentDownloads")) {
        m_maxConcurrentDownloads = json["maxConcurrentDownloads"].toInt();
    }

    if (json.contains("databasePath")) {
        m_databasePath = json["databasePath"].toString();
    }

    if (json.contains("theme")) {
        QString loadedTheme = json["theme"].toString();
        if (isValidTheme(loadedTheme)) {
            m_theme = loadedTheme;
        }
        else {
            qWarning() << "⚠️ 配置文件中的主题无效：" << loadedTheme;
            m_theme = "dark";
        }
    }

    if (json.contains("fontSize")) {
        m_fontSize = json["fontSize"].toInt();
    }

    if (json.contains("proxyEnabled")) {
        m_proxyEnabled = json["proxyEnabled"].toBool();
    }

    if (json.contains("proxyUrl")) {
        m_proxyUrl = json["proxyUrl"].toString();
    }
}

void AppConfig::saveToJson(QJsonObject& json) const {
    json["downloadPath"] = m_downloadPath;
    json["ytDlpPath"] = m_ytDlpPath;
    json["ffmpegPath"] = m_ffmpegPath;
    json["defaultQualityPreset"] = m_defaultQualityPreset;
    json["defaultAudioFormat"] = static_cast<int>(m_defaultAudioFormat);
    json["maxConcurrentDownloads"] = m_maxConcurrentDownloads;
    json["databasePath"] = m_databasePath;
    json["theme"] = m_theme;
    json["fontSize"] = m_fontSize;
    json["proxyEnabled"] = m_proxyEnabled;
    json["proxyUrl"] = m_proxyUrl;
}

bool AppConfig::isValidTheme(const QString& theme) const {
    return s_validThemes.contains(theme);
}

QStringList AppConfig::availableThemes() {
    return s_validThemes;
}

// Getters
QString AppConfig::getDownloadPath() const { return m_downloadPath; }
QString AppConfig::getYtDlpPath() const { return m_ytDlpPath; }
QString AppConfig::getFfmpegPath() const { return m_ffmpegPath; }
QString AppConfig::getDefaultQualityPreset() const { return m_defaultQualityPreset; }
AudioFormat AppConfig::getDefaultAudioFormat() const { return m_defaultAudioFormat; }
int AppConfig::getMaxConcurrentDownloads() const { return m_maxConcurrentDownloads; }
QString AppConfig::getDatabasePath() const { return m_databasePath; }
QString AppConfig::getTheme() const { return m_theme; }
int AppConfig::getFontSize() const { return m_fontSize; }
bool AppConfig::getProxyEnabled() const { return m_proxyEnabled; }
QString AppConfig::getProxyUrl() const { return m_proxyUrl; }

// Setters
void AppConfig::setDownloadPath(const QString& path) { m_downloadPath = path; }
void AppConfig::setYtDlpPath(const QString& path) { m_ytDlpPath = path; }
void AppConfig::setFfmpegPath(const QString& path) { m_ffmpegPath = path; }
void AppConfig::setDefaultQualityPreset(const QString& preset) {
    if (m_defaultQualityPreset == preset) return;
    m_defaultQualityPreset = preset;
    m_defaultAudioFormat = formatForPreset(preset); 
    qDebug() << "AppConfig: preset ->" << preset
        << ", format ->" << static_cast<int>(m_defaultAudioFormat);
}
void AppConfig::setDefaultAudioFormat(AudioFormat format) { m_defaultAudioFormat = format; }
void AppConfig::setMaxConcurrentDownloads(int count) { m_maxConcurrentDownloads = count; }
void AppConfig::setDatabasePath(const QString& path) { m_databasePath = path; }
void AppConfig::setFontSize(int size) { m_fontSize = size; }
void AppConfig::setProxyEnabled(bool enabled) { m_proxyEnabled = enabled; }
void AppConfig::setProxyUrl(const QString& url) { m_proxyUrl = url; }
void AppConfig::setTheme(const QString& theme) {
    if (m_theme == theme) return;

    if (isValidTheme(theme)) {
        m_theme = theme;
        qDebug() << "✅ 主题已设置为：" << theme;
        emit themeChanged(theme);
    }
    else {
        qWarning() << "❌ 无效的主题名称：" << theme;
    }
}
