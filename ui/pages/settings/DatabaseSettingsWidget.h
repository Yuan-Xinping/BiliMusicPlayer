#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "../../../infra/DownloadConfig.h"

class DatabaseSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit DatabaseSettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    bool validate();
    void applySettings();

private slots:
    void onBrowseDatabasePathClicked();
    void onClearDatabaseClicked();

private:
    void setupUI();
    void setupStyles();

    QLineEdit* m_databasePathInput = nullptr;
    QPushButton* m_browseDatabasePathBtn = nullptr;
    QPushButton* m_clearDatabaseBtn = nullptr;
    QLabel* m_databaseInfoLabel = nullptr;
};
