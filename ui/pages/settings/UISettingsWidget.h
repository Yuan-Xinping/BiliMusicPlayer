#pragma once
#include <QWidget>
#include <QComboBox>
#include "../../themes/ThemeManager.h"

class UISettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit UISettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    bool validate();
    void applySettings();

private slots:
    void onThemeChanged(int index);

private:
    void setupUI();
    void setupStyles();

    QComboBox* m_themeCombo = nullptr;
};
