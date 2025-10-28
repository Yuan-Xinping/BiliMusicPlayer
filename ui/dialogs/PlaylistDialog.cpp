// ui/dialogs/PlaylistDialog.cpp
#include "PlaylistDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include "../../service/PlaybackService.h"

static QString displayTitle(const Song& s) {
    QString t = s.getTitle();
    QString a = s.getArtist();
    return a.isEmpty() ? t : QString("%1 - %2").arg(t, a);
}

PlaylistDialog::PlaylistDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("正在播放");
    setWindowFlag(Qt::Tool);
    setModal(false);
    resize(520, 420);
    setupUI();
}

void PlaylistDialog::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto* plLabel = new QLabel("当前播放列表", this);
    m_playlistView = new QListWidget(this);
    m_playlistView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* qLabel = new QLabel("播放队列（下一首优先）", this);
    m_queueView = new QListWidget(this);
    m_queueView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_clearQueueBtn = new QPushButton("清空队列", this);

    auto* btns = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    root->addWidget(plLabel);
    root->addWidget(m_playlistView, 1);
    root->addWidget(qLabel);
    root->addWidget(m_queueView, 1);

    auto* h = new QHBoxLayout();
    h->addWidget(m_clearQueueBtn);
    h->addStretch(1);
    h->addWidget(btns);
    root->addLayout(h);

    // 双击播放：播放列表
    connect(m_playlistView, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        int row = m_playlistView->row(item);
        if (row >= 0 && row < m_playlist.size()) {
            PlaybackService::instance().playSong(m_playlist[row]);
        }
        });

    // 双击队列项：立即播放该项并从队列移除（等价于“下一首”指定）
    connect(m_queueView, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        int row = m_queueView->row(item);
        if (row >= 0 && row < m_queue.size()) {
            // 先移除到队首
            auto& ps = PlaybackService::instance();
            // 简化：清空并只保留该项在队首
            ps.clearPlaybackQueue();
            ps.addNextToQueue(m_queue[row]);
            ps.playNext();
        }
        });

    // 清空队列
    connect(m_clearQueueBtn, &QPushButton::clicked, this, []() {
        PlaybackService::instance().clearPlaybackQueue();
        });
}

void PlaylistDialog::setPlaylist(const QList<Song>& list, int currentIndex) {
    m_playlist = list;
    m_currentIndex = currentIndex;
    refreshPlaylist();
}

void PlaylistDialog::setQueue(const QList<Song>& queue) {
    m_queue = queue;
    refreshQueue();
}

void PlaylistDialog::refreshPlaylist() {
    m_playlistView->clear();
    for (int i = 0; i < m_playlist.size(); ++i) {
        QString text = QString::number(i + 1).rightJustified(2, ' ') + ". " + displayTitle(m_playlist[i]);
        auto* item = new QListWidgetItem(text);
        if (i == m_currentIndex) {
            item->setText("▶ " + item->text());
            item->setForeground(QColor("#FB7299"));
        }
        m_playlistView->addItem(item);
    }
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size())
        m_playlistView->setCurrentRow(m_currentIndex);
}

void PlaylistDialog::refreshQueue() {
    m_queueView->clear();
    for (int i = 0; i < m_queue.size(); ++i) {
        QString text = QString::number(i + 1).rightJustified(2, ' ') + ". " + displayTitle(m_queue[i]);
        m_queueView->addItem(new QListWidgetItem(text));
    }
}
