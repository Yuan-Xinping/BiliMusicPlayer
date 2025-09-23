#pragma once
#include <QObject>

class FileSystemHelper : public QObject {
    Q_OBJECT
public:
    explicit FileSystemHelper(QObject* parent = nullptr);
    static FileSystemHelper& instance();
};
