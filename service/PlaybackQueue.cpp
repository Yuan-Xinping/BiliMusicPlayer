// service/PlaybackQueue.cpp
#include "PlaybackQueue.h"
#include <QDebug>

PlaybackQueue::PlaybackQueue(QObject* parent)
    : QObject(parent)
{
    qDebug() << "PlaybackQueue initialized";
}

void PlaybackQueue::enqueue(const Song& song) {
    if (song.getId().isEmpty()) {
        qWarning() << "PlaybackQueue: 尝试添加空歌曲到队列";
        return;
    }

    m_queue.enqueue(song);
    emit songEnqueued(song);
    emit queueChanged(getQueue());

    qDebug() << "PlaybackQueue: 歌曲入队:" << song.getTitle() << "队列大小:" << m_queue.size();
}

void PlaybackQueue::enqueueNext(const Song& song) {
    if (song.getId().isEmpty()) {
        qWarning() << "PlaybackQueue: 尝试添加空歌曲到队列头部";
        return;
    }

    // 在 Qt 的 QQueue 中，我们需要将所有元素取出，插入新歌曲，然后放回
    QList<Song> tempList;
    while (!m_queue.isEmpty()) {
        tempList.append(m_queue.dequeue());
    }

    // 新歌曲放在最前面
    m_queue.enqueue(song);
    for (const Song& s : tempList) {
        m_queue.enqueue(s);
    }

    emit songEnqueued(song);
    emit queueChanged(getQueue());

    qDebug() << "PlaybackQueue: 歌曲插入队列头部:" << song.getTitle();
}

void PlaybackQueue::enqueueList(const QList<Song>& songs) {
    for (const Song& song : songs) {
        if (!song.getId().isEmpty()) {
            m_queue.enqueue(song);
        }
    }

    emit queueChanged(getQueue());
    qDebug() << "PlaybackQueue: 批量入队" << songs.size() << "首歌曲，队列大小:" << m_queue.size();
}

Song PlaybackQueue::dequeue() {
    if (m_queue.isEmpty()) {
        return Song();
    }

    Song song = m_queue.dequeue();
    emit songDequeued(song);
    emit queueChanged(getQueue());

    qDebug() << "PlaybackQueue: 歌曲出队:" << song.getTitle() << "剩余队列大小:" << m_queue.size();
    return song;
}

void PlaybackQueue::clear() {
    m_queue.clear();
    emit queueChanged(getQueue());
    qDebug() << "PlaybackQueue: 清空队列";
}

QList<Song> PlaybackQueue::getQueue() const {
    QList<Song> result;
    QQueue<Song> tempQueue = m_queue;  // 复制队列
    while (!tempQueue.isEmpty()) {
        result.append(tempQueue.dequeue());
    }
    return result;
}

int PlaybackQueue::size() const {
    return m_queue.size();
}

bool PlaybackQueue::isEmpty() const {
    return m_queue.isEmpty();
}

Song PlaybackQueue::peek() const {
    if (m_queue.isEmpty()) {
        return Song();
    }
    return m_queue.head();
}

void PlaybackQueue::removeSong(int index) {
    QList<Song> songList = getQueue();
    if (index < 0 || index >= songList.size()) {
        qWarning() << "PlaybackQueue: 无效的移除索引:" << index;
        return;
    }

    QString title = songList[index].getTitle();
    songList.removeAt(index);

    // 重建队列
    m_queue.clear();
    for (const Song& song : songList) {
        m_queue.enqueue(song);
    }

    emit queueChanged(getQueue());
    qDebug() << "PlaybackQueue: 移除歌曲:" << title;
}

void PlaybackQueue::moveSong(int from, int to) {
    QList<Song> songList = getQueue();
    if (from < 0 || from >= songList.size() || to < 0 || to >= songList.size()) {
        qWarning() << "PlaybackQueue: 无效的移动索引:" << from << "to" << to;
        return;
    }

    if (from == to) {
        return;
    }

    Song song = songList.takeAt(from);
    songList.insert(to, song);

    // 重建队列
    m_queue.clear();
    for (const Song& s : songList) {
        m_queue.enqueue(s);
    }

    emit queueChanged(getQueue());
    qDebug() << "PlaybackQueue: 歌曲移动:" << song.getTitle() << "从" << from << "到" << to;
}

bool PlaybackQueue::contains(const Song& song) const {
    QList<Song> songList = getQueue();
    return songList.contains(song);
}
