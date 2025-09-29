// src/components/HoverButton.h
#pragma once

#include <QPushButton>
#include <QPropertyAnimation>
#include <QEvent>
#include <QSize>

class HoverButton : public QPushButton
{
    Q_OBJECT
        Q_PROPERTY(QSize currentSize READ currentSize WRITE setCurrentSize)

public:
    explicit HoverButton(QWidget* parent = nullptr);
    explicit HoverButton(const QString& text, QWidget* parent = nullptr);

    void setHoverSize(const QSize& size);

    QSize currentSize() const;
    void setCurrentSize(const QSize& size);

protected:
    bool event(QEvent* event) override;

private:
    void init();

    QSize m_originalSize;
    QSize m_hoverSize;
    QPropertyAnimation* m_animation;
};
