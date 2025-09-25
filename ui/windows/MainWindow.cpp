#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QApplication>
#include <QScreen>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentPageIndex(0)
    , m_isMaximized(false)
    , m_isDragging(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置窗口属性
    setWindowTitle("BiliMusicPlayer - 哔哩哔哩音乐播放器");
    setWindowIcon(QIcon(":/icons/app_icon.png"));

    // 居中显示窗口
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    // 初始化各个组件
    setupNavigationBar();
    setupPlaybackBar();
    setupStyles();

    // 默认显示音乐库页面
    switchToPage(0);
    addShadowEffect();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        // 只在导航栏区域允许拖拽
        QWidget* childWidget = childAt(event->pos());
        if (childWidget && (childWidget == ui->navigationWidget ||
            childWidget->parent() == ui->navigationWidget)) {

            // 检查是否点击在窗口控制按钮上
            QPushButton* btn = qobject_cast<QPushButton*>(childWidget);
            if (btn && btn->objectName() == "windowButton") {
                return; // 如果是窗口按钮，不启动拖拽
            }

            m_isDragging = true;
            m_dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        if (!m_isMaximized) {
            move(event->globalPosition().toPoint() - m_dragStartPosition);
        }
        else {
            // 如果窗口是最大化状态，先恢复正常大小再移动
            showNormal();
            m_isMaximized = false;

            // 调整拖拽起始点，保持鼠标在标题栏的相对位置
            QPoint newPos = event->globalPosition().toPoint();
            m_dragStartPosition = QPoint(width() / 2, 30); // 假设标题栏中央
            move(newPos - m_dragStartPosition);
        }
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::setupNavigationBar()
{
    // 创建导航栏布局
    QHBoxLayout* navLayout = new QHBoxLayout(ui->navigationWidget);
    navLayout->setContentsMargins(30, 0, 30, 0);
    navLayout->setSpacing(0);

    // 左侧：应用标题和Logo
    QWidget* leftSection = new QWidget();
    QHBoxLayout* leftLayout = new QHBoxLayout(leftSection);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    QLabel* logoLabel = new QLabel("🎵");
    logoLabel->setObjectName("logoLabel");
    logoLabel->setFixedSize(32, 32);

    QLabel* titleLabel = new QLabel("BiliMusicPlayer");
    titleLabel->setObjectName("titleLabel");

    leftLayout->addWidget(logoLabel);
    leftLayout->addWidget(titleLabel);
    leftLayout->addStretch();

    // 中间：导航按钮区域
    QWidget* centerSection = new QWidget();
    QHBoxLayout* centerLayout = new QHBoxLayout(centerSection);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(8); // 按钮之间的间距

    // 创建导航按钮
    m_musicLibraryBtn = new QPushButton("🏠 音乐库");
    m_downloadManagerBtn = new QPushButton("📥 下载管理");
    m_settingsBtn = new QPushButton("⚙️ 设置");

    // 设置按钮属性
    QList<QPushButton*> buttons = { m_musicLibraryBtn, m_downloadManagerBtn, m_settingsBtn };
    for (QPushButton* btn : buttons) {
        btn->setFixedSize(140, 36); // 固定大小，更统一
        btn->setCheckable(true);
        btn->setObjectName("navigationButton");
        connect(btn, &QPushButton::clicked, this, &MainWindow::onNavigationButtonClicked);
    }

    centerLayout->addStretch();
    centerLayout->addWidget(m_musicLibraryBtn);
    centerLayout->addWidget(m_downloadManagerBtn);
    centerLayout->addWidget(m_settingsBtn);
    centerLayout->addStretch();

    // 右侧：功能按钮区域
    QWidget* rightSection = new QWidget();
    QHBoxLayout* rightLayout = new QHBoxLayout(rightSection);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    QPushButton* minimizeBtn = new QPushButton("─");
    QPushButton* maximizeBtn = new QPushButton("□");
    QPushButton* closeBtn = new QPushButton("✕");

    QList<QPushButton*> windowButtons = { minimizeBtn, maximizeBtn, closeBtn };
    for (QPushButton* btn : windowButtons) {
        btn->setFixedSize(32, 32);
        btn->setObjectName("windowButton");
    }

    closeBtn->setObjectName("closeButton"); // 特殊样式

    // 连接窗口控制信号
    connect(minimizeBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(maximizeBtn, &QPushButton::clicked, [this]() {
        if (m_isMaximized) {
            showNormal();
            if (!m_normalGeometry.isNull()) {
                setGeometry(m_normalGeometry);
            }
            m_isMaximized = false;
        }
        else {
            m_normalGeometry = geometry(); // 保存当前几何信息
            showMaximized();
            m_isMaximized = true;
        }
        });
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);

    rightLayout->addStretch();
    rightLayout->addWidget(minimizeBtn);
    rightLayout->addWidget(maximizeBtn);
    rightLayout->addWidget(closeBtn);

    // 添加到主布局
    navLayout->addWidget(leftSection, 1);      // 左侧占1份
    navLayout->addWidget(centerSection, 2);    // 中间占2份
    navLayout->addWidget(rightSection, 1);     // 右侧占1份

    // 默认选中音乐库
    m_musicLibraryBtn->setChecked(true);
}


void MainWindow::setupPlaybackBar()
{
    // 创建播放栏布局
    QHBoxLayout* playbackLayout = new QHBoxLayout(ui->playbackBarWidget);
    playbackLayout->setContentsMargins(20, 10, 20, 10);
    
    // 临时占位符
    playbackLayout->addWidget(new QLabel("🎵 播放控制栏 - 待实现", ui->playbackBarWidget));
}

void MainWindow::setupStyles()
{
    QString styleSheet = R"(
        QMainWindow {
            background-color: transparent;
            color: #FFFFFF;
            font-family: "Microsoft YaHei", "Segoe UI", Arial, sans-serif;
        }
        
        QMainWindow > QWidget#centralwidget {
            background-color: #1A1A1A;
            border-radius: 12px;
            border: 1px solid #333333;
        }
        
        #navigationWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2D2D2D, stop:1 #252525);
            border-bottom: 2px solid #FB7299;
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
        }
        
        #logoLabel {
            font-size: 20px;
            color: #FB7299;
            border: 2px solid #FB7299;
            border-radius: 16px;
            background: rgba(251, 114, 153, 0.1);
            qproperty-alignment: AlignCenter;
        }
        
        #titleLabel {
            font-size: 16px;
            font-weight: bold;
            color: #FFFFFF;
            margin: 0px;
        }
        
        QPushButton#navigationButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(255,255,255,0.08), stop:1 rgba(255,255,255,0.02));
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 18px;
            color: #CCCCCC;
            font-size: 13px;
            font-weight: 500;
            padding: 0px;
        }
        
        QPushButton#navigationButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(251,114,153,0.2), stop:1 rgba(251,114,153,0.1));
            border: 1px solid rgba(251,114,153,0.3);
            color: #FFFFFF;
        }
        
        QPushButton#navigationButton:checked {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FB7299, stop:1 #E85D85);
            border: 1px solid #FF8BB5;
            color: #FFFFFF;
            font-weight: bold;
        }
        
        QPushButton#navigationButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #E85D85, stop:1 #D4527A);
        }
        
        QPushButton#windowButton {
            background: transparent;
            border: none;
            border-radius: 16px;
            color: #AAAAAA;
            font-size: 14px;
            font-weight: bold;
        }
        
        QPushButton#windowButton:hover {
            background: rgba(255,255,255,0.1);
            color: #FFFFFF;
        }
        
        QPushButton#closeButton:hover {
            background: #E74C3C;
            color: #FFFFFF;
        }
        
        #playbackBarWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #252525, stop:1 #2D2D2D);
            border-top: 1px solid #333333;
            border-bottom-left-radius: 12px;
            border-bottom-right-radius: 12px;
        }
        
        QStackedWidget {
            background-color: #1A1A1A;
            border-left: 1px solid #333333;
            border-right: 1px solid #333333;
        }
        
        QLabel {
            color: #FFFFFF;
            font-size: 16px;
        }
    )";

    setStyleSheet(styleSheet);
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
    } else if (clickedButton == m_downloadManagerBtn) {
        switchToPage(1);
    } else if (clickedButton == m_settingsBtn) {
        switchToPage(2);
    }
}

void MainWindow::switchToPage(int pageIndex)
{
    if (pageIndex >= 0 && pageIndex < ui->contentStackedWidget->count()) {
        ui->contentStackedWidget->setCurrentIndex(pageIndex);
        m_currentPageIndex = pageIndex;
    }
}

void MainWindow::addShadowEffect()
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);           // 阴影模糊半径
    shadow->setColor(QColor(0, 0, 0, 120)); // 阴影颜色和透明度
    shadow->setOffset(0, 8);             // 阴影偏移量（水平，垂直）

    ui->centralwidget->setGraphicsEffect(shadow);
}
