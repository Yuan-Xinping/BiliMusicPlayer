#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PlaybackBar;
class DownloadManagerPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onNavigationButtonClicked();

private:
    void setupTitleBar();
    void setupPlaybackBar();
    void setupContentPages();
    void setupStyles();
    void addShadowEffect();
    void switchToPage(int pageIndex);
    QString getEmbeddedStyle() const;

private:
    Ui::MainWindow* ui;

    // 导航按钮
    QPushButton* m_musicLibraryBtn = nullptr;
    QPushButton* m_downloadManagerBtn = nullptr;
    QPushButton* m_settingsBtn = nullptr;

    // 播放控制栏
    PlaybackBar* m_playbackBar = nullptr;

    // 内容页面
    DownloadManagerPage* m_downloadManagerPage = nullptr;

    // 测试定时器
    QTimer* m_progressTimer = nullptr;
    int m_testPosition = 0;

    // 当前页面索引
    int m_currentPageIndex = 0;

    // 拖拽相关
    QPoint m_dragStartPosition;
};
