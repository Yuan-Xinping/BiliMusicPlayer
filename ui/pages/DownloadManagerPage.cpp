// ui/pages/DownloadManagerPage.cpp
#include "DownloadManagerPage.h"
#include "../components/DownloadTaskItem.h"
#include "../../common/AppConfig.h"

#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QDateTime>
#include <QHeaderView>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTabWidget>
#include <QListWidget>
#include <QTableWidget>

DownloadManagerPage::DownloadManagerPage(QWidget* parent)
    : QWidget(parent)
    , m_downloadService(new DownloadService(this))
{
    setupUI();
    setupConnections();
    setupStyles();

    QString downloadPath = AppConfig::instance().getDownloadPath();
    qDebug() << "====================================";
    qDebug() << "📁 当前下载路径：" << downloadPath;
    qDebug() << "====================================";

    // 在状态栏显示下载路径
    m_statusLabel->setText(QString("💤 等待任务 | 📁 %1").arg(downloadPath));
}

void DownloadManagerPage::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // ========== 输入区域 ==========
    QGroupBox* inputGroup = new QGroupBox("📥 新建下载");
    inputGroup->setObjectName("downloadInputGroup");

    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    inputLayout->setSpacing(12);

    // URL 输入行
    QHBoxLayout* urlLayout = new QHBoxLayout();
    QLabel* urlLabel = new QLabel("BV号/URL:");
    urlLabel->setFixedWidth(80);
    urlLabel->setObjectName("inputLabel");
    urlLabel->setStyleSheet("color: #CCCCCC; font-size: 13px;"); 

    m_urlInput = new QLineEdit();
    m_urlInput->setPlaceholderText("输入 BV 号或完整 URL，例如：BV1xx411c7mD");
    m_urlInput->setObjectName("urlInput");

    urlLayout->addWidget(urlLabel);
    urlLayout->addWidget(m_urlInput);

    // 音质选择行
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    QLabel* qualityLabel = new QLabel("音质选择:");
    qualityLabel->setFixedWidth(80);
    qualityLabel->setObjectName("inputLabel"); 
    qualityLabel->setStyleSheet("color: #CCCCCC; font-size: 13px;");

    m_qualityCombo = new QComboBox();
    m_qualityCombo->addItem("🎵 高品质 MP3 (320kbps)", "high_quality_mp3");
    m_qualityCombo->addItem("🎶 标准 MP3 (192kbps)", "standard_mp3");
    m_qualityCombo->addItem("🎼 最佳音质 (原始格式)", "best_quality");
    m_qualityCombo->setObjectName("qualityCombo");

    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(m_qualityCombo);
    qualityLayout->addStretch();

    // 按钮行
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("🚀 开始下载");
    m_startBtn->setObjectName("startDownloadBtn");
    m_startBtn->setFixedHeight(40);

    m_batchDownloadBtn = new QPushButton("📋 批量下载");
    m_batchDownloadBtn->setObjectName("batchDownloadBtn");
    m_batchDownloadBtn->setFixedHeight(40);

    buttonLayout->addWidget(m_startBtn);
    buttonLayout->addWidget(m_batchDownloadBtn);
    buttonLayout->addStretch();

    inputLayout->addLayout(urlLayout);
    inputLayout->addLayout(qualityLayout);
    inputLayout->addLayout(buttonLayout);

    // ========== 任务显示区域 ==========
    m_tabWidget = new QTabWidget();
    m_tabWidget->setObjectName("downloadTabWidget");

    // 下载队列 Tab
    m_queueList = new QListWidget();
    m_queueList->setObjectName("downloadQueueList");
    m_queueList->setSpacing(8);
    m_queueList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_tabWidget->addTab(m_queueList, "⏳ 下载队列");

    // 下载历史 Tab
    m_historyTable = new QTableWidget();
    m_historyTable->setObjectName("downloadHistoryTable");
    m_historyTable->setColumnCount(5);
    m_historyTable->setHorizontalHeaderLabels({
        "状态", "歌曲标题", "艺术家", "完成时间", "备注"
        });
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tabWidget->addTab(m_historyTable, "📋 下载历史");

    QHBoxLayout* statusLayout = new QHBoxLayout();

    m_statusLabel = new QLabel("💤 等待下载任务...");
    m_statusLabel->setObjectName("downloadStatusLabel");

    QPushButton* openFolderBtn = new QPushButton("📁 打开下载文件夹");
    openFolderBtn->setObjectName("openFolderBtn");
    openFolderBtn->setFixedHeight(35);
    connect(openFolderBtn, &QPushButton::clicked, [this]() {
        QString downloadPath = AppConfig::instance().getDownloadPath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(downloadPath));
        });

    QPushButton* clearHistoryBtn = new QPushButton("🗑️ 清空历史");
    clearHistoryBtn->setObjectName("controlBtn");
    clearHistoryBtn->setFixedHeight(35);
    connect(clearHistoryBtn, &QPushButton::clicked, [this]() {
        auto reply = QMessageBox::question(
            this, "确认清空", "确定要清空所有下载历史记录吗？",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::Yes) {
            m_historyTable->setRowCount(0);
            m_statusLabel->setText("🗑️ 历史记录已清空");
        }
        });

    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(openFolderBtn);
    statusLayout->addWidget(clearHistoryBtn);

    // ========== 添加到主布局 ==========
    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(m_tabWidget, 1);
    mainLayout->addLayout(statusLayout);
}

void DownloadManagerPage::setupConnections()
{
    // UI 按钮信号
    connect(m_startBtn, &QPushButton::clicked,
        this, &DownloadManagerPage::onStartDownloadClicked);

    // DownloadService 信号
    connect(m_downloadService, &DownloadService::taskAdded,
        this, &DownloadManagerPage::onTaskAdded);
    connect(m_downloadService, &DownloadService::taskStarted,
        this, &DownloadManagerPage::onTaskStarted);
    connect(m_downloadService, &DownloadService::taskProgress,
        this, &DownloadManagerPage::onTaskProgress);
    connect(m_downloadService, &DownloadService::taskCompleted,
        this, &DownloadManagerPage::onTaskCompleted);
    connect(m_downloadService, &DownloadService::taskFailed,
        this, &DownloadManagerPage::onTaskFailed);
    connect(m_downloadService, &DownloadService::taskSkipped,
        this, &DownloadManagerPage::onTaskSkipped);
}

void DownloadManagerPage::onTaskSkipped(const QString& identifier, const Song& existingSong)
{
    qDebug() << "前端收到跳过信号:" << identifier;

    // 显示友好的提示框
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("歌曲已存在");
    msgBox.setText(QString("📀 歌曲《%1》已存在于音乐库中").arg(existingSong.getTitle()));
    msgBox.setInformativeText(QString(
        "艺术家: %1\n"
        "时长: %2 秒\n"
        "下载日期: %3\n\n"
        "文件位置:\n%4"
    ).arg(existingSong.getArtist())
        .arg(existingSong.getDurationSeconds())
        .arg(existingSong.getDownloadDate().toString("yyyy-MM-dd HH:mm"))
        .arg(existingSong.getLocalFilePath()));

    msgBox.setStandardButtons(QMessageBox::Ok);

    // 样式美化
    msgBox.setStyleSheet(R"(
        QMessageBox {
            background-color: #2A2A2A;
            color: #FFFFFF;
        }
        QLabel {
            color: #FFFFFF;
            font-size: 13px;
        }
        QPushButton {
            background-color: #FB7299;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #FF8BB5;
        }
    )");

    msgBox.exec();

    // 更新状态栏
    m_statusLabel->setText(QString("ℹ️ 歌曲《%1》已存在，已跳过").arg(existingSong.getTitle()));

    // 清空输入框
    m_urlInput->clear();
}

void DownloadManagerPage::setupStyles()
{
    setStyleSheet(R"(
        QGroupBox#downloadInputGroup {
            font-size: 14px;
            font-weight: bold;
            border: 2px solid #444444;
            border-radius: 12px;
            margin-top: 12px;
            padding-top: 20px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,0.03),
                stop:1 rgba(255,255,255,0.01));
        }
        
        QGroupBox#downloadInputGroup::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px;
            color: #FB7299;
        }
        
        QLineEdit#urlInput {
            padding: 10px;
            border: 2px solid #444444;
            border-radius: 8px;
            background-color: #252525;
            color: #FFFFFF;
            font-size: 13px;
        }
        
        QLineEdit#urlInput:focus {
            border: 2px solid #FB7299;
        }
        
        QComboBox#qualityCombo {
            padding: 8px;
            border: 2px solid #444444;
            border-radius: 8px;
            background-color: #252525;
            color: #FFFFFF;
        }
        
        QComboBox#qualityCombo::drop-down {
            border: none;
            width: 30px;
        }
        
        QComboBox#qualityCombo::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #CCCCCC;
            margin-right: 8px;
        }
        
        QComboBox#qualityCombo QAbstractItemView {
            background-color: #2A2A2A;
            color: #FFFFFF;
            border: 2px solid #444444;
            border-radius: 6px;
            selection-background-color: #FB7299;
            selection-color: #FFFFFF;
            padding: 4px;
        }
        
        QComboBox#qualityCombo QAbstractItemView::item {
            padding: 8px;
            border-radius: 4px;
        }
        
        QComboBox#qualityCombo QAbstractItemView::item:hover {
            background-color: rgba(251, 114, 153, 0.2);
        }

        QPushButton#startDownloadBtn {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FF8BB5, stop:1 #FB7299);
            border: none;
            border-radius: 8px;
            color: #FFFFFF;
            font-size: 14px;
            font-weight: bold;
            min-width: 150px;
        }
        
        QPushButton#startDownloadBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFB3D1, stop:1 #FF8BB5);
        }
        
        QPushButton#batchDownloadBtn,
        QPushButton#controlBtn {
            background-color: #333333;
            border: 2px solid #555555;
            border-radius: 8px;
            color: #CCCCCC;
            font-size: 13px;
            min-width: 120px;
        }
        
        QPushButton#batchDownloadBtn:hover,
        QPushButton#controlBtn:hover {
            background-color: #444444;
            border-color: #FB7299;
            color: #FFFFFF;
        }
        
        /* 🆕 新增样式：打开文件夹按钮 */
        QPushButton#openFolderBtn {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #4CAF50, stop:1 #45A049);
            border: none;
            border-radius: 8px;
            color: #FFFFFF;
            font-size: 13px;
            min-width: 140px;
        }
        
        QPushButton#openFolderBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #66BB6A, stop:1 #4CAF50);
        }
        
        QLabel#downloadStatusLabel {
            color: #AAAAAA;
            font-size: 13px;
            padding: 5px;
        }
        
        QListWidget#downloadQueueList {
            background-color: #1A1A1A;
            border: none;
            padding: 10px;
            color: #FFFFFF;
            font-size: 14px;
        }
        
        QListWidget#downloadQueueList::item {
            background-color: transparent;
            color: #FFFFFF;
            padding: 4px;
        }
        
        QListWidget#downloadQueueList::item:hover {
            background-color: rgba(251, 114, 153, 0.1);
        }
        
        QListWidget#downloadQueueList::item:selected {
            background-color: rgba(251, 114, 153, 0.2);
        }
        
        QTableWidget#downloadHistoryTable {
            background-color: #1A1A1A;
            gridline-color: #333333;
            border: none;
            color: #FFFFFF;
            font-size: 14px;
        }
        
        QTableWidget#downloadHistoryTable::item {
            padding: 8px;
            color: #FFFFFF;
        }
        
        QTableWidget#downloadHistoryTable::item:hover {
            background-color: rgba(251, 114, 153, 0.1);
        }
        
        QTableWidget#downloadHistoryTable::item:selected {
            background-color: rgba(251, 114, 153, 0.25);
            color: #FFFFFF;
        }
        
        QHeaderView::section {
            background-color: #2A2A2A;
            color: #FB7299;
            font-size: 13px;
            font-weight: bold;
            padding: 8px;
            border: none;
            border-bottom: 2px solid #FB7299;
        }
    )");
}

bool DownloadManagerPage::validateInput() const
{
    QString url = m_urlInput->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(const_cast<DownloadManagerPage*>(this),
            "输入错误", "请输入 BV 号或 URL！");
        return false;
    }
    return true;
}

void DownloadManagerPage::onStartDownloadClicked()
{
    if (!validateInput()) return;

    QString identifier = m_urlInput->text().trimmed();
    QString preset = m_qualityCombo->currentData().toString();

    qDebug() << "添加下载任务:" << identifier << "音质:" << preset;

    m_downloadService->addDownloadTask(identifier, preset);
    m_urlInput->clear();
    m_tabWidget->setCurrentIndex(0);
}

void DownloadManagerPage::onTaskAdded(const DownloadService::DownloadTask& task)
{
    qDebug() << "任务已添加:" << task.identifier;
    addTaskToQueue(task);

    int queueSize = m_downloadService->getQueueSize();
    m_statusLabel->setText(QString("📥 队列中有 %1 个任务").arg(queueSize));
}

void DownloadManagerPage::onTaskStarted(const DownloadService::DownloadTask& task)
{
    qDebug() << "任务开始:" << task.identifier;
    m_statusLabel->setText(QString("⏬ 正在下载: %1").arg(task.identifier));

    DownloadTaskItem* item = findTaskItem(task.identifier);
    if (item) {
        item->setStatus("正在下载...");
    }
}

void DownloadManagerPage::onTaskProgress(
    const DownloadService::DownloadTask& task,
    double progress,
    const QString& message)
{
    DownloadTaskItem* item = findTaskItem(task.identifier);
    if (item) {
        item->setProgress(progress);
        item->setStatus(message);
    }

    qDebug() << "进度更新:" << task.identifier << progress << message;
}

void DownloadManagerPage::onTaskCompleted(
    const DownloadService::DownloadTask& task,
    const Song& song)
{
    qDebug() << "任务完成:" << song.getTitle();
    qDebug() << "📁 文件保存在:" << song.getLocalFilePath();

    moveTaskToHistory(task.identifier, true);

    int completed = m_downloadService->getCompletedCount();
    m_statusLabel->setText(QString("✅ 已完成 %1 个下载").arg(completed));

    // 添加到历史表格
    int row = m_historyTable->rowCount();
    m_historyTable->insertRow(row);
    m_historyTable->setItem(row, 0, new QTableWidgetItem("✅ 成功"));
    m_historyTable->setItem(row, 1, new QTableWidgetItem(song.getTitle()));
    m_historyTable->setItem(row, 2, new QTableWidgetItem(song.getArtist()));
    m_historyTable->setItem(row, 3,
        new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")));
    m_historyTable->setItem(row, 4, new QTableWidgetItem("下载成功"));
}

void DownloadManagerPage::onTaskFailed(
    const DownloadService::DownloadTask& task,
    const QString& error)
{
    qDebug() << "任务失败:" << task.identifier << error;

    moveTaskToHistory(task.identifier, false);

    int failed = m_downloadService->getFailedCount();
    m_statusLabel->setText(QString("❌ %1 个任务失败").arg(failed));

    // 添加到历史表格
    int row = m_historyTable->rowCount();
    m_historyTable->insertRow(row);
    m_historyTable->setItem(row, 0, new QTableWidgetItem("❌ 失败"));
    m_historyTable->setItem(row, 1, new QTableWidgetItem(task.identifier));
    m_historyTable->setItem(row, 2, new QTableWidgetItem("-"));
    m_historyTable->setItem(row, 3,
        new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")));
    m_historyTable->setItem(row, 4, new QTableWidgetItem(error));
}

void DownloadManagerPage::addTaskToQueue(const DownloadService::DownloadTask& task)
{
    DownloadTaskItem* taskItem = new DownloadTaskItem(task, this);
    m_taskItems.insert(task.identifier, taskItem);

    QListWidgetItem* listItem = new QListWidgetItem(m_queueList);
    listItem->setSizeHint(taskItem->sizeHint());
    m_queueList->addItem(listItem);
    m_queueList->setItemWidget(listItem, taskItem);
}

void DownloadManagerPage::moveTaskToHistory(const QString& identifier, bool success)
{
    DownloadTaskItem* item = m_taskItems.value(identifier, nullptr);
    if (!item) return;

    for (int i = 0; i < m_queueList->count(); ++i) {
        QListWidgetItem* listItem = m_queueList->item(i);
        if (m_queueList->itemWidget(listItem) == item) {
            m_queueList->takeItem(i);
            break;
        }
    }

    m_taskItems.remove(identifier);
    item->deleteLater();
}

DownloadTaskItem* DownloadManagerPage::findTaskItem(const QString& identifier) const
{
    return m_taskItems.value(identifier, nullptr);
}
