#include "PlaylistDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QSignalBlocker>
#include <QColor>
#include <QDebug>
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
    m_playlistView->setObjectName("playlistList");
    m_playlistView->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* qLabel = new QLabel("播放队列（下一首优先）", this);
    m_queueView = new QListWidget(this);
    m_queueView->setObjectName("queueList");
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
        if (m_updatingSelection) return; // 程序化更新时不触发
        int row = m_playlistView->row(item);
        if (row >= 0 && row < m_playlist.size()) {
            PlaybackService::instance().playSong(m_playlist[row]);
        }
        });

    // 双击队列项：立即播放该项并从队列移除（等价于“下一首”指定）
    connect(m_queueView, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (m_updatingSelection) return;
        int row = m_queueView->row(item);
        if (row >= 0 && row < m_queue.size()) {
            auto& ps = PlaybackService::instance();
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
    QSignalBlocker block(m_playlistView);
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
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        m_playlistView->setCurrentRow(m_currentIndex);
        if (auto* it = m_playlistView->item(m_currentIndex)) {
            m_playlistView->scrollToItem(it, QAbstractItemView::PositionAtCenter);
        }
    }
}

void PlaylistDialog::refreshQueue() {
    QSignalBlocker block(m_queueView);
    m_queueView->clear();
    for (int i = 0; i < m_queue.size(); ++i) {
        QString text = QString::number(i + 1).rightJustified(2, ' ') + ". " + displayTitle(m_queue[i]);
        m_queueView->addItem(new QListWidgetItem(text));
    }
}

void PlaylistDialog::setCurrentIndex(int row)
{
    if (!m_playlistView) return;
    int rc = m_playlistView->count();
    if (row < 0 || row >= rc) return;

    m_updatingSelection = true;

    // 去除旧行的箭头与高亮色
    if (m_currentIndex >= 0 && m_currentIndex < rc) {
        if (auto* oldItem = m_playlistView->item(m_currentIndex)) {
            QString t = oldItem->text();
            if (t.startsWith("▶ ")) t = t.mid(2);
            oldItem->setText(t);
            oldItem->setForeground(QBrush()); // 恢复默认前景色
        }
    }

    // 新行加箭头与主题色
    if (auto* newItem = m_playlistView->item(row)) {
        QString nt = newItem->text();
        if (!nt.startsWith("▶ ")) nt = "▶ " + nt;
        newItem->setText(nt);
        newItem->setForeground(QColor("#FB7299"));
    }

    // 选中并滚动到位（屏蔽信号避免联动）
    {
        QSignalBlocker b1(m_playlistView);
        m_playlistView->setCurrentRow(row, QItemSelectionModel::ClearAndSelect);
        if (auto* it = m_playlistView->item(row)) {
            m_playlistView->scrollToItem(it, QAbstractItemView::PositionAtCenter);
        }
    }

    m_currentIndex = row;
    m_updatingSelection = false;
}
