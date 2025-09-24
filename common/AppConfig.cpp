#include "AppConfig.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

AppConfig& AppConfig::instance() {
    static AppConfig instance;
    return instance;
}

AppConfig::AppConfig() {
    setDefaultValues();
}

void AppConfig::setDefaultValues() {
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    m_downloadPath = homeDir + "/BiliMusicPlayer_Downloads";

    // 优先使用内置二进制文件
    m_ytDlpPath = getBundledBinaryPath("yt-dlp.exe");
    m_ffmpegPath = getBundledBinaryPath("ffmpeg.exe");

    // 如果找不到内置文件，给出明确的警告
    if (m_ytDlpPath.isEmpty()) {
        qWarning() << "未找到内置的 yt-dlp.exe，请确保文件位于 external_binaries/win/ 目录";
    }
    if (m_ffmpegPath.isEmpty()) {
        qWarning() << "未找到内置的 ffmpeg.exe，请确保文件位于 external_binaries/win/ 目录";
    }
}

bool AppConfig::loadConfig() {
    QString configPath = getConfigFilePath();
    QFile configFile(configPath);

    if (!configFile.exists()) {
        qDebug() << "配置文件不存在，使用默认配置:" << configPath;
        return saveConfig();
    }

    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开配置文件:" << configPath;
        return false;
    }

    QByteArray data = configFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject()) {
        qWarning() << "配置文件 JSON 格式无效";
        return false;
    }

    QJsonObject obj = doc.object();

    // 加载配置，如果配置文件中的路径为空或不存在，则使用默认的内置路径
    QString configDownloadPath = obj.value("downloadPath").toString();
    QString configYtDlpPath = obj.value("ytDlpPath").toString();
    QString configFfmpegPath = obj.value("ffmpegPath").toString();

    m_downloadPath = configDownloadPath.isEmpty() ? m_downloadPath : configDownloadPath;

    // 对于二进制文件路径，优先使用内置版本
    if (configYtDlpPath.isEmpty() || !QFileInfo::exists(configYtDlpPath)) {
        m_ytDlpPath = getBundledBinaryPath("yt-dlp.exe");
    }
    else {
        m_ytDlpPath = configYtDlpPath;
    }

    if (configFfmpegPath.isEmpty() || !QFileInfo::exists(configFfmpegPath)) {
        m_ffmpegPath = getBundledBinaryPath("ffmpeg.exe");
    }
    else {
        m_ffmpegPath = configFfmpegPath;
    }

    qDebug() << "配置加载成功:" << configPath;
    return true;
}

bool AppConfig::saveConfig() {
    QString configPath = getConfigFilePath();
    QFileInfo fileInfo(configPath);
    QDir dir = fileInfo.dir();

    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "无法创建配置目录:" << dir.absolutePath();
            return false;
        }
    }

    QJsonObject obj;
    obj["downloadPath"] = m_downloadPath;
    obj["ytDlpPath"] = m_ytDlpPath;
    obj["ffmpegPath"] = m_ffmpegPath;

    QJsonDocument doc(obj);
    QFile configFile(configPath);

    if (!configFile.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入配置文件:" << configPath;
        return false;
    }

    configFile.write(doc.toJson());
    qDebug() << "配置已保存到:" << configPath;
    return true;
}

void AppConfig::setDownloadPath(const QString& path) {
    m_downloadPath = path;
    saveConfig();
}

void AppConfig::setYtDlpPath(const QString& path) {
    m_ytDlpPath = path;
    saveConfig();
}

void AppConfig::setFfmpegPath(const QString& path) {
    m_ffmpegPath = path;
    saveConfig();
}

QString AppConfig::getConfigFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return configDir + "/BiliMusicPlayer/config.json";
}

QString AppConfig::getBundledBinaryPath(const QString& binaryName) {
    QString appRoot = getApplicationRoot();

    QStringList possiblePaths = {
        appRoot + "/../../bin/" + binaryName,
        appRoot + "/../../../bin/" + binaryName,
        appRoot + "/bin/" + binaryName
    };

    qDebug() << "查找二进制文件:" << binaryName << "在路径:";

    for (const QString& path : possiblePaths) {
        QString normalizedPath = QDir::cleanPath(path);
        qDebug() << "  " << normalizedPath;

        if (QFileInfo::exists(normalizedPath)) {
            qDebug() << "✓ 找到:" << normalizedPath;
            return QDir::toNativeSeparators(normalizedPath);
        }
    }

    qWarning() << "✗ 未找到二进制文件:" << binaryName;
    return QString();
}


QString AppConfig::getApplicationRoot() {
    return QCoreApplication::applicationDirPath();
}
