#include "AdvancedSettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

AdvancedSettingsWidget::AdvancedSettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();
}

void AdvancedSettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QGroupBox* advancedGroup = new QGroupBox("🔬 高级设置");
    advancedGroup->setObjectName("settingsGroup");
    QVBoxLayout* advancedLayout = new QVBoxLayout(advancedGroup);
    advancedLayout->setSpacing(12);

    // 代理设置
    m_proxyEnabledCheck = new QCheckBox("启用代理");
    m_proxyEnabledCheck->setObjectName("settingsCheckbox");

    QHBoxLayout* proxyLayout = new QHBoxLayout();
    QLabel* proxyLabel = new QLabel("代理地址:");
    proxyLabel->setFixedWidth(100);
    proxyLabel->setObjectName("settingsLabel");

    m_proxyUrlInput = new QLineEdit();
    m_proxyUrlInput->setObjectName("settingsInput");
    m_proxyUrlInput->setPlaceholderText("例如：http://127.0.0.1:7890");
    m_proxyUrlInput->setEnabled(false);

    proxyLayout->addWidget(proxyLabel);
    proxyLayout->addWidget(m_proxyUrlInput);

    // 说明文本
    QLabel* infoLabel = new QLabel(
        "💡 提示：\n"
        "• 启用代理后，下载将通过代理服务器进行\n"
        "• 代理地址格式：http://ip:port 或 socks5://ip:port\n"
        "• 如果代理设置错误可能导致下载失败"
    );
    infoLabel->setObjectName("infoLabel");
    infoLabel->setWordWrap(true);

    advancedLayout->addWidget(m_proxyEnabledCheck);
    advancedLayout->addLayout(proxyLayout);
    advancedLayout->addSpacing(10);
    advancedLayout->addWidget(infoLabel);

    mainLayout->addWidget(advancedGroup);
    mainLayout->addStretch();

    connect(m_proxyEnabledCheck, &QCheckBox::toggled,
        m_proxyUrlInput, &QLineEdit::setEnabled);
}

void AdvancedSettingsWidget::setupStyles()
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
        
        QLineEdit#settingsInput:disabled {
            background-color: #1A1A1A;
            color: #666666;
        }
        
        QCheckBox#settingsCheckbox {
            color: #CCCCCC;
            font-size: 12px;
        }
        
        QCheckBox#settingsCheckbox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #444444;
            border-radius: 4px;
            background-color: #252525;
        }
        
        QCheckBox#settingsCheckbox::indicator:checked {
            background-color: #FB7299;
            border-color: #FB7299;
        }
    )");
}

void AdvancedSettingsWidget::loadSettings()
{
    AppConfig& config = AppConfig::instance();

    m_proxyEnabledCheck->setChecked(config.getProxyEnabled());
    m_proxyUrlInput->setText(config.getProxyUrl());
}

bool AdvancedSettingsWidget::validate()
{
    if (m_proxyEnabledCheck->isChecked()) {
        QString proxyUrl = m_proxyUrlInput->text().trimmed();
        if (proxyUrl.isEmpty()) {
            QMessageBox::warning(this, "输入错误", "代理已启用但未填写地址！");
            return false;
        }
    }
    return true;
}

void AdvancedSettingsWidget::applySettings()
{
    AppConfig& config = AppConfig::instance();

    config.setProxyEnabled(m_proxyEnabledCheck->isChecked());
    config.setProxyUrl(m_proxyUrlInput->text());
}
