#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "../pages/DownloadManagerPage.h"
#include "../pages/LibraryPage.h"
#include "../components/PlaybackBar.h"
#include "../../common/entities/Song.h"
#include "../../app/BiliMusicPlayerApp.h"
#include "../../service/DownloadService.h"
#include "../../viewmodel/DownloadViewModel.h"
#include "../../viewmodel/LibraryViewModel.h"
#include "../themes/ThemeManager.h"
#include "../../common/AppConfig.h"
#include "../pages/settings/SettingsPage.h"
#include "../../service/PlaybackService.h"
#include "../../common/PlaybackMode.h"
#include "../../common/PlaybackState.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QGraphicsDropShadowEffect>

namespace {
    // UI(0,1,2,3) -> Service PlaybackMode
    inline PlaybackMode toPlaybackMode(int uiMode) {
        switch (uiMode) {
        case 0: return PlaybackMode::Normal;     // 顺序
        case 1: return PlaybackMode::Shuffle;    // 随机
        case 2: return PlaybackMode::RepeatOne;  // 单曲
        case 3: return PlaybackMode::RepeatAll;  // 列表
        default: return PlaybackMode::Normal;
        }
    }
    // Service PlaybackMode -> UI(0,1,2,3)
    inline int toUiMode(PlaybackMode mode) {
        switch (mode) {
        case PlaybackMode::Normal:    return 0;
        case PlaybackMode::Shuffle:   return 1;
        case PlaybackMode::RepeatOne: return 2;
        case PlaybackMode::RepeatAll: return 3;
        default: return 0;
        }
    }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentPageIndex(0)
    , m_testPosition(0)
{
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(themes);

    ui->setupUi(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // 设置窗口属性
    setWindowTitle("BiliMusicPlayer - 哔哩哔哩音乐播放器");
    setMinimumSize(1000, 600);
    resize(1280, 800);

    // 居中显示窗口
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    loadThemeFromConfig();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
        this, &MainWindow::onThemeChanged);

    // 初始化各个组件
    setupTitleBar();
    setupPlaybackBar();
    addShadowEffect();

    // 默认显示音乐库页面
    switchToPage(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupPlaybackBar()
{
    // 创建播放栏组件
    m_playbackBar = new PlaybackBar(ui->playbackBarWidget);

    // 设置布局
    QVBoxLayout* layout = new QVBoxLayout(ui->playbackBarWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_playbackBar);

    ui->playbackBarWidget->setFixedHeight(90);

    // —— 连接 UI -> Service 控制 —— //
    PlaybackService* ps = &PlaybackService::instance();

    connect(m_playbackBar, &PlaybackBar::playPauseClicked, this, [ps]() {
        ps->togglePlayPause();
        });
    connect(m_playbackBar, &PlaybackBar::previousClicked, this, [ps]() {
        ps->playPrevious();
        });
    connect(m_playbackBar, &PlaybackBar::nextClicked, this, [ps]() {
        ps->playNext();
        });
    connect(m_playbackBar, &PlaybackBar::positionChanged, this, [ps](int sec) {
        ps->seek(static_cast<qint64>(sec) * 1000);
        });
    connect(m_playbackBar, &PlaybackBar::volumeChanged, this, [ps](int vol) {
        ps->setVolume(vol);
        });
    connect(m_playbackBar, &PlaybackBar::playModeChanged, this, [ps](int uiMode) {
        ps->setPlaybackMode(toPlaybackMode(uiMode));
        });

    // —— 连接 Service -> UI 状态 —— //
    connect(ps, &PlaybackService::playbackStateChanged, this, [this](PlaybackState st) {
        m_playbackBar->setPlaybackState(st == PlaybackState::Playing);
        });
    connect(ps, &PlaybackService::currentSongChanged, this, [this](const Song& s) {
        m_playbackBar->setSong(s);
        // 优先用元数据的总时长（秒），随后由 durationChanged 精确覆盖
        if (s.getDurationSeconds() > 0)
            m_playbackBar->setDuration(static_cast<int>(s.getDurationSeconds()));
        // 切歌重置进度显示
        m_playbackBar->setPosition(0);
        });
    connect(ps, &PlaybackService::positionChanged, this, [this](qint64 posMs) {
        m_playbackBar->setPosition(static_cast<int>(posMs / 1000));
        });
    connect(ps, &PlaybackService::durationChanged, this, [this](qint64 durMs) {
        m_playbackBar->setDuration(static_cast<int>(durMs / 1000));
        });
    connect(ps, &PlaybackService::volumeChanged, this, [this](int v) {
        m_playbackBar->setVolume(v);
        });
    connect(ps, &PlaybackService::playbackModeChanged, this, [this](PlaybackMode m) {
        m_playbackBar->setPlayMode(toUiMode(m));
        });
    connect(ps, &PlaybackService::error, this, [](const QString& err) {
        qWarning() << "Playback error:" << err;
        // 可进一步加 Toast 提示
        });

    // —— 初始同步（从服务拉取当前状态） —— //
    m_playbackBar->setVolume(ps->getVolume());
    m_playbackBar->setPlaybackState(ps->getPlaybackState() == PlaybackState::Playing);
    m_playbackBar->setPlayMode(toUiMode(ps->getPlaybackMode()));
    m_playbackBar->setPosition(static_cast<int>(ps->getCurrentPosition() / 1000));
    m_playbackBar->setDuration(static_cast<int>(ps->getDuration() / 1000));

    const Song cur = ps->getCurrentSong();
    if (!cur.getId().isEmpty()) {
        m_playbackBar->setSong(cur);
        if (cur.getDurationSeconds() > 0) {
            m_playbackBar->setDuration(static_cast<int>(cur.getDurationSeconds()));
        }
    }
}

void MainWindow::setupStyles()
{
    QString currentStyle = styleSheet();
    if (!currentStyle.isEmpty()) {
        qDebug() << "✅ 主题样式已应用（长度：" << currentStyle.length() << "字符）";
    }
    else {
        qWarning() << "⚠️ 样式表为空，可能主题加载失败";
    }
}

QString MainWindow::getEmbeddedStyle() const
{
    return R"CSS(
/* BiliMusicPlayer 主样式表*/

/* 全局样式 */
* {
    font-family: "Microsoft YaHei", "PingFang SC", "Helvetica Neue", Arial, sans-serif;
}

QMainWindow {
    background-color: #1A1A1A;
    color: #FFFFFF;
    border: 1px solid #444444;
    border-radius: 16px;
}

/* 窗口控制按钮 */
QPushButton#windowControlButton {
    background-color: transparent;
    border: none;
    color: #AAAAAA;
    font-size: 14px;
    border-radius: 8px;
    margin: 0px 2px;
    min-width: 35px;
    min-height: 35px;
    max-width: 35px;
    max-height: 35px;
}

QPushButton#windowControlButton:hover {
    background-color: rgba(255, 255, 255, 0.1);
    color: #FFFFFF;
}

QPushButton#windowControlButton:pressed {
    background-color: rgba(255, 255, 255, 0.2);
}

/* 导航按钮 */
QPushButton#navigationButton {
    background-color: transparent;
    border: none;
    color: #AAAAAA;
    font-size: 14px;
    font-weight: bold;
    padding: 6px 16px;
    margin: 0px 1px;
    border-radius: 12px;
    min-width: 100px;
}

QPushButton#navigationButton:hover {
    background-color: rgba(251, 114, 153, 0.1);
    color: #FFFFFF;
}

QPushButton#navigationButton:checked {
    background-color: #FB7299;
    color: #FFFFFF;
}

QPushButton#navigationButton:pressed {
    background-color: #E85D85;
}

/* 圆形播放控制按钮 */
QPushButton#controlButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(255,255,255,0.12),
        stop:1 rgba(255,255,255,0.08));
    border: 1px solid rgba(255,255,255,0.2);
    border-radius: 20px;
    color: #DDDDDD;
    font-size: 16px;
    font-weight: bold;
    min-width: 40px;
    min-height: 40px;
    max-width: 40px;
    max-height: 40px;
}

QPushButton#controlButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(251,114,153,0.25),
        stop:1 rgba(251,114,153,0.15));
    border: 1px solid rgba(251,114,153,0.4);
    color: #FFFFFF;
}

QPushButton#controlButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(251,114,153,0.35),
        stop:1 rgba(251,114,153,0.25));
}

/* 主播放按钮 */
QPushButton#playPauseButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #FF8BB5, stop:0.5 #FB7299, stop:1 #E85D85);
    border: 2px solid rgba(255, 255, 255, 0.3);
    border-radius: 26px;
    color: #FFFFFF;
    font-size: 20px;
    font-weight: bold;
    min-width: 52px;
    min-height: 52px;
    max-width: 52px;
    max-height: 52px;
}

QPushButton#playPauseButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #FFB3D1, stop:0.5 #FF8BB5, stop:1 #FB7299);
    border: 2px solid rgba(255, 255, 255, 0.5);
}

QPushButton#playPauseButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #E85D85, stop:0.5 #D4527A, stop:1 #C44A75);
}

/* 统一的模式切换按钮 */
QPushButton#modeButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(255,255,255,0.08),
        stop:1 rgba(255,255,255,0.04));
    border: 1px solid rgba(255,255,255,0.15);
    border-radius: 16px;
    color: #CCCCCC;
    font-size: 11px;
    font-weight: 500;
    padding: 4px 8px;
    min-width: 80px;
    min-height: 32px;
    max-height: 32px;
}

QPushButton#modeButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(251,114,153,0.15),
        stop:1 rgba(251,114,153,0.08));
    color: #FFFFFF;
    border: 1px solid rgba(251,114,153,0.3);
}

QPushButton#modeButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(251,114,153,0.25),
        stop:1 rgba(251,114,153,0.15));
}

/* 音量和播放列表按钮 */
QPushButton#volumeButton,
QPushButton#playlistButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(255,255,255,0.06),
        stop:1 rgba(255,255,255,0.02));
    border: 1px solid rgba(255,255,255,0.1);
    color: #BBBBBB;
    font-size: 16px;
    border-radius: 18px;
    min-width: 36px;
    min-height: 36px;
    max-width: 36px;
    max-height: 36px;
}

QPushButton#volumeButton:hover,
QPushButton#playlistButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(251,114,153,0.12),
        stop:1 rgba(251,114,153,0.06));
    color: #FFFFFF;
    border: 1px solid rgba(251,114,153,0.2);
}

/* 增强进度条样式 */
QSlider#positionSlider {
    max-height: 20px;
}

QSlider#positionSlider::groove:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #404040, stop:1 #2A2A2A);
    height: 5px;
    border-radius: 2px;
    border: 1px solid rgba(255,255,255,0.05);
}

QSlider#positionSlider::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #FF8BB5, stop:1 #FB7299);
    width: 18px;
    height: 18px;
    border-radius: 9px;
    margin: -7px 0;
    border: 2px solid #FFFFFF;
}

QSlider#positionSlider::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #FFB3D1, stop:1 #FF8BB5);
}

QSlider#positionSlider::sub-page:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #FB7299, stop:1 #FF8BB5);
    border-radius: 2px;
}

/* 音量滑块样式 */
QSlider#volumeSlider {
    max-width: 90px;
    max-height: 16px;
}

QSlider#volumeSlider::groove:horizontal {
    background: rgba(255,255,255,0.1);
    height: 4px;
    border-radius: 2px;
}

QSlider#volumeSlider::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #DDDDDD, stop:1 #BBBBBB);
    width: 14px;
    height: 14px;
    border-radius: 7px;
    margin: -5px 0;
    border: 1px solid rgba(255,255,255,0.3);
}

QSlider#volumeSlider::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #FFFFFF, stop:1 #DDDDDD);
}

QSlider#volumeSlider::sub-page:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #AAAAAA, stop:1 #CCCCCC);
    border-radius: 2px;
}

/* 标签样式 */
#songTitle {
    color: #FFFFFF;
    font-size: 16px;
    font-weight: 600;
    margin: 0px;
    padding: 0px;
}

#artistName {
    color: #B8B8B8;
    font-size: 13px;
    font-weight: 400;
    margin: 0px;
    padding: 0px;
}

#timeLabel {
    color: #B8B8B8;
    font-size: 12px;
    font-weight: 500;
    font-family: "Consolas", "Monaco", "Menlo", monospace;
    background: rgba(255,255,255,0.02);
    border: 1px solid rgba(255,255,255,0.05);
    border-radius: 8px;
    padding: 2px 6px;
}

/* 标题栏样式 */
#titleBarWidget {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 #2A2A2A, stop:1 #252525);
    border-bottom: 2px solid #FB7299;
    border-top-left-radius: 16px;
    border-top-right-radius: 16px;
    min-height: 55px;
    max-height: 55px;
}

/* 播放栏样式 */
#playbackBarWidget {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(42, 42, 42, 0.95),
        stop:1 rgba(35, 35, 35, 0.95));
    border-top: 1px solid rgba(255, 255, 255, 0.1);
    border-bottom-left-radius: 16px;
    border-bottom-right-radius: 16px;
    min-height: 90px;
    max-height: 90px;
}

/* PlaybackBar组件样式 */
PlaybackBar {
    background: transparent;
    max-height: 90px;
    min-height: 90px;
}

/* 内容区域样式 */
QStackedWidget {
    background-color: #1A1A1A;
    border: none;
    margin: 0px 1px;
    border-radius: 0px;
}

/* 中央区域样式 */
QWidget#centralwidget {
    background-color: #1A1A1A;
    border: none;
    margin: 0px;
    padding: 0px;
    border-radius: 16px;
}

/* 内容页面占位符样式 */
QLabel[objectName*="Placeholder"] {
    color: #666666;
    font-size: 18px;
    background-color: transparent;
}

/* 滚动条样式 */
QScrollBar:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #2A2A2A, stop:1 #252525);
    width: 12px;
    border-radius: 6px;
}

QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #555555, stop:1 #444444);
    border-radius: 6px;
    min-height: 20px;
    border: 1px solid rgba(255,255,255,0.1);
}

QScrollBar::handle:vertical:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #FB7299, stop:1 #E85D85);
}

/* 列表视图样式 */
QListView {
    background-color: #1A1A1A;
    color: #FFFFFF;
    border: none;
    selection-background-color: rgba(251, 114, 153, 0.3);
}

QListView::item {
    padding: 8px;
    border-bottom: 1px solid #333333;
    border-radius: 8px;
    margin: 2px;
}

QListView::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(255, 255, 255, 0.08),
        stop:1 rgba(255, 255, 255, 0.04));
}

QListView::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
        stop:0 rgba(251, 114, 153, 0.25),
        stop:1 rgba(251, 114, 153, 0.15));
}
    )CSS";
}

void MainWindow::addShadowEffect()
{
    // 为主窗口添加阴影效果
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);           // 阴影模糊半径
    shadow->setColor(QColor(0, 0, 0, 120)); // 阴影颜色和透明度
    shadow->setOffset(0, 8);             // 阴影偏移量（水平，垂直）

    // 应用到中央部件
    ui->centralwidget->setGraphicsEffect(shadow);
}

void MainWindow::onNavigationButtonClicked()
{
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;

    // 取消所有按钮的选中状态
    m_musicLibraryBtn->setChecked(false);
    m_downloadManagerBtn->setChecked(false);
    m_settingsBtn->setChecked(false);

    // 设置当前按钮为选中状态并切换页面
    clickedButton->setChecked(true);

    if (clickedButton == m_musicLibraryBtn) {
        switchToPage(0);
    }
    else if (clickedButton == m_downloadManagerBtn) {
        switchToPage(1);
    }
    else if (clickedButton == m_settingsBtn) {
        switchToPage(2);
    }
}

void MainWindow::switchToPage(int pageIndex)
{
    if (pageIndex >= 0 && pageIndex < ui->contentStackedWidget->count()) {
        ui->contentStackedWidget->setCurrentIndex(pageIndex);
        m_currentPageIndex = pageIndex;

        QString pageName;
        switch (pageIndex) {
        case 0: pageName = "音乐库"; break;
        case 1: pageName = "下载管理"; break;
        case 2: pageName = "设置"; break;
        }
        qDebug() << "切换到页面：" << pageName;
    }
}

void MainWindow::setupTitleBar()
{
    // 创建标题栏布局
    QHBoxLayout* titleLayout = new QHBoxLayout(ui->titleBarWidget);
    titleLayout->setContentsMargins(20, 10, 20, 10);
    titleLayout->setSpacing(0);

    // 左侧：应用标题
    QLabel* appTitle = new QLabel("🎵 BiliMusicPlayer");
    appTitle->setStyleSheet("color: #FB7299; font-size: 16px; font-weight: bold;");

    // 中间：导航按钮区域
    QWidget* navButtonsWidget = new QWidget();
    QHBoxLayout* navLayout = new QHBoxLayout(navButtonsWidget);
    navLayout->setContentsMargins(0, 0, 0, 0);
    navLayout->setSpacing(0);

    // 创建导航按钮
    m_musicLibraryBtn = new QPushButton("🏠 音乐库");
    m_downloadManagerBtn = new QPushButton("📥 下载管理");
    m_settingsBtn = new QPushButton("⚙️ 设置");

    // 设置按钮属性
    QList<QPushButton*> buttons = { m_musicLibraryBtn, m_downloadManagerBtn, m_settingsBtn };
    for (QPushButton* btn : buttons) {
        btn->setFixedHeight(35);
        btn->setMinimumWidth(120);
        btn->setCheckable(true);
        btn->setObjectName("navigationButton");
        connect(btn, &QPushButton::clicked, this, &MainWindow::onNavigationButtonClicked);
    }

    navLayout->addWidget(m_musicLibraryBtn);
    navLayout->addWidget(m_downloadManagerBtn);
    navLayout->addWidget(m_settingsBtn);

    // 右侧：窗口控制按钮
    QPushButton* minimizeBtn = new QPushButton("-");
    QPushButton* maximizeBtn = new QPushButton("🗖");
    QPushButton* closeBtn = new QPushButton("🗙");

    QList<QPushButton*> windowButtons = { minimizeBtn, maximizeBtn, closeBtn };
    for (QPushButton* btn : windowButtons) {
        btn->setFixedSize(35, 35);
        btn->setObjectName("windowControlButton");
    }

    // 连接窗口控制按钮信号
    connect(minimizeBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(maximizeBtn, &QPushButton::clicked, [this]() {
        if (isMaximized()) {
            showNormal();
        }
        else {
            showMaximized();
        }
        });
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    // 添加到标题栏布局
    titleLayout->addWidget(appTitle);
    titleLayout->addStretch(1);
    titleLayout->addWidget(navButtonsWidget);
    titleLayout->addStretch(1);
    titleLayout->addWidget(minimizeBtn);
    titleLayout->addWidget(maximizeBtn);
    titleLayout->addWidget(closeBtn);

    // 默认选中音乐库
    m_musicLibraryBtn->setChecked(true);

    //添加拖拽功能
    ui->titleBarWidget->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->titleBarWidget) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragStartPosition = mouseEvent->globalPos() - frameGeometry().topLeft();
                return true;
            }
        }
        else if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                move(mouseEvent->globalPos() - m_dragStartPosition);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setupContentPages()
{
    qDebug() << "🔧 开始初始化内容页面...";

    if (!m_app) {
        qCritical() << "❌ 无法创建页面：BiliMusicPlayerApp 未设置";
        return;
    }

    // ===== 下载管理（保持不变） =====
    DownloadService* downloadService = m_app->getDownloadService();
    if (!downloadService) {
        qCritical() << "❌ 无法创建页面：DownloadService 为空";
        return;
    }
    m_downloadViewModel = new DownloadViewModel(downloadService, this);
    m_downloadManagerPage = new DownloadManagerPage(m_downloadViewModel, this);

    QWidget* oldDownloadPage = ui->contentStackedWidget->widget(1);
    ui->contentStackedWidget->removeWidget(oldDownloadPage);
    if (oldDownloadPage) oldDownloadPage->deleteLater();
    ui->contentStackedWidget->insertWidget(1, m_downloadManagerPage);

    // ===== 设置（保持不变） =====
    m_settingsPage = new SettingsPage(this);
    QWidget* oldSettingsPage = ui->contentStackedWidget->widget(2);
    ui->contentStackedWidget->removeWidget(oldSettingsPage);
    if (oldSettingsPage) oldSettingsPage->deleteLater();
    ui->contentStackedWidget->insertWidget(2, m_settingsPage);

    connect(m_settingsPage, &SettingsPage::settingsChanged,
        m_downloadManagerPage, &DownloadManagerPage::onSettingsChanged);
    connect(m_settingsPage, &SettingsPage::settingsChanged,
        downloadService, &DownloadService::refreshConfig);

    // ===== 🎵 音乐库 =====
    {
        LibraryViewModel* libraryVM = &LibraryViewModel::instance();
        m_libraryPage = new LibraryPage(libraryVM, this);

        QWidget* oldLibraryPage = ui->contentStackedWidget->widget(0);
        ui->contentStackedWidget->removeWidget(oldLibraryPage);
        if (oldLibraryPage) oldLibraryPage->deleteLater();
        ui->contentStackedWidget->insertWidget(0, m_libraryPage);

        // 双击播放 -> 调用服务播放（UI 状态由服务信号驱动）
        connect(m_libraryPage, &LibraryPage::requestPlaySongs, this,
            [](const QList<Song>& list, int index) {
                if (list.isEmpty() || index < 0 || index >= list.size()) return;
                PlaybackService::instance().playPlaylist(list, index);
            });
    }

    //  关键：显式切回“音乐库”页，修复启动落在下载页的问题
    ui->contentStackedWidget->setCurrentIndex(0);
    if (m_musicLibraryBtn && m_downloadManagerBtn && m_settingsBtn) {
        m_musicLibraryBtn->setChecked(true);
        m_downloadManagerBtn->setChecked(false);
        m_settingsBtn->setChecked(false);
    }

    qDebug() << "✅ 所有页面已集成完成";
    qDebug() << "   内容页面总数:" << ui->contentStackedWidget->count();
}

void MainWindow::setApp(BiliMusicPlayerApp* app)
{
    m_app = app;
    qDebug() << "✅ MainWindow: 应用实例已设置";

    setupContentPages();
}

void MainWindow::loadThemeFromConfig()
{
    AppConfig& config = AppConfig::instance();
    QString themeName = config.getTheme();

    qDebug() << "📋 从配置加载主题：" << themeName;

    if (ThemeManager::instance().loadTheme(themeName)) {
        qDebug() << "✅ 主题加载成功";
        ThemeManager::instance().applyToWidget(this);
    }
    else {
        qWarning() << "❌ 主题加载失败，使用内嵌样式";
        QString fallbackStyle = getEmbeddedStyle();
        setStyleSheet(fallbackStyle);
    }
}

void MainWindow::onThemeChanged(ThemeManager::Theme theme)
{
    Q_UNUSED(theme);

    qDebug() << "🎨 主题已切换：" << ThemeManager::instance().currentThemeName();

    ThemeManager::instance().applyToWidget(this);

    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();

    QList<QWidget*> allWidgets = this->findChildren<QWidget*>();
    for (QWidget* widget : allWidgets) {
        if (widget) {
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }
    }

    if (m_downloadManagerPage) {
        m_downloadManagerPage->style()->unpolish(m_downloadManagerPage);
        m_downloadManagerPage->style()->polish(m_downloadManagerPage);
        m_downloadManagerPage->update();
        qDebug() << "  - 下载管理页面已刷新";
    }
    if (m_settingsPage) {
        m_settingsPage->style()->unpolish(m_settingsPage);
        m_settingsPage->style()->polish(m_settingsPage);
        m_settingsPage->update();
        qDebug() << "  - 设置页面已刷新";
    }
    if (m_playbackBar) {
        m_playbackBar->style()->unpolish(m_playbackBar);
        m_playbackBar->style()->polish(m_playbackBar);
        m_playbackBar->update();
        qDebug() << "  - 播放栏已刷新";
    }
    if (m_libraryPage) {
        m_libraryPage->style()->unpolish(m_libraryPage);
        m_libraryPage->style()->polish(m_libraryPage);
        m_libraryPage->update();
        qDebug() << "  - 音乐库页面已刷新";
    }



    qDebug() << "✅ 所有组件已更新主题";
}