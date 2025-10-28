// ui/dialogs/PlaylistDialog.h
#pragma once
#include <QDialog>
#include <QList>
#include "../../common/entities/Song.h"

class QListWidget;
class QPushButton;

class PlaylistDialog : public QDialog {
    Q_OBJECT
public:
    explicit PlaylistDialog(QWidget* parent = nullptr);
    void setPlaylist(const QList<Song>& list, int currentIndex);
    void setQueue(const QList<Song>& queue);

private:
    void setupUI();
    void refreshPlaylist();
    void refreshQueue();

    QListWidget* m_playlistView = nullptr;
    QListWidget* m_queueView = nullptr;
    QPushButton* m_clearQueueBtn = nullptr;

    QList<Song> m_playlist;
    QList<Song> m_queue;
    int m_currentIndex = -1;
};
