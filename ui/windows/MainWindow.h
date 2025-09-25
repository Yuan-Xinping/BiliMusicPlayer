#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    // 重写鼠标事件处理窗口拖拽
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:
    void onNavigationButtonClicked();

private:
    void setupNavigationBar();
    void setupPlaybackBar();
    void setupStyles();
    void switchToPage(int pageIndex);
	void addShadowEffect();

private:
    Ui::MainWindow* ui;

    // 导航按钮
    QPushButton* m_musicLibraryBtn;
    QPushButton* m_downloadManagerBtn;
    QPushButton* m_settingsBtn;

    // 当前页面索引
    int m_currentPageIndex;
    bool m_isMaximized;

    // 窗口拖拽相关
    bool m_isDragging;
    QPoint m_dragStartPosition;
    QRect m_normalGeometry;
};
