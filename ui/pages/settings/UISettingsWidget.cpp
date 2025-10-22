#include "UISettingsWidget.h"
#include "../../../common/AppConfig.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

UISettingsWidget::UISettingsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    setupStyles();

    // 连接主题切换信号
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &UISettingsWidget::onThemeChanged);
}

void UISettingsWidget::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(16);

    QGroupBox* uiGroup = new QGroupBox("🎨 界面设置");
    uiGroup->setObjectName("settingsGroup");
    QVBoxLayout* uiLayout = new QVBoxLayout(uiGroup);

    // 主题选择
    QHBoxLayout* themeLayout = new QHBoxLayout();
    QLabel* themeLabel = new QLabel("主题:");
    themeLabel->setFixedWidth(100);
    themeLabel->setObjectName("settingsLabel");

    m_themeCombo = new QComboBox();
    m_themeCombo->setObjectName("settingsCombo");
    m_themeCombo->addItem("🌙 深色主题", static_cast<int>(ThemeManager::Theme::Dark));
    m_themeCombo->addItem("☀️ 浅色主题", static_cast<int>(ThemeManager::Theme::Light));

    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(m_themeCombo);
    themeLayout->addStretch();

    // 说明文本
    QLabel* infoLabel = new QLabel(
        "💡 提示：\n"
        "• 主题切换会立即生效\n"
        "• 建议在切换后保存设置"
    );
    infoLabel->setObjectName("infoLabel");
    infoLabel->setWordWrap(true);

    uiLayout->addLayout(themeLayout);
    uiLayout->addSpacing(10);
    uiLayout->addWidget(infoLabel);

    mainLayout->addWidget(uiGroup);
    mainLayout->addStretch();
}

void UISettingsWidget::setupStyles()
{
}

void UISettingsWidget::onThemeChanged(int index) {
    ThemeManager::Theme theme = static_cast<ThemeManager::Theme>(
        m_themeCombo->itemData(index).toInt()
        );

    if (ThemeManager::instance().loadTheme(theme)) {
        qDebug() << "✅ 主题已切换：" << ThemeManager::instance().currentThemeName();
    }
    else {
        QMessageBox::warning(this, "错误", "主题切换失败！");
    }
}

void UISettingsWidget::loadSettings()
{
    QString theme = AppConfig::instance().getTheme();
    qDebug() << "🔍 当前配置的主题：" << theme;

    ThemeManager::Theme themeEnum = (theme == "light") ?
        ThemeManager::Theme::Light : ThemeManager::Theme::Dark;

    qDebug() << "🔍 转换后的枚举值：" << static_cast<int>(themeEnum);

    int index = m_themeCombo->findData(static_cast<int>(themeEnum));
    qDebug() << "🔍 下拉框选中索引：" << index;

    if (index >= 0) {
        m_themeCombo->blockSignals(true);
        m_themeCombo->setCurrentIndex(index);
        m_themeCombo->blockSignals(false);
    }
}

bool UISettingsWidget::validate()
{
    return true;
}

void UISettingsWidget::applySettings()
{
    int themeInt = m_themeCombo->currentData().toInt();
    ThemeManager::Theme theme = static_cast<ThemeManager::Theme>(themeInt);

    QString themeName = (theme == ThemeManager::Theme::Dark) ? "dark" : "light";
    AppConfig::instance().setTheme(themeName);
}
