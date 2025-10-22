#include "SettingsPage.h"
#include "DownloadSettingsWidget.h"
#include "DatabaseSettingsWidget.h"
#include "ToolsSettingsWidget.h"
#include "UISettingsWidget.h"
#include "AdvancedSettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
    loadAllSettings();
}

void SettingsPage::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    setupSidebar();

    // 右侧内容区域
    QVBoxLayout* contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(20);

    setupSubPages();
    setupButtons();

    contentLayout->addWidget(m_subPagesStack, 1);
    contentLayout->addLayout(createButtonLayout());

    mainLayout->addWidget(m_categoryList);
    mainLayout->addLayout(contentLayout, 1);
}

void SettingsPage::setupSidebar()
{
    m_categoryList = new QListWidget();
    m_categoryList->setObjectName("settingsSidebar");
    m_categoryList->setFixedWidth(180);
    m_categoryList->setSpacing(4);

    // 添加分类项
    m_categoryList->addItem("📁 下载设置");
    m_categoryList->addItem("💾 数据库设置");
    m_categoryList->addItem("🔧 工具路径");
    //m_categoryList->addItem("🎨 界面设置");
    m_categoryList->addItem("🔬 高级设置");

    m_categoryList->setCurrentRow(0);

    connect(m_categoryList, &QListWidget::currentRowChanged,
        this, &SettingsPage::onCategorySelected);
}

void SettingsPage::setupSubPages()
{
    m_subPagesStack = new QStackedWidget();

    m_downloadSettings = new DownloadSettingsWidget();
    m_databaseSettings = new DatabaseSettingsWidget();
    m_toolsSettings = new ToolsSettingsWidget();
    m_uiSettings = new UISettingsWidget();
    m_advancedSettings = new AdvancedSettingsWidget();

    m_subPagesStack->addWidget(m_downloadSettings);
    m_subPagesStack->addWidget(m_databaseSettings);
    m_subPagesStack->addWidget(m_toolsSettings);
    //m_subPagesStack->addWidget(m_uiSettings);
    m_subPagesStack->addWidget(m_advancedSettings);
}

void SettingsPage::setupButtons()
{
    m_saveBtn = new QPushButton("💾 保存所有设置");
    m_saveBtn->setObjectName("saveBtn");
    m_saveBtn->setFixedHeight(40);
    m_saveBtn->setMinimumWidth(150);

    m_resetBtn = new QPushButton("🔄 恢复默认");
    m_resetBtn->setObjectName("resetBtn");
    m_resetBtn->setFixedHeight(40);
    m_resetBtn->setMinimumWidth(150);

    connect(m_saveBtn, &QPushButton::clicked,
        this, &SettingsPage::onSaveAllSettingsClicked);
    connect(m_resetBtn, &QPushButton::clicked,
        this, &SettingsPage::onResetToDefaultsClicked);
}

QHBoxLayout* SettingsPage::createButtonLayout()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_resetBtn);
    buttonLayout->addWidget(m_saveBtn);
    return buttonLayout;
}

void SettingsPage::setupStyles()
{
    setStyleSheet(R"(
        QListWidget#settingsSidebar {
            background-color: #1E1E1E;
            border: 2px solid #444444;
            border-radius: 12px;
            padding: 10px;
            font-size: 13px;
        }
        
        QListWidget#settingsSidebar::item {
            color: #CCCCCC;
            padding: 12px;
            border-radius: 8px;
            margin: 2px 0px;
        }
        
        QListWidget#settingsSidebar::item:hover {
            background-color: rgba(251, 114, 153, 0.1);
            color: #FFFFFF;
        }
        
        QListWidget#settingsSidebar::item:selected {
            background-color: #FB7299;
            color: #FFFFFF;
            font-weight: bold;
        }
        
        QPushButton#saveBtn {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FF8BB5, stop:1 #FB7299);
            border: none;
            border-radius: 8px;
            color: #FFFFFF;
            font-size: 14px;
            font-weight: bold;
        }
        
        QPushButton#saveBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFB3D1, stop:1 #FF8BB5);
        }
        
        QPushButton#resetBtn {
            background-color: #333333;
            border: 2px solid #555555;
            border-radius: 8px;
            color: #CCCCCC;
            font-size: 14px;
        }
        
        QPushButton#resetBtn:hover {
            background-color: #444444;
            border-color: #FB7299;
            color: #FFFFFF;
        }
    )");
}

void SettingsPage::onCategorySelected(int index)
{
    m_subPagesStack->setCurrentIndex(index);
    qDebug() << "切换到设置子页面:" << index;
}

void SettingsPage::loadAllSettings()
{
    m_downloadSettings->loadSettings();
    m_databaseSettings->loadSettings();
    m_toolsSettings->loadSettings();
    m_uiSettings->loadSettings();
    m_advancedSettings->loadSettings();

    qDebug() << "所有设置已加载";
}

bool SettingsPage::validateAllSettings()
{
    if (!m_downloadSettings->validate()) return false;
    if (!m_databaseSettings->validate()) return false;
    if (!m_toolsSettings->validate()) return false;
    if (!m_uiSettings->validate()) return false;
    if (!m_advancedSettings->validate()) return false;
    return true;
}

void SettingsPage::applyAllSettings()
{
    m_downloadSettings->applySettings();
    m_databaseSettings->applySettings();
    m_toolsSettings->applySettings();
    m_uiSettings->applySettings();
    m_advancedSettings->applySettings();

    qDebug() << "所有设置已应用";
}

void SettingsPage::onSaveAllSettingsClicked()
{
    // 1. 验证所有设置
    if (!validateAllSettings()) {
        return;
    }

    // 2. 应用所有设置（仅修改内存中的配置）
    applyAllSettings();

    // 3. 🔥 保存配置到文件
    AppConfig& config = AppConfig::instance();
    if (!config.save()) {
        qWarning() << "❌ 配置文件保存失败";
        QMessageBox::critical(this, "❌ 保存失败",
            "配置文件保存失败！\n请检查文件权限。");
        return;
    }

    // 4. 日志输出
    qDebug() << "✅ 所有设置已保存到配置文件";
    qDebug() << "  - 下载路径:" << config.getDownloadPath();
    qDebug() << "  - 默认音质:" << config.getDefaultQualityPreset();
    qDebug() << "  - 默认格式:" << static_cast<int>(config.getDefaultAudioFormat());
    qDebug() << "  - 并行下载数:" << config.getMaxConcurrentDownloads();

    emit settingsChanged();

    QMessageBox::information(this, "✅ 成功",
        "所有设置已保存！\n部分设置需要重启应用后生效。");
}

void SettingsPage::onResetToDefaultsClicked()
{
    auto reply = QMessageBox::question(
        this,
        "确认恢复",
        "确定要恢复所有设置为默认值吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QMessageBox::information(this, "提示", "恢复默认功能正在开发中...");
    }
}
