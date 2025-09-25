// service/PlaybackQueue.h
#pragma once
#include <QObject>
#include <QQueue>
#include "../common/entities/Song.h"

class PlaybackQueue : public QObject {
    Q_OBJECT

public:
    explicit PlaybackQueue(QObject* parent = nullptr);

    // 队列管理
    void enqueue(const Song& song);
    void enqueueNext(const Song& song);  // 添加到队列头部
    void enqueueList(const QList<Song>& songs);
    Song dequeue();
    void clear();

    // 队列查询
    QList<Song> getQueue() const;
    int size() const;
    bool isEmpty() const;
    Song peek() const;  // 查看下一首歌但不移除

    // 队列操作
    void removeSong(int index);
    void moveSong(int from, int to);
    bool contains(const Song& song) const;

signals:
    void queueChanged(const QList<Song>& queue);
    void songEnqueued(const Song& song);
    void songDequeued(const Song& song);

private:
    QQueue<Song> m_queue;
};
