// ui/components/Toast.h
#pragma once
#include <QWidget>
#include <QString>

class Toast : public QWidget {
    Q_OBJECT
public:
    explicit Toast(QWidget* parent = nullptr);
    static void showToast(QWidget* parent, const QString& text, int durationMs = 2000);

private:
    void showAtParentBottomRight(const QString& text, int durationMs);
};
