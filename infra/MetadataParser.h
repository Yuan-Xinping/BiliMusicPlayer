#pragma once
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include "../common/entities/Song.h"

class MetadataParser : public QObject {
    Q_OBJECT

public:
    struct ParseResult {
        bool success = false;
        Song song;
        QString errorMessage;
    };

    explicit MetadataParser(QObject* parent = nullptr);

    // 从 info.json 文件解析歌曲信息
    ParseResult parseFromInfoJson(const QString& jsonFilePath);

    // 从 JSON 对象解析歌曲信息  
    ParseResult parseFromJsonObject(const QJsonObject& jsonObj);

    // 清理文件名（移除非法字符）
    static QString sanitizeFilename(const QString& filename);

private:
    QString extractId(const QJsonObject& json);
    QString extractTitle(const QJsonObject& json);
    QString extractArtist(const QJsonObject& json);
    QString extractUrl(const QJsonObject& json);
    QString extractCoverUrl(const QJsonObject& json);
    qlonglong extractDuration(const QJsonObject& json);
};
