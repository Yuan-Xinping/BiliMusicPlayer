#pragma once
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>

class UISettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit UISettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    bool validate();
    void applySettings();

private:
    void setupUI();
    void setupStyles();

    QComboBox* m_themeCombo = nullptr;
    QSpinBox* m_fontSizeSpin = nullptr;
};
