#pragma once
#include <QObject>

class PlaybackService : public QObject {
    Q_OBJECT
public:
    explicit PlaybackService(QObject* parent = nullptr);
    static PlaybackService& instance();
};
