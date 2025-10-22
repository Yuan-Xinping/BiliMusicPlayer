#pragma once

#include <QMainWindow>
#include <QMouseEvent>
#include <QPushButton>
#include <QTimer>
#include "../themes/ThemeManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PlaybackBar;
class DownloadManagerPage;
class SettingsPage;
class BiliMusicPlayerApp;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setApp(BiliMusicPlayerApp* app);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onNavigationButtonClicked();
    void onThemeChanged(ThemeManager::Theme theme);

private:
    void setupTitleBar();
    void setupPlaybackBar();
    void setupContentPages();
    void setupStyles();
    void addShadowEffect();
    void switchToPage(int pageIndex);
    void loadThemeFromConfig();
    QString getEmbeddedStyle() const;

    Ui::MainWindow* ui;

    // UI 组件
    QPushButton* m_musicLibraryBtn;
    QPushButton* m_downloadManagerBtn;
    QPushButton* m_settingsBtn;
    PlaybackBar* m_playbackBar;
    DownloadManagerPage* m_downloadManagerPage;
    SettingsPage* m_settingsPage;

    // 应用实例
    BiliMusicPlayerApp* m_app = nullptr;

    // 其他成员
    int m_currentPageIndex;
    QPoint m_dragStartPosition;
    QTimer* m_progressTimer;
    int m_testPosition;
};
