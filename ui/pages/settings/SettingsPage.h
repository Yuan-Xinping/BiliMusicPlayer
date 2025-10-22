#pragma once
#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QHBoxLayout>

// 前向声明子页面
class DownloadSettingsWidget;
class DatabaseSettingsWidget;
class ToolsSettingsWidget;
class UISettingsWidget;
class AdvancedSettingsWidget;

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

signals:
    void settingsChanged();

private slots:
    void onCategorySelected(int index);
    void onSaveAllSettingsClicked();
    void onResetToDefaultsClicked();

private:
    void setupUI();
    void setupSidebar();
    void setupSubPages();
    void setupButtons();
    void setupStyles();
    void loadAllSettings();
    bool validateAllSettings();
    void applyAllSettings();

    QHBoxLayout* createButtonLayout();

    // 侧边栏
    QListWidget* m_categoryList = nullptr;

    // 子页面容器
    QStackedWidget* m_subPagesStack = nullptr;

    // 子页面
    DownloadSettingsWidget* m_downloadSettings = nullptr;
    DatabaseSettingsWidget* m_databaseSettings = nullptr;
    ToolsSettingsWidget* m_toolsSettings = nullptr;
    UISettingsWidget* m_uiSettings = nullptr;
    AdvancedSettingsWidget* m_advancedSettings = nullptr;

    // 底部按钮
    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_resetBtn = nullptr;
};
