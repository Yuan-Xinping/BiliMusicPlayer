#include "DownloadManagerPage.h"
#include "../components/DownloadTaskItem.h"
#include "../../common/AppConfig.h"
#include "../../service/DownloadService.h"

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

DownloadManagerPage::DownloadManagerPage(DownloadViewModel* viewModel, QWidget* parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    Q_ASSERT(m_viewModel != nullptr);

    qDebug() << "🔧 DownloadManagerPage 构造函数开始...";

    setupUI();
    setupConnections();
    loadDefaultSettings();

    QString downloadPath = m_viewModel->getDownloadPath();
    qDebug() << "====================================";
    qDebug() << "📁 当前下载路径：" << downloadPath;
    qDebug() << "====================================";

    // 初始化状态栏
    m_statusLabel->setText(m_viewModel->statusText());
}

void DownloadManagerPage::setupUI()
{
    qDebug() << "🔧 DownloadManagerPage::setupUI() 开始...";

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
    m_qualityCombo->addItem("🌟 最佳音质 (自动)", "best_quality");
    m_qualityCombo->addItem("🎼 无损音质 WAV", "lossless_wav");
    m_qualityCombo->addItem("🎵 无损音质 FLAC", "lossless_flac");
    m_qualityCombo->addItem("🎧 高品质 MP3 (320kbps)", "high_quality_mp3");
    m_qualityCombo->addItem("🎶 标准 MP3 (192kbps)", "medium_quality_mp3");
    m_qualityCombo->addItem("🎹 小文件 OPUS", "small_size_opus");
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

    // ========== 状态栏 ==========
    QHBoxLayout* statusLayout = new QHBoxLayout();

    m_statusLabel = new QLabel();
    m_statusLabel->setObjectName("downloadStatusLabel");

    QPushButton* openFolderBtn = new QPushButton("📁 打开下载文件夹");
    openFolderBtn->setObjectName("openFolderBtn");
    openFolderBtn->setFixedHeight(35);
    connect(openFolderBtn, &QPushButton::clicked, [this]() {
        m_viewModel->openDownloadFolder();
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
    connect(m_startBtn, &QPushButton::clicked,
        this, &DownloadManagerPage::onStartDownloadClicked);

    connect(m_viewModel, &DownloadViewModel::taskAdded,
        this, &DownloadManagerPage::onTaskAdded);
    connect(m_viewModel, &DownloadViewModel::taskStarted,
        this, &DownloadManagerPage::onTaskStarted);
    connect(m_viewModel, &DownloadViewModel::taskProgressUpdated,
        this, &DownloadManagerPage::onTaskProgressUpdated);
    connect(m_viewModel, &DownloadViewModel::taskCompleted,
        this, &DownloadManagerPage::onTaskCompleted);
    connect(m_viewModel, &DownloadViewModel::taskFailed,
        this, &DownloadManagerPage::onTaskFailed);
    connect(m_viewModel, &DownloadViewModel::taskSkipped,
        this, &DownloadManagerPage::onTaskSkipped);

    // 状态文本自动更新
    connect(m_viewModel, &DownloadViewModel::statusTextChanged, this, [this]() {
        m_statusLabel->setText(m_viewModel->statusText());
        });

    // 错误处理
    connect(m_viewModel, &DownloadViewModel::errorOccurred, this,
        [this](const QString& title, const QString& message) {
            QMessageBox::warning(this, title, message);
        });
}

bool DownloadManagerPage::validateInput() const
{
    return !m_urlInput->text().trimmed().isEmpty();
}

void DownloadManagerPage::onStartDownloadClicked()
{
    if (!validateInput()) {
        QMessageBox::warning(const_cast<DownloadManagerPage*>(this),
            "输入错误", "请输入 BV 号或 URL！");
        return;
    }

    QString identifier = m_urlInput->text().trimmed();
    QString preset = m_qualityCombo->currentData().toString();

    qDebug() << "UI: 提交下载任务:" << identifier << "音质:" << preset;

    m_viewModel->addDownloadTaskWithPreset(identifier, preset);

    m_urlInput->clear();
    m_tabWidget->setCurrentIndex(0);
}

void DownloadManagerPage::onTaskAdded(const QString& identifier)
{
    qDebug() << "UI: 任务已添加:" << identifier;
    addTaskToQueue(identifier);
}

void DownloadManagerPage::onTaskStarted(const QString& identifier)
{
    qDebug() << "UI: 任务开始:" << identifier;

    DownloadTaskItem* item = findTaskItem(identifier);
    if (item) {
        item->setStatus("正在下载...");
    }
}

void DownloadManagerPage::onTaskProgressUpdated(
    const QString& identifier,
    double progress,
    const QString& message)
{
    DownloadTaskItem* item = findTaskItem(identifier);
    if (item) {
        item->setProgress(progress);
        item->setStatus(message);
    }
}

void DownloadManagerPage::onTaskCompleted(const QString& identifier, const Song& song)
{
    qDebug() << "UI: 任务完成:" << song.getTitle();

    moveTaskToHistory(identifier, true);

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

void DownloadManagerPage::onTaskFailed(const QString& identifier, const QString& error)
{
    qDebug() << "UI: 任务失败:" << identifier << error;

    moveTaskToHistory(identifier, false);

    // 添加到历史表格
    int row = m_historyTable->rowCount();
    m_historyTable->insertRow(row);
    m_historyTable->setItem(row, 0, new QTableWidgetItem("❌ 失败"));
    m_historyTable->setItem(row, 1, new QTableWidgetItem(identifier));
    m_historyTable->setItem(row, 2, new QTableWidgetItem("-"));
    m_historyTable->setItem(row, 3,
        new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm")));
    m_historyTable->setItem(row, 4, new QTableWidgetItem(error));
}

void DownloadManagerPage::onTaskSkipped(const QString& identifier, const Song& existingSong)
{
    qDebug() << "UI: 任务跳过:" << identifier;

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
    msgBox.exec();

    m_urlInput->clear();
}

void DownloadManagerPage::addTaskToQueue(const QString& identifier)
{
    DownloadService::DownloadTask tempTask;
    tempTask.identifier = identifier;
    tempTask.status = DownloadService::DownloadStatus::Idle;

    DownloadTaskItem* taskItem = new DownloadTaskItem(tempTask, this);
    m_taskItems.insert(identifier, taskItem);

    QListWidgetItem* listItem = new QListWidgetItem(m_queueList);
    listItem->setSizeHint(taskItem->sizeHint());
    m_queueList->addItem(listItem);
    m_queueList->setItemWidget(listItem, taskItem);
}

void DownloadManagerPage::moveTaskToHistory(const QString& identifier, bool success)
{
    Q_UNUSED(success);

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

void DownloadManagerPage::loadDefaultSettings()
{
    if (!m_qualityCombo) {
        qCritical() << "❌ loadDefaultSettings: m_qualityCombo 为空！";
        return;
    }

    QString defaultPreset = m_viewModel->currentQualityPreset();
    int presetIndex = m_qualityCombo->findData(defaultPreset);

    qDebug() << "📋 加载默认音质设置:" << defaultPreset;

    if (presetIndex >= 0) {
        m_qualityCombo->blockSignals(true);
        m_qualityCombo->setCurrentIndex(presetIndex);
        m_qualityCombo->blockSignals(false);
        qDebug() << "✅ 默认音质已设置";
    }
    else {
        qWarning() << "⚠️ 未找到音质预设，使用默认";
    }
}

void DownloadManagerPage::onSettingsChanged()
{
    qDebug() << "🔄 下载管理页面收到配置变更信号";

    m_viewModel->refreshConfig();
    loadDefaultSettings();

    qDebug() << "✅ 下载管理页面已刷新配置";
}

void DownloadManagerPage::setupStyles()
{
}
