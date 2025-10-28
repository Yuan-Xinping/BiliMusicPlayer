// ui/components/Toast.cpp
#include "Toast.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

Toast::Toast(QWidget* parent) : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_ShowWithoutActivating);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 10, 16, 10);
    auto* label = new QLabel(this);
    label->setObjectName("toastLabel");
    label->setStyleSheet("color:#fff; font-size:13px;");
    layout->addWidget(label);

    setStyleSheet("Toast { background: rgba(0,0,0,0.75); border-radius:10px; }");

    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 160));
    setGraphicsEffect(shadow);
}

void Toast::showAtParentBottomRight(const QString& text, int durationMs) {
    auto* label = findChild<QLabel*>("toastLabel");
    if (label) label->setText(text);

    QWidget* p = parentWidget();
    if (!p) return;
    adjustSize();

    const int margin = 20;
    QPoint pos(p->width() - width() - margin, p->height() - height() - margin);
    move(p->mapToGlobal(pos));
    show();

    QTimer::singleShot(durationMs, this, [this]() { this->close(); this->deleteLater(); });
}

void Toast::showToast(QWidget* parent, const QString& text, int durationMs) {
    auto* t = new Toast(parent);
    t->showAtParentBottomRight(text, durationMs);
}
