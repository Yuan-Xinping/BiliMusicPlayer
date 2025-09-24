#include "MetadataParser.h"
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>

MetadataParser::MetadataParser(QObject* parent)
    : QObject(parent)
{
}

MetadataParser::ParseResult MetadataParser::parseFromInfoJson(const QString& jsonFilePath) {
    ParseResult result;

    QFile file(jsonFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.errorMessage = QString("无法打开文件: %1").arg(jsonFilePath);
        qWarning() << "MetadataParser:" << result.errorMessage;
        return result;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        result.errorMessage = QString("JSON 解析错误: %1").arg(error.errorString());
        qWarning() << "MetadataParser:" << result.errorMessage;
        return result;
    }

    if (!doc.isObject()) {
        result.errorMessage = "JSON 根节点不是对象";
        qWarning() << "MetadataParser:" << result.errorMessage;
        return result;
    }

    return parseFromJsonObject(doc.object());
}

MetadataParser::ParseResult MetadataParser::parseFromJsonObject(const QJsonObject& jsonObj) {
    ParseResult result;

    try {
        QString id = extractId(jsonObj);
        QString title = extractTitle(jsonObj);
        QString artist = extractArtist(jsonObj);
        QString url = extractUrl(jsonObj);
        QString coverUrl = extractCoverUrl(jsonObj);
        qlonglong duration = extractDuration(jsonObj);

        // 创建 Song 对象
        result.song = Song(
            id,
            title,
            artist,
            url,
            "", // localFilePath 稍后设置
            coverUrl,
            duration,
            QDateTime::currentDateTime(),
            false // isFavorite
        );

        result.success = true;

        qDebug() << "MetadataParser: 成功解析歌曲信息:";
        qDebug() << "  ID:" << id;
        qDebug() << "  标题:" << title;
        qDebug() << "  作者:" << artist;
        qDebug() << "  时长:" << duration << "秒";

    }
    catch (const std::exception& e) {
        result.errorMessage = QString("解析过程中发生异常: %1").arg(e.what());
        qWarning() << "MetadataParser:" << result.errorMessage;
    }

    return result;
}

QString MetadataParser::sanitizeFilename(const QString& filename) {
    QString sanitized = filename;

    // 修复正则表达式
    QRegularExpression invalidCharsRegex(R"([<>:"/\\|?*\[\]])");
    if (invalidCharsRegex.isValid()) {
        sanitized.replace(invalidCharsRegex, "_");
    }
    else {
        qWarning() << "MetadataParser: 无效的正则表达式模式";
        // 使用简单的字符替换作为后备方案
        sanitized.replace('<', '_').replace('>', '_').replace(':', '_')
            .replace('"', '_').replace('/', '_').replace('\\', '_')
            .replace('|', '_').replace('?', '_').replace('*', '_')
            .replace('[', '_').replace(']', '_');
    }

    // 移除控制字符
    QRegularExpression controlCharsRegex("[\x00-\x1f\x7f]");
    sanitized.replace(controlCharsRegex, "");

    // 移除开头和结尾的空格和点号
    sanitized = sanitized.trimmed();
    while (sanitized.endsWith('.')) {
        sanitized.chop(1);
    }

    // 确保不为空
    if (sanitized.isEmpty()) {
        sanitized = "unnamed_song";
    }

    // 限制长度（Windows 文件名限制）
    if (sanitized.length() > 200) {
        sanitized = sanitized.left(200);
    }

    return sanitized;
}

QString MetadataParser::extractId(const QJsonObject& json) {
    if (json.contains("id")) {
        return json["id"].toString();
    }
    if (json.contains("display_id")) {
        return json["display_id"].toString();
    }
    return "unknown_id";
}

QString MetadataParser::extractTitle(const QJsonObject& json) {
    if (json.contains("title")) {
        return json["title"].toString();
    }
    if (json.contains("fulltitle")) {
        return json["fulltitle"].toString();
    }
    return "未知标题";
}

QString MetadataParser::extractArtist(const QJsonObject& json) {
    // B站的UP主信息
    if (json.contains("uploader")) {
        return json["uploader"].toString();
    }
    if (json.contains("channel")) {
        return json["channel"].toString();
    }
    if (json.contains("creator")) {
        return json["creator"].toString();
    }
    return "未知艺术家";
}

QString MetadataParser::extractUrl(const QJsonObject& json) {
    if (json.contains("webpage_url")) {
        return json["webpage_url"].toString();
    }
    if (json.contains("original_url")) {
        return json["original_url"].toString();
    }
    return "";
}

QString MetadataParser::extractCoverUrl(const QJsonObject& json) {
    if (json.contains("thumbnail")) {
        return json["thumbnail"].toString();
    }
    // 尝试从 thumbnails 数组中获取最佳质量的缩略图
    if (json.contains("thumbnails") && json["thumbnails"].isArray()) {
        QJsonArray thumbnails = json["thumbnails"].toArray();
        if (!thumbnails.isEmpty()) {
            // 通常最后一个是最高质量的
            QJsonObject lastThumbnail = thumbnails.last().toObject();
            if (lastThumbnail.contains("url")) {
                return lastThumbnail["url"].toString();
            }
        }
    }
    return "";
}

qlonglong MetadataParser::extractDuration(const QJsonObject& json) {
    if (json.contains("duration")) {
        return json["duration"].toVariant().toLongLong();
    }
    return 0;
}
