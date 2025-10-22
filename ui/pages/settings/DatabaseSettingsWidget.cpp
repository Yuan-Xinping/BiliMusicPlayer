#include "DatabaseSettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

DatabaseSettingsWidget::DatabaseSettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
}

void DatabaseSettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QGroupBox* databaseGroup = new QGroupBox("💾 数据库设置");
    databaseGroup->setObjectName("settingsGroup");
    QVBoxLayout* databaseLayout = new QVBoxLayout(databaseGroup);
    databaseLayout->setSpacing(12);

    // 数据库路径
    QHBoxLayout* dbPathLayout = new QHBoxLayout();
    QLabel* dbPathLabel = new QLabel("数据库路径:");
    dbPathLabel->setFixedWidth(100);
    dbPathLabel->setObjectName("settingsLabel");

    m_databasePathInput = new QLineEdit();
    m_databasePathInput->setObjectName("settingsInput");
    m_databasePathInput->setReadOnly(true);

    m_browseDatabasePathBtn = new QPushButton("📁 浏览");
    m_browseDatabasePathBtn->setObjectName("browseBtn");
    m_browseDatabasePathBtn->setFixedWidth(80);

    dbPathLayout->addWidget(dbPathLabel);
    dbPathLayout->addWidget(m_databasePathInput);
    dbPathLayout->addWidget(m_browseDatabasePathBtn);

    // 数据库信息
    m_databaseInfoLabel = new QLabel("数据库大小: 加载中...");
    m_databaseInfoLabel->setObjectName("infoLabel");

    // 危险操作区域
    QGroupBox* dangerZone = new QGroupBox("⚠️ 危险操作");
    dangerZone->setObjectName("dangerZone");
    QVBoxLayout* dangerLayout = new QVBoxLayout(dangerZone);

    m_clearDatabaseBtn = new QPushButton("🗑️ 清空数据库");
    m_clearDatabaseBtn->setObjectName("dangerBtn");
    m_clearDatabaseBtn->setFixedWidth(120);

    QLabel* warningLabel = new QLabel("⚠️ 此操作将删除所有下载记录和音乐库数据，不可恢复！");
    warningLabel->setObjectName("warningLabel");
    warningLabel->setWordWrap(true);

    dangerLayout->addWidget(warningLabel);
    dangerLayout->addWidget(m_clearDatabaseBtn, 0, Qt::AlignLeft);

    databaseLayout->addLayout(dbPathLayout);
    databaseLayout->addWidget(m_databaseInfoLabel);
    databaseLayout->addSpacing(10);
    databaseLayout->addWidget(dangerZone);

    mainLayout->addWidget(databaseGroup);
    mainLayout->addStretch();

    connect(m_browseDatabasePathBtn, &QPushButton::clicked,
        this, &DatabaseSettingsWidget::onBrowseDatabasePathClicked);
    connect(m_clearDatabaseBtn, &QPushButton::clicked,
        this, &DatabaseSettingsWidget::onClearDatabaseClicked);
}

void DatabaseSettingsWidget::setupStyles()
{
    setStyleSheet(R"(
        QGroupBox#settingsGroup {
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
        
        QGroupBox#settingsGroup::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 8px;
            color: #FB7299;
        }
        
        QGroupBox#dangerZone {
            font-size: 13px;
            font-weight: bold;
            border: 2px solid #8B0000;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 15px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(139,0,0,0.1),
                stop:1 rgba(139,0,0,0.05));
        }
        
        QGroupBox#dangerZone::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
            color: #FF6B6B;
        }
        
        QLabel#settingsLabel {
            color: #CCCCCC;
            font-size: 13px;
            font-weight: normal;
        }
        
        QLabel#infoLabel {
            color: #AAAAAA;
            font-size: 12px;
            padding: 8px;
            background-color: #2A2A2A;
            border-radius: 6px;
        }
        
        QLabel#warningLabel {
            color: #FF6B6B;
            font-size: 12px;
            padding: 6px;
        }
        
        QLineEdit#settingsInput {
            padding: 8px;
            border: 2px solid #444444;
            border-radius: 6px;
            background-color: #252525;
            color: #FFFFFF;
            font-size: 12px;
        }
        
        QPushButton#browseBtn {
            background-color: #333333;
            border: 2px solid #555555;
            border-radius: 6px;
            color: #CCCCCC;
            font-size: 12px;
        }
        
        QPushButton#browseBtn:hover {
            background-color: #444444;
            border-color: #FB7299;
            color: #FFFFFF;
        }
        
        QPushButton#dangerBtn {
            background-color: #8B0000;
            border: none;
            border-radius: 6px;
            color: #FFFFFF;
            font-size: 12px;
            padding: 8px 16px;
        }
        
        QPushButton#dangerBtn:hover {
            background-color: #A00000;
        }
        
        QPushButton#dangerBtn:pressed {
            background-color: #7A0000;
        }
    )");
}

void DatabaseSettingsWidget::loadSettings()
{
    AppConfig& config = AppConfig::instance();
    m_databasePathInput->setText(config.getDatabasePath());

    // 更新数据库信息
    QFileInfo dbFile(config.getDatabasePath());
    if (dbFile.exists()) {
        qint64 size = dbFile.size();
        QString sizeStr;
        if (size < 1024) {
            sizeStr = QString::number(size) + " B";
        }
        else if (size < 1024 * 1024) {
            sizeStr = QString::number(size / 1024.0, 'f', 2) + " KB";
        }
        else {
            sizeStr = QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
        }
        m_databaseInfoLabel->setText(QString("数据库大小: %1").arg(sizeStr));
    }
    else {
        m_databaseInfoLabel->setText("数据库尚未创建");
    }
}

bool DatabaseSettingsWidget::validate()
{
    if (m_databasePathInput->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择数据库路径！");
        return false;
    }
    return true;
}

void DatabaseSettingsWidget::applySettings()
{
    AppConfig& config = AppConfig::instance();
    config.setDatabasePath(m_databasePathInput->text());
}

void DatabaseSettingsWidget::onBrowseDatabasePathClicked()
{
    QString file = QFileDialog::getSaveFileName(
        this,
        "选择数据库文件位置",
        m_databasePathInput->text(),
        "SQLite Database (*.db)"
    );

    if (!file.isEmpty()) {
        m_databasePathInput->setText(file);
    }
}

void DatabaseSettingsWidget::onClearDatabaseClicked()
{
    auto reply = QMessageBox::warning(
        this,
        "⚠️ 危险操作",
        "确定要清空数据库吗？这将删除所有下载记录和音乐库数据！\n\n此操作不可恢复！",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "提示", "数据库清空功能正在开发中...");
    }
}
