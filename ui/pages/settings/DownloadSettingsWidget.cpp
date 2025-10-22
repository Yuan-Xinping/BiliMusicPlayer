#include "DownloadSettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

DownloadSettingsWidget::DownloadSettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupQualityFormatMapping(); 
    setupUI();
    setupStyles();
}

void DownloadSettingsWidget::setupQualityFormatMapping()
{
    m_qualityToFormatMap["high_quality_mp3"] = AudioFormat::MP3;
    m_qualityToFormatMap["medium_quality_mp3"] = AudioFormat::MP3;
    m_qualityToFormatMap["lossless_flac"] = AudioFormat::FLAC;
    m_qualityToFormatMap["small_size_opus"] = AudioFormat::OPUS;
    m_qualityToFormatMap["lossless_wav"] = AudioFormat::WAV;
    m_qualityToFormatMap["best_quality"] = AudioFormat::FLAC; 

    qDebug() << "✅ 音质-格式映射表已建立，共" << m_qualityToFormatMap.size() << "项";
}

void DownloadSettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QGroupBox* downloadGroup = new QGroupBox("📁 下载设置");
    downloadGroup->setObjectName("settingsGroup");
    QVBoxLayout* downloadLayout = new QVBoxLayout(downloadGroup);
    downloadLayout->setSpacing(12);

    // ========== 下载路径 ==========
    QHBoxLayout* downloadPathLayout = new QHBoxLayout();
    QLabel* downloadPathLabel = new QLabel("下载路径:");
    downloadPathLabel->setFixedWidth(100);
    downloadPathLabel->setObjectName("settingsLabel");

    m_downloadPathInput = new QLineEdit();
    m_downloadPathInput->setObjectName("settingsInput");
    m_downloadPathInput->setPlaceholderText("选择下载文件夹");

    m_browseDownloadPathBtn = new QPushButton("📁 浏览");
    m_browseDownloadPathBtn->setObjectName("browseBtn");
    m_browseDownloadPathBtn->setFixedWidth(80);

    downloadPathLayout->addWidget(downloadPathLabel);
    downloadPathLayout->addWidget(m_downloadPathInput);
    downloadPathLayout->addWidget(m_browseDownloadPathBtn);

    // ========== 默认音质 ==========
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    QLabel* qualityLabel = new QLabel("默认音质:");
    qualityLabel->setFixedWidth(100);
    qualityLabel->setObjectName("settingsLabel");

    m_defaultQualityCombo = new QComboBox();
    m_defaultQualityCombo->addItem("🌟 最佳音质 (自动)", "best_quality");        // 1. 最佳
    m_defaultQualityCombo->addItem("🎼 无损音质 WAV", "lossless_wav");         // 2. WAV 无损
    m_defaultQualityCombo->addItem("🎵 无损音质 FLAC", "lossless_flac");       // 3. FLAC 无损
    m_defaultQualityCombo->addItem("🎧 高品质 MP3 (320kbps)", "high_quality_mp3");  // 4. 高品质
    m_defaultQualityCombo->addItem("🎶 标准 MP3 (192kbps)", "medium_quality_mp3");  // 5. 标准
    m_defaultQualityCombo->addItem("🎹 小文件 OPUS", "small_size_opus");        // 6. 最小

    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(m_defaultQualityCombo);
    qualityLayout->addStretch();

    // ========== 默认格式（只读显示） ==========
    QHBoxLayout* formatLayout = new QHBoxLayout();
    QLabel* formatLabel = new QLabel("对应格式:");
    formatLabel->setFixedWidth(100);
    formatLabel->setObjectName("settingsLabel");

    m_defaultFormatCombo = new QComboBox();
    m_defaultFormatCombo->setObjectName("settingsCombo");
    m_defaultFormatCombo->addItem("MP3", static_cast<int>(AudioFormat::MP3));
    m_defaultFormatCombo->addItem("M4A", static_cast<int>(AudioFormat::M4A));
    m_defaultFormatCombo->addItem("FLAC", static_cast<int>(AudioFormat::FLAC));
    m_defaultFormatCombo->addItem("OPUS", static_cast<int>(AudioFormat::OPUS));
    m_defaultFormatCombo->addItem("WAV", static_cast<int>(AudioFormat::WAV));  // 🔥 新增 WAV

    m_defaultFormatCombo->setEnabled(false);
    m_defaultFormatCombo->setToolTip("格式由音质预设自动决定");

    QLabel* formatHint = new QLabel("(自动根据音质设置)");
    formatHint->setObjectName("hintLabel");

    formatLayout->addWidget(formatLabel);
    formatLayout->addWidget(m_defaultFormatCombo);
    formatLayout->addWidget(formatHint);
    formatLayout->addStretch();

    // ========== 并行下载任务数 ==========
    QHBoxLayout* concurrentLayout = new QHBoxLayout();
    QLabel* concurrentLabel = new QLabel("并行下载:");
    concurrentLabel->setFixedWidth(100);
    concurrentLabel->setObjectName("settingsLabel");

    m_maxConcurrentDownloadsSpin = new QSpinBox();
    m_maxConcurrentDownloadsSpin->setObjectName("settingsSpin");
    m_maxConcurrentDownloadsSpin->setRange(1, 10);
    m_maxConcurrentDownloadsSpin->setValue(3);
    m_maxConcurrentDownloadsSpin->setSuffix(" 个任务");
    m_maxConcurrentDownloadsSpin->setButtonSymbols(QSpinBox::PlusMinus);

    QLabel* concurrentHint = new QLabel("(同时进行的下载任务数量)");
    concurrentHint->setObjectName("hintLabel");

    concurrentLayout->addWidget(concurrentLabel);
    concurrentLayout->addWidget(m_maxConcurrentDownloadsSpin);
    concurrentLayout->addWidget(concurrentHint);
    concurrentLayout->addStretch();

    // ========== 添加到主布局 ==========
    downloadLayout->addLayout(downloadPathLayout);
    downloadLayout->addLayout(qualityLayout);
    downloadLayout->addLayout(formatLayout);
    downloadLayout->addLayout(concurrentLayout);

    mainLayout->addWidget(downloadGroup);
    mainLayout->addStretch();

    // ========== 连接信号 ==========
    connect(m_browseDownloadPathBtn, &QPushButton::clicked,
        this, &DownloadSettingsWidget::onBrowseDownloadPathClicked);

    connect(m_defaultQualityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &DownloadSettingsWidget::onQualityPresetChanged);
}

void DownloadSettingsWidget::setupStyles()
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
        
        QLabel#settingsLabel {
            color: #CCCCCC;
            font-size: 13px;
            font-weight: normal;
        }
        
        QLabel#hintLabel {
            color: #999999;
            font-size: 11px;
            font-style: italic;
        }
        
        QLineEdit#settingsInput {
            padding: 8px;
            border: 2px solid #444444;
            border-radius: 6px;
            background-color: #252525;
            color: #FFFFFF;
            font-size: 12px;
        }
        
        QLineEdit#settingsInput:focus {
            border: 2px solid #FB7299;
        }
        
        QComboBox#settingsCombo {
            padding: 6px;
            border: 2px solid #444444;
            border-radius: 6px;
            background-color: #252525;
            color: #FFFFFF;
            font-size: 12px;
        }
        
        QComboBox#settingsCombo:disabled {
            background-color: #1A1A1A;
            color: #888888;
            border-color: #333333;
        }
        
        QComboBox#settingsCombo::drop-down {
            border: none;
            width: 25px;
        }
        
        QComboBox#settingsCombo QAbstractItemView {
            background-color: #2A2A2A;
            color: #FFFFFF;
            border: 2px solid #444444;
            selection-background-color: #FB7299;
        }
        
        QSpinBox#settingsSpin {
            padding: 6px 10px;
            border: 2px solid #444444;
            border-radius: 6px;
            background-color: #252525;
            color: #FFFFFF;
            font-size: 12px;
            min-width: 120px;
        }
        
        QSpinBox#settingsSpin::down-button {
            subcontrol-origin: border;
            subcontrol-position: left;
            width: 20px;
            border-right: 1px solid #444444;
            background-color: #333333;
            border-top-left-radius: 4px;
            border-bottom-left-radius: 4px;
        }
        
        QSpinBox#settingsSpin::down-button:hover {
            background-color: #FB7299;
        }
        
        QSpinBox#settingsSpin::down-arrow {
            image: none;
            border-left: 3px solid transparent;
            border-right: 3px solid transparent;
            border-top: 4px solid #CCCCCC;
            width: 0px;
            height: 0px;
        }
        
        QSpinBox#settingsSpin::up-button {
            subcontrol-origin: border;
            subcontrol-position: right;
            width: 20px;
            border-left: 1px solid #444444;
            background-color: #333333;
            border-top-right-radius: 4px;
            border-bottom-right-radius: 4px;
        }
        
        QSpinBox#settingsSpin::up-button:hover {
            background-color: #FB7299;
        }
        
        QSpinBox#settingsSpin::up-arrow {
            image: none;
            border-left: 3px solid transparent;
            border-right: 3px solid transparent;
            border-bottom: 4px solid #CCCCCC;
            width: 0px;
            height: 0px;
        }
        
        QPushButton#browseBtn {
            background-color: #333333;
            border: 2px solid #555555;
            border-radius: 6px;
            color: #CCCCCC;
            font-size: 12px;
            padding: 6px;
        }
        
        QPushButton#browseBtn:hover {
            background-color: #444444;
            border-color: #FB7299;
            color: #FFFFFF;
        }
    )");
}

void DownloadSettingsWidget::onQualityPresetChanged(int index)
{
    if (index < 0) return;

    QString preset = m_defaultQualityCombo->itemData(index).toString();

    if (m_qualityToFormatMap.contains(preset)) {
        AudioFormat format = m_qualityToFormatMap[preset];

        int formatIndex = m_defaultFormatCombo->findData(static_cast<int>(format));
        if (formatIndex >= 0) {
            m_defaultFormatCombo->setCurrentIndex(formatIndex);

            qDebug() << "🔄 音质变更联动：" << preset
                << "→" << m_defaultFormatCombo->currentText();
        }
    }
    else {
        qWarning() << "⚠️ 未找到音质预设对应的格式：" << preset;
    }
}

void DownloadSettingsWidget::loadSettings()
{
    AppConfig& config = AppConfig::instance();

    m_downloadPathInput->setText(config.getDownloadPath());

    QString qualityPreset = config.getDefaultQualityPreset();
    int qualityIndex = m_defaultQualityCombo->findData(qualityPreset);
    if (qualityIndex != -1) {
        m_defaultQualityCombo->setCurrentIndex(qualityIndex);
    }

    m_maxConcurrentDownloadsSpin->setValue(config.getMaxConcurrentDownloads());

    qDebug() << "✅ 设置已加载：音质=" << qualityPreset
        << "，格式=" << m_defaultFormatCombo->currentText();
}

bool DownloadSettingsWidget::validate()
{
    if (m_downloadPathInput->text().isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请选择下载路径");
        return false;
    }

    return true;
}

void DownloadSettingsWidget::applySettings()
{
    AppConfig& config = AppConfig::instance();

    config.setDownloadPath(m_downloadPathInput->text());
    config.setDefaultQualityPreset(m_defaultQualityCombo->currentData().toString());

    QString preset = m_defaultQualityCombo->currentData().toString();
    if (m_qualityToFormatMap.contains(preset)) {
        config.setDefaultAudioFormat(m_qualityToFormatMap[preset]);

        qDebug() << "✅ 设置已保存：音质=" << preset
            << "，格式=" << static_cast<int>(m_qualityToFormatMap[preset]);
    }

    config.setMaxConcurrentDownloads(m_maxConcurrentDownloadsSpin->value());
}

void DownloadSettingsWidget::onBrowseDownloadPathClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "选择下载目录",
        m_downloadPathInput->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dir.isEmpty()) {
        m_downloadPathInput->setText(dir);
    }
}
