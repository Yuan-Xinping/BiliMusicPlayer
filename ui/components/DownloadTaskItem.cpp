// ui/components/DownloadTaskItem.cpp
#include "DownloadTaskItem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

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
    setObjectName("downloadTaskItem");

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    mainLayout->setSpacing(15);

    // 左侧：信息区域
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(8);

    m_titleLabel = new QLabel(m_identifier);
    m_titleLabel->setObjectName("taskTitleLabel"); 
    m_statusLabel = new QLabel("等待下载...");
    m_statusLabel->setObjectName("taskStatusLabel"); 
    m_progressBar = new QProgressBar();
    m_progressBar->setObjectName("taskProgressBar"); 
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(8);

    infoLayout->addWidget(m_titleLabel);
    infoLayout->addWidget(m_progressBar);
    infoLayout->addWidget(m_statusLabel);

    // 右侧：按钮区域
    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(5);

    m_pauseBtn = new QPushButton("⏸️");
    m_pauseBtn->setObjectName("taskControlBtn");  
    m_pauseBtn->setFixedSize(40, 40);
    m_pauseBtn->setToolTip("暂停");

    m_cancelBtn = new QPushButton("❌");
    m_cancelBtn->setObjectName("taskControlBtn");  
    m_cancelBtn->setFixedSize(40, 40);
    m_cancelBtn->setToolTip("取消");

    buttonLayout->addWidget(m_pauseBtn);
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addStretch();

    mainLayout->addLayout(infoLayout, 1);
    mainLayout->addLayout(buttonLayout);


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
