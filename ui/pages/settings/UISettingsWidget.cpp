#include "UISettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>

UISettingsWidget::UISettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
}

void UISettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QGroupBox* uiGroup = new QGroupBox("🎨 界面设置");
    uiGroup->setObjectName("settingsGroup");
    QVBoxLayout* uiLayout = new QVBoxLayout(uiGroup);
    uiLayout->setSpacing(12);

    // 主题选择
    QHBoxLayout* themeLayout = new QHBoxLayout();
    QLabel* themeLabel = new QLabel("主题:");
    themeLabel->setFixedWidth(100);
    themeLabel->setObjectName("settingsLabel");

    m_themeCombo = new QComboBox();
    m_themeCombo->setObjectName("settingsCombo");
    m_themeCombo->addItem("🌙 深色主题", "dark");
    m_themeCombo->addItem("☀️ 浅色主题", "light");

    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(m_themeCombo);
    themeLayout->addStretch();

    // 字体大小
    QHBoxLayout* fontLayout = new QHBoxLayout();
    QLabel* fontLabel = new QLabel("字体大小:");
    fontLabel->setFixedWidth(100);
    fontLabel->setObjectName("settingsLabel");

    m_fontSizeSpin = new QSpinBox();
    m_fontSizeSpin->setObjectName("settingsSpin");
    m_fontSizeSpin->setRange(10, 20);
    m_fontSizeSpin->setValue(13);
    m_fontSizeSpin->setSuffix(" px");

    fontLayout->addWidget(fontLabel);
    fontLayout->addWidget(m_fontSizeSpin);
    fontLayout->addStretch();

    // 说明文本
    QLabel* infoLabel = new QLabel(
        "⚠️ 注意：\n"
        "• 主题切换功能暂不可用\n"
        "• 字体大小需要重启应用后生效"
    );
    infoLabel->setObjectName("infoLabel");
    infoLabel->setWordWrap(true);

    uiLayout->addLayout(themeLayout);
    uiLayout->addLayout(fontLayout);
    uiLayout->addSpacing(10);
    uiLayout->addWidget(infoLabel);

    mainLayout->addWidget(uiGroup);
    mainLayout->addStretch();
}

void UISettingsWidget::setupStyles()
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
        
        QLabel#infoLabel {
            color: #AAAAAA;
            font-size: 12px;
            padding: 10px;
            background-color: rgba(255, 255, 255, 0.02);
            border: 1px solid #333333;
            border-radius: 6px;
        }
        
        QComboBox#settingsCombo {
            padding: 6px;
            border: 2px solid #444444;
            border-radius: 6px;
            background-color: #252525;
            color: #FFFFFF;
            font-size: 12px;
        }
        
        QComboBox#settingsCombo::drop-down {
            border: none;
            width: 25px;
        }
        
        QComboBox#settingsCombo::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid #CCCCCC;
            margin-right: 8px;
        }
        
        QComboBox#settingsCombo QAbstractItemView {
            background-color: #2A2A2A;
            color: #FFFFFF;
            border: 2px solid #444444;
            selection-background-color: #FB7299;
        }
        
        QSpinBox#settingsSpin {
            padding: 6px;
            border: 2px solid #444444;
            border-radius: 6px;
            background-color: #252525;
            color: #FFFFFF;
            font-size: 12px;
        }
    )");
}

void UISettingsWidget::loadSettings()
{
    AppConfig& config = AppConfig::instance();

    QString theme = config.getTheme();
    int themeIndex = m_themeCombo->findData(theme);
    if (themeIndex >= 0) {
        m_themeCombo->setCurrentIndex(themeIndex);
    }

    m_fontSizeSpin->setValue(config.getFontSize());
}

bool UISettingsWidget::validate()
{
    return true;
}

void UISettingsWidget::applySettings()
{
    AppConfig& config = AppConfig::instance();

    config.setTheme(m_themeCombo->currentData().toString());
    config.setFontSize(m_fontSizeSpin->value());
}
