#pragma once
#include <QWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QTimer>

class AdvancedSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit AdvancedSettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    bool validate();
    void applySettings();

private slots:
	void onTestProxyClicked();

private:
    void setupUI();
    void setupStyles();

    QCheckBox* m_proxyEnabledCheck = nullptr;
    QLineEdit* m_proxyUrlInput = nullptr;
    QPushButton* m_testProxyBtn = nullptr;
};
