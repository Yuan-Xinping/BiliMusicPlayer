#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include "../../../infra/DownloadConfig.h"

class DownloadSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit DownloadSettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    bool validate();
    void applySettings();

private slots:
    void onBrowseDownloadPathClicked();
    void onQualityPresetChanged(int index);

private:
    void setupUI();
    void setupStyles();
    void setupQualityFormatMapping();

    QLineEdit* m_downloadPathInput = nullptr;
    QPushButton* m_browseDownloadPathBtn = nullptr;
    QComboBox* m_defaultQualityCombo = nullptr;
    QComboBox* m_defaultFormatCombo = nullptr;
    QSpinBox* m_maxConcurrentDownloadsSpin = nullptr;

    QMap<QString, AudioFormat> m_qualityToFormatMap;

};
