// ui/components/DownloadTaskItem.cpp
#include "DownloadTaskItem.h"

DownloadTaskItem::DownloadTaskItem(
    const DownloadService::DownloadTask& task,
    QWidget* parent)
    : QWidget(parent)
    , m_identifier(task.identifier)
{
    setupUI();
}

void DownloadTaskItem::setupUI()
{
    setFixedHeight(100);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(15);

    // 左侧：信息区域
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(8);

    m_titleLabel = new QLabel(m_identifier);
    m_titleLabel->setStyleSheet("color: #FFFFFF; font-size: 14px; font-weight: bold;");

    m_statusLabel = new QLabel("等待下载...");
    m_statusLabel->setStyleSheet("color: #AAAAAA; font-size: 12px;");

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(8);
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #444444;
            border-radius: 4px;
            background-color: #252525;
            text-align: center;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #FB7299, stop:1 #FF8BB5);
            border-radius: 3px;
        }
    )");

    infoLayout->addWidget(m_titleLabel);
    infoLayout->addWidget(m_progressBar);
    infoLayout->addWidget(m_statusLabel);

    // 右侧：按钮区域
    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(5);

    m_pauseBtn = new QPushButton("⏸️");
    m_pauseBtn->setFixedSize(40, 40);
    m_pauseBtn->setToolTip("暂停");

    m_cancelBtn = new QPushButton("❌");
    m_cancelBtn->setFixedSize(40, 40);
    m_cancelBtn->setToolTip("取消");

    buttonLayout->addWidget(m_pauseBtn);
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addStretch();

    mainLayout->addLayout(infoLayout, 1);
    mainLayout->addLayout(buttonLayout);

    // 整体样式
    setStyleSheet(R"(
        DownloadTaskItem {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,0.05),
                stop:1 rgba(255,255,255,0.02));
            border: 1px solid #333333;
            border-radius: 10px;
        }
        QPushButton {
            background-color: #333333;
            border: 1px solid #555555;
            border-radius: 8px;
            color: #CCCCCC;
        }
        QPushButton:hover {
            background-color: #444444;
            border-color: #FB7299;
        }
    )");

    connect(m_pauseBtn, &QPushButton::clicked, this, &DownloadTaskItem::pauseClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &DownloadTaskItem::cancelClicked);
}

void DownloadTaskItem::setProgress(double progress)
{
    m_progressBar->setValue(static_cast<int>(progress * 100));
}

void DownloadTaskItem::setStatus(const QString& status)
{
    m_statusLabel->setText(status);
}
