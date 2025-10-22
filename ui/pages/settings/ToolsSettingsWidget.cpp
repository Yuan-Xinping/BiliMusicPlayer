#include "ToolsSettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>

ToolsSettingsWidget::ToolsSettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
}

void ToolsSettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QGroupBox* toolsGroup = new QGroupBox("🔧 工具路径");
    toolsGroup->setObjectName("settingsGroup");
    QVBoxLayout* toolsLayout = new QVBoxLayout(toolsGroup);
    toolsLayout->setSpacing(16);

    // yt-dlp 路径
    QHBoxLayout* ytdlpLayout = new QHBoxLayout();
    QLabel* ytdlpLabel = new QLabel("yt-dlp:");
    ytdlpLabel->setFixedWidth(80);
    ytdlpLabel->setObjectName("settingsLabel");

    m_ytDlpPathInput = new QLineEdit();
    m_ytDlpPathInput->setObjectName("settingsInput");
    m_ytDlpPathInput->setReadOnly(true);

    m_browseYtDlpBtn = new QPushButton("📁 浏览");
    m_browseYtDlpBtn->setObjectName("browseBtn");
    m_browseYtDlpBtn->setFixedWidth(80);

    m_ytDlpStatusLabel = new QLabel("未检测");
    m_ytDlpStatusLabel->setObjectName("statusLabel");
    m_ytDlpStatusLabel->setFixedWidth(200);

    ytdlpLayout->addWidget(ytdlpLabel);
    ytdlpLayout->addWidget(m_ytDlpPathInput);
    ytdlpLayout->addWidget(m_browseYtDlpBtn);
    ytdlpLayout->addWidget(m_ytDlpStatusLabel);

    // FFmpeg 路径
    QHBoxLayout* ffmpegLayout = new QHBoxLayout();
    QLabel* ffmpegLabel = new QLabel("FFmpeg:");
    ffmpegLabel->setFixedWidth(80);
    ffmpegLabel->setObjectName("settingsLabel");

    m_ffmpegPathInput = new QLineEdit();
    m_ffmpegPathInput->setObjectName("settingsInput");
    m_ffmpegPathInput->setReadOnly(true);

    m_browseFfmpegBtn = new QPushButton("📁 浏览");
    m_browseFfmpegBtn->setObjectName("browseBtn");
    m_browseFfmpegBtn->setFixedWidth(80);

    m_ffmpegStatusLabel = new QLabel("未检测");
    m_ffmpegStatusLabel->setObjectName("statusLabel");
    m_ffmpegStatusLabel->setFixedWidth(200);

    ffmpegLayout->addWidget(ffmpegLabel);
    ffmpegLayout->addWidget(m_ffmpegPathInput);
    ffmpegLayout->addWidget(m_browseFfmpegBtn);
    ffmpegLayout->addWidget(m_ffmpegStatusLabel);

    // 说明文本
    QLabel* infoLabel = new QLabel(
        "💡 提示：\n"
        "• 默认使用内置版本的 yt-dlp 和 FFmpeg\n"
        "• 点击\"浏览\"可以选择自定义版本\n"
        "• 选择后会自动验证工具是否可用"
    );
    infoLabel->setObjectName("infoLabel");
    infoLabel->setWordWrap(true);

    toolsLayout->addLayout(ytdlpLayout);
    toolsLayout->addLayout(ffmpegLayout);
    toolsLayout->addWidget(m_testAllBtn);
    toolsLayout->addSpacing(10);
    toolsLayout->addWidget(infoLabel);

    mainLayout->addWidget(toolsGroup);
    mainLayout->addStretch();

    connect(m_browseYtDlpBtn, &QPushButton::clicked,
        this, &ToolsSettingsWidget::onBrowseYtDlpPathClicked);
    connect(m_browseFfmpegBtn, &QPushButton::clicked,
        this, &ToolsSettingsWidget::onBrowseFfmpegPathClicked);
    connect(m_testAllBtn, &QPushButton::clicked,
        this, &ToolsSettingsWidget::onTestAllToolsClicked);
}

void ToolsSettingsWidget::setupStyles()
{
    
}

void ToolsSettingsWidget::loadSettings()
{
    AppConfig& config = AppConfig::instance();

    QString ytDlpPath = config.getYtDlpPath();
    QString ffmpegPath = config.getFfmpegPath();

    m_ytDlpPathInput->setText(ytDlpPath);
    m_ffmpegPathInput->setText(ffmpegPath);

    testYtDlpPath(ytDlpPath);
    testFfmpegPath(ffmpegPath);
}

bool ToolsSettingsWidget::validate()
{
    return true;
}

void ToolsSettingsWidget::applySettings()
{
    AppConfig& config = AppConfig::instance();

    config.setYtDlpPath(m_ytDlpPathInput->text());
    config.setFfmpegPath(m_ffmpegPathInput->text());
}

void ToolsSettingsWidget::onBrowseYtDlpPathClicked()
{
    QString file = QFileDialog::getOpenFileName(
        this,
        "选择 yt-dlp 可执行文件",
        m_ytDlpPathInput->text(),
        "Executable (*.exe)"
    );

    if (!file.isEmpty()) {
        m_ytDlpPathInput->setText(file);
        testYtDlpPath(file);
    }
}

void ToolsSettingsWidget::onBrowseFfmpegPathClicked()
{
    QString file = QFileDialog::getOpenFileName(
        this,
        "选择 FFmpeg 可执行文件",
        m_ffmpegPathInput->text(),
        "Executable (*.exe)"
    );

    if (!file.isEmpty()) {
        m_ffmpegPathInput->setText(file);
        testFfmpegPath(file);
    }
}

void ToolsSettingsWidget::testYtDlpPath(const QString& path)
{
    if (path.isEmpty()) {
        m_ytDlpStatusLabel->setText("❌ 未设置");
        m_ytDlpStatusLabel->setStyleSheet("color: #F44336;");
        return;
    }

    m_ytDlpStatusLabel->setText("🔄 测试中...");
    m_ytDlpStatusLabel->setStyleSheet("color: #FFA726;");

    QProcess process;
    process.start(path, QStringList() << "--version");

    if (process.waitForFinished(3000)) {
        QString output = process.readAllStandardOutput().trimmed();
        if (!output.isEmpty()) {
            m_ytDlpStatusLabel->setText("✅ " + output);
            m_ytDlpStatusLabel->setStyleSheet("color: #4CAF50;");
            qDebug() << "✅ yt-dlp 可用，版本:" << output;
        }
        else {
            m_ytDlpStatusLabel->setText("❌ 无效");
            m_ytDlpStatusLabel->setStyleSheet("color: #F44336;");
        }
    }
    else {
        m_ytDlpStatusLabel->setText("❌ 无法执行");
        m_ytDlpStatusLabel->setStyleSheet("color: #F44336;");
    }
}

void ToolsSettingsWidget::testFfmpegPath(const QString& path)
{
    if (path.isEmpty()) {
        m_ffmpegStatusLabel->setText("❌ 未设置");
        m_ffmpegStatusLabel->setStyleSheet("color: #F44336;");
        return;
    }

    m_ffmpegStatusLabel->setText("🔄 测试中...");
    m_ffmpegStatusLabel->setStyleSheet("color: #FFA726;");

    QProcess process;
    process.start(path, QStringList() << "-version");

    if (process.waitForFinished(3000)) {
        QString output = process.readAllStandardOutput();
        if (output.contains("ffmpeg version")) {
            m_ffmpegStatusLabel->setText("✅ 可用");
            m_ffmpegStatusLabel->setStyleSheet("color: #4CAF50;");
            qDebug() << "✅ FFmpeg 可用";
        }
        else {
            m_ffmpegStatusLabel->setText("❌ 无效");
            m_ffmpegStatusLabel->setStyleSheet("color: #F44336;");
        }
    }
    else {
        m_ffmpegStatusLabel->setText("❌ 无法执行");
        m_ffmpegStatusLabel->setStyleSheet("color: #F44336;");
    }
}

void ToolsSettingsWidget::onTestAllToolsClicked()
{
    qDebug() << "开始测试所有工具...";
    testYtDlpPath(m_ytDlpPathInput->text());
    testFfmpegPath(m_ffmpegPathInput->text());
}