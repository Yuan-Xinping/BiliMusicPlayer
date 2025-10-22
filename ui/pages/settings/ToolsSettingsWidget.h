#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ToolsSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ToolsSettingsWidget(QWidget* parent = nullptr);

    void loadSettings();
    bool validate();
    void applySettings();

private slots:
    void onBrowseYtDlpPathClicked();
    void onBrowseFfmpegPathClicked();
	void onTestAllToolsClicked();

private:
    void setupUI();
    void setupStyles();
    void testYtDlpPath(const QString& path);
    void testFfmpegPath(const QString& path);

    QLineEdit* m_ytDlpPathInput = nullptr;
    QPushButton* m_browseYtDlpBtn = nullptr;
    QLabel* m_ytDlpStatusLabel = nullptr;

    QLineEdit* m_ffmpegPathInput = nullptr;
    QPushButton* m_browseFfmpegBtn = nullptr;
    QLabel* m_ffmpegStatusLabel = nullptr;

	QPushButton* m_testAllBtn = nullptr;
};
