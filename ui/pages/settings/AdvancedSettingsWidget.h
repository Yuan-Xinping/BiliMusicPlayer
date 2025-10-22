#pragma once
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>

class AdvancedSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit AdvancedSettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    bool validate();
    void applySettings();

private:
    void setupUI();
    void setupStyles();

    QCheckBox* m_proxyEnabledCheck = nullptr;
    QLineEdit* m_proxyUrlInput = nullptr;
};
