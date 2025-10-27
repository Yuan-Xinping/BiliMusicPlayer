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
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

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
    m_categoryList->addItem("🎨 界面设置");
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
    m_subPagesStack->addWidget(m_uiSettings);
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
    if (!validateAllSettings()) {
        return;
    }

    applyAllSettings();

    AppConfig& config = AppConfig::instance();
    if (!config.save()) {
        qWarning() << "❌ 配置文件保存失败";
        QMessageBox::critical(this, "❌ 保存失败",
            "配置文件保存失败！\n请检查文件权限。");
        return;
    }

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
        "⚠️ 确认恢复",
        "确定要恢复所有设置为默认值吗？\n\n"
        "默认设置：\n"
        "📁 下载路径: C:/Users/用户名/BiliMusicPlayer_Downloads\n"
        "🎵 默认音质: 高品质 (192K MP3)\n"
        "⚡ 并发下载: 3 个任务\n"
        "💾 数据库: C:/Users/用户名/BiliMusicPlayer/bili_music_player.db\n"
        "🔧 工具: 内置工具\n"
        "🎨 界面主题: 浅色\n"
        "🌐 代理: 关闭\n\n"
        "⚠️ 当前设置将被覆盖！",
        QMessageBox::No | QMessageBox::Yes,
        QMessageBox::No  // 默认选中 No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    qDebug() << "🔄 开始恢复默认设置...";

    // 重置 AppConfig 为默认值
    if (!resetConfigToDefaults()) {
        QMessageBox::critical(this, "❌ 失败",
            "恢复默认设置失败！\n请检查文件权限。");
        return;
    }

    // 重新加载所有设置页面
    loadAllSettings();

    qDebug() << "✅ 默认设置已恢复";

    QMessageBox::information(this, "✅ 成功",
        "所有设置已恢复为默认值！\n\n"
        "📝 提示：请点击\"保存所有设置\"以持久化更改。");
}

bool SettingsPage::resetConfigToDefaults()
{
    try {
        AppConfig& config = AppConfig::instance();

        // 获取用户主目录
        QString userHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

        // 1️⃣ 下载设置 - 默认值
        QString defaultDownloadPath = userHome + "/BiliMusicPlayer_Downloads";
        config.setDownloadPath(defaultDownloadPath);
        config.setDefaultQualityPreset("medium_quality_mp3"); // 192kbps
        // 不再单独设置格式，交由 AppConfig 根据预设自动同步
        config.setMaxConcurrentDownloads(3);

        // 2️⃣ 数据库设置 - 默认值
        QString defaultDbPath = userHome + "/BiliMusicPlayer/bili_music_player.db";
        config.setDatabasePath(defaultDbPath);

        // 3️⃣ 工具设置 - 智能检测
        QString defaultYtDlpPath = findDefaultToolPath("yt-dlp.exe");
        QString defaultFfmpegPath = findDefaultToolPath("ffmpeg.exe");

        config.setYtDlpPath(defaultYtDlpPath);
        config.setFfmpegPath(defaultFfmpegPath);

        qDebug() << "  ✅ 工具设置已重置:";
        qDebug() << "    - yt-dlp:" << defaultYtDlpPath;
        qDebug() << "    - FFmpeg:" << defaultFfmpegPath;

        // 4️⃣ 界面设置 - 默认值
        config.setTheme("light");

        // 5️⃣ 高级设置 - 默认值
        config.setProxyEnabled(false);
        config.setProxyUrl("");

        // 保存配置
        if (!config.save()) {
            qWarning() << "❌ 配置文件保存失败";
            return false;
        }

        qDebug() << "✅ 默认设置已保存到配置文件";
        return true;

    }
    catch (const std::exception& e) {
        qCritical() << "❌ 恢复默认设置时发生异常:" << e.what();
        return false;
    }
}

QString SettingsPage::findDefaultToolPath(const QString& toolName)
{
    QString appDir = QCoreApplication::applicationDirPath();

    // 尝试多个可能的位置
    QStringList possiblePaths = {
        appDir + "/" + toolName,                  // app/Debug/yt-dlp.exe
        appDir + "/../bin/" + toolName,           // bin/yt-dlp.exe (Release)
        appDir + "/../../bin/" + toolName,        // bin/yt-dlp.exe (Debug)
    };

    // 查找第一个存在的文件
    for (const QString& path : possiblePaths) {
        QString cleanPath = QDir::cleanPath(path);
        if (QFile::exists(cleanPath)) {
            qDebug() << "    ✅ 找到工具:" << cleanPath;
            return cleanPath;
        }
    }

    // 如果都不存在，返回最常见的位置
    QString fallbackPath = QDir::cleanPath(appDir + "/../../bin/" + toolName);
    qWarning() << "    ⚠️  未找到工具，使用默认路径:" << fallbackPath;
    return fallbackPath;
}

