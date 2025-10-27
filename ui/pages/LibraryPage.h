#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QListWidget>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QPoint>
#include <QShortcut>
#include <QTimer>
#include <QStringList>

#include "../../common/entities/Song.h"
#include "../../common/entities/Playlist.h"
#include "../../viewmodel/LibraryViewModel.h"

class QListWidgetItem;
class QGraphicsOpacityEffect; 
class QPropertyAnimation; 
class QResizeEvent; 

/**
 * 音乐库页面（左侧歌单 + 右侧歌曲表）
 */
class LibraryPage : public QWidget {
    Q_OBJECT
public:
    explicit LibraryPage(LibraryViewModel* viewModel, QWidget* parent = nullptr);

signals:
    void requestPlaySongs(const QList<Song>& playlist, int startIndex);

private slots:
    // 左侧：点击/右键
    void onSidebarItemClicked(QListWidgetItem* item);
    void onSidebarContextMenuRequested(const QPoint& pos);

    // 右侧：搜索/双击/右键（搜索采用防抖）
    void onSearchTextChanged(const QString& text); // 只负责启动定时器
    void onSearchTimeout();                        // 真正执行搜索
    void onSongCellDoubleClicked(int row, int column);
    void onSongTableContextMenuRequested(const QPoint& pos);

    // 数据变更 -> 刷新视图
    void onSongsChanged();
    void onPlaylistsChanged();

    // 侧边栏拖拽排序后回调（持久化顺序）
    void onSidebarOrderChanged();

private:
    // UI
    void setupUI();
    void setupConnections();

    // 左侧歌单
    void reloadPlaylists();
    void selectMyMusic();
    QString currentPlaylistId() const;
    bool inPlaylistMode() const;

    // 右侧歌曲表
    void reloadSongs();
    void loadSongs(const QList<Song>& songs);
    QString formatDuration(qlonglong seconds) const;
    QString humanizeDuration(qlonglong seconds) const; // 友好显示总时长
    void updateHeaderText();

    // 空状态（Empty State）
    void updateEmptyState();

    // 轻提示（Toast）
    void showToast(const QString& text);

    // 歌曲右键菜单动作（单项）
    void actAddToPlaylist(const QString& songId);
    void actRemoveFromCurrentPlaylist(const QString& songId);
    void actEditSongMeta(const Song& song);
    void actDeleteSong(const QString& songId);

    // 侧边栏右键菜单动作
    void actCreatePlaylist(); // 由按钮触发
    void actRenamePlaylist(const QString& playlistId, const QString& oldName);
    void actDeletePlaylist(const QString& playlistId, const QString& name);
    void actExportPlaylist(const QString& playlistId, const QString& name);
    void actImportPlaylist(); // （可选）按钮触发

    // 工具
    Playlist findPlaylistById(const QString& id) const;

protected:
    // 确保空状态层大小随表格变动
    void resizeEvent(QResizeEvent* e) override;

private:
    LibraryViewModel* m_viewModel = nullptr;

    // 左侧
    QWidget* m_sidebarPanel = nullptr;
    QPushButton* m_btnCreate = nullptr;
    QPushButton* m_btnImport = nullptr;
    QListWidget* m_sidebar = nullptr;

    // 右侧
    QLineEdit* m_searchInput = nullptr;
    QTableWidget* m_songTable = nullptr;
    QLabel* m_summaryLabel = nullptr;
    QLabel* m_pageTitle = nullptr;
    QLabel* m_pageSubtitle = nullptr;

    // 空状态控件
    QWidget* m_emptyState = nullptr;
    QLabel* m_emptyTitle = nullptr;
    QLabel* m_emptyDesc = nullptr;
    QPushButton* m_emptyClearBtn = nullptr;

    // 轻提示（Toast）控件
    QWidget* m_toast = nullptr;
    QLabel* m_toastLabel = nullptr;
    QGraphicsOpacityEffect* m_toastFx = nullptr;
    QPropertyAnimation* m_toastAnim = nullptr;

    // 搜索防抖
    QTimer* m_searchDebounceTimer = nullptr;
    QString  m_searchQuery;

    // 当前显示的歌曲
    QList<Song> m_currentSongs;

    // 快捷键
    QShortcut* m_shortcutRename = nullptr;
    QShortcut* m_shortcutDelete = nullptr;

    // —— 选择与视图顺序 —— 
    QStringList selectedSongIdsFromView() const;  // 从表格“当前可见顺序”取所选歌曲 IDs
    QList<Song> songsInViewOrder() const;         // 返回按表格可见顺序排列的歌曲列表

    // —— 批量动作 —— 
    void actAddToPlaylist(const QStringList& songIds, const QString& playlistId); // 批量加歌单
    void actRemoveFromCurrentPlaylist(const QStringList& songIds);                 // 批量从当前歌单移除
    void actDeleteSongs(const QStringList& songIds);                               // 批量删除

    // —— 快捷键（表格范围）——
    QShortcut* m_tblSelectAll = nullptr;  // Ctrl+A
    QShortcut* m_tblDelete = nullptr;  // Delete
};
