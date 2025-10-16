// ui/components/DownloadTaskItem.h
#pragma once

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../../service/DownloadService.h"

class DownloadTaskItem : public QWidget {
    Q_OBJECT

public:
    explicit DownloadTaskItem(const DownloadService::DownloadTask& task,
        QWidget* parent = nullptr);

    void setProgress(double progress);
    void setStatus(const QString& status);

signals:
    void pauseClicked();
    void cancelClicked();

private:
    void setupUI();

    QString m_identifier;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QPushButton* m_pauseBtn = nullptr;
    QPushButton* m_cancelBtn = nullptr;
};
