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
        qWarning() << "未找到内置的 yt-dlp.exe，请确保完整运行了 CMake 构建过程";
    }
    if (m_ffmpegPath.isEmpty()) {
        qWarning() << "未找到内置的 ffmpeg.exe，请确保完整运行了 CMake 构建过程";
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
    qDebug() << "yt-dlp 路径:" << m_ytDlpPath;
    qDebug() << "ffmpeg 路径:" << m_ffmpegPath;
    qDebug() << "下载路径:" << m_downloadPath;

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

bool AppConfig::isYtDlpAvailable() const {
    if (m_ytDlpPath.isEmpty()) return false;
    QFileInfo file(m_ytDlpPath);
    return file.exists() && file.isExecutable();
}

bool AppConfig::isFfmpegAvailable() const {
    if (m_ffmpegPath.isEmpty()) return false;
    QFileInfo file(m_ffmpegPath);
    return file.exists() && file.isExecutable();
}

void AppConfig::refreshBinaryPaths() {
    qDebug() << "重新扫描二进制文件...";
    QString oldYtDlpPath = m_ytDlpPath;
    QString oldFfmpegPath = m_ffmpegPath;

    m_ytDlpPath = getBundledBinaryPath("yt-dlp.exe");
    m_ffmpegPath = getBundledBinaryPath("ffmpeg.exe");

    if (oldYtDlpPath != m_ytDlpPath || oldFfmpegPath != m_ffmpegPath) {
        qDebug() << "二进制文件路径已更新";
        qDebug() << "yt-dlp:" << oldYtDlpPath << "->" << m_ytDlpPath;
        qDebug() << "ffmpeg:" << oldFfmpegPath << "->" << m_ffmpegPath;
        saveConfig(); // 保存更新的路径
    }
    else {
        qDebug() << "二进制文件路径无变化";
    }
}

QString AppConfig::getConfigFilePath() const {
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return configDir + "/BiliMusicPlayer/config.json";
}

QString AppConfig::getBundledBinaryPath(const QString& binaryName) {
    QString appRoot = getApplicationRoot();

    QStringList possiblePaths = {
        appRoot + "/../bin/" + binaryName,                    

        appRoot + "/bin/" + binaryName,                       
        appRoot + "/../../bin/" + binaryName,                
        appRoot + "/../../../bin/" + binaryName,           

        appRoot + "/../../../../build/bin/" + binaryName,     

        appRoot + "/../../../../external_binaries/win/" + binaryName,
        appRoot + "/../../../../../external_binaries/win/" + binaryName
    };

    qDebug() << "当前应用程序路径:" << appRoot;
    qDebug() << "查找二进制文件:" << binaryName;

    for (const QString& path : possiblePaths) {
        QString normalizedPath = QDir::cleanPath(path);
        qDebug() << "  检查:" << normalizedPath;

        QFileInfo fileInfo(normalizedPath);
        if (fileInfo.exists() && fileInfo.isFile()) {
            // 验证文件大小（确保不是空文件）
            if (fileInfo.size() > 0) {
                qDebug() << "✓ 找到:" << normalizedPath << "(" << fileInfo.size() << " bytes)";
                return QDir::toNativeSeparators(normalizedPath);
            }
            else {
                qWarning() << "  文件存在但为空:" << normalizedPath;
            }
        }
    }

    qWarning() << "✗ 未找到二进制文件:" << binaryName;
    qWarning() << "请确保：";
    qWarning() << "1. 运行了完整的 CMake 构建过程";
    qWarning() << "2. external_binaries/win/ 目录中存在" << binaryName;
    qWarning() << "3. CMakeLists.txt 中的复制规则正确执行";

    return QString();
}


QString AppConfig::getApplicationRoot() {
    return QCoreApplication::applicationDirPath();
}
