// src/components/HoverButton.cpp
#include "HoverButton.h"

HoverButton::HoverButton(QWidget* parent)
    : QPushButton(parent)
{
    init();
}

HoverButton::HoverButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    init();
}

void HoverButton::init()
{
    m_animation = new QPropertyAnimation(this, "currentSize", this);
    m_animation->setDuration(150);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
}

void HoverButton::setHoverSize(const QSize& size)
{
    m_hoverSize = size;
}

QSize HoverButton::currentSize() const
{
    return size();
}

void HoverButton::setCurrentSize(const QSize& size)
{
    setFixedSize(size);
}

bool HoverButton::event(QEvent* event)
{
    switch (event->type())
    {
    case QEvent::Show:
        if (m_originalSize.isEmpty()) {
            m_originalSize = this->size();
            if (m_hoverSize.isEmpty()) {
                m_hoverSize = m_originalSize * 1.1;
            }
        }
        break;

    case QEvent::Enter:
        if (!m_originalSize.isEmpty()) {
            m_animation->stop(); // 停止任何正在进行的动画
            m_animation->setStartValue(this->size());
            m_animation->setEndValue(m_hoverSize);
            m_animation->start();
        }
        break;

    case QEvent::Leave:
        if (!m_originalSize.isEmpty()) {
            m_animation->stop(); // 停止任何正在进行的动画
            m_animation->setStartValue(this->size());
            m_animation->setEndValue(m_originalSize);
            m_animation->start();
        }
        break;
    default:
        break;
    }
    return QPushButton::event(event);
}
