#include "LibraryPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QKeySequence>
#include <QDebug>
#include <QShortcut>
#include <QMenu>
#include <QFont>
#include <QTimer>
#include <QHash>
#include <QGraphicsOpacityEffect> 
#include <QPropertyAnimation> 
#include <QAbstractItemModel>  
#include <QMetaObject>
#include <QResizeEvent>        
#include <QSet>
#include "../../service/PlaybackService.h"
#include "../../common/AppConfig.h"

static const char* kMyMusicId = "";        
static const char* kMyMusicText = "我的音乐";

LibraryPage::LibraryPage(LibraryViewModel* viewModel, QWidget* parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    Q_ASSERT(m_viewModel);
    setupUI();
    setupConnections();

    reloadPlaylists();
    selectMyMusic(); 
    reloadSongs();
}

/* ---------------- UI ---------------- */
void LibraryPage::setupUI() {
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    // ===== 左：侧边栏面板（含顶部按钮条 + 分组标题 + 列表） =====
    m_sidebarPanel = new QWidget(this);
    m_sidebarPanel->setObjectName("librarySidebarPanel");
    auto* sideLayout = new QVBoxLayout(m_sidebarPanel);
    sideLayout->setContentsMargins(12, 12, 12, 12);
    sideLayout->setSpacing(10);

    // 顶部按钮条
    auto* btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);

    m_btnCreate = new QPushButton("新建歌单", this);
    m_btnCreate->setObjectName("createPlaylistBtn");
    m_btnCreate->setCursor(Qt::PointingHandCursor);

    m_btnImport = new QPushButton("导入歌单", this); // 若不想露出可 setVisible(false)
    m_btnImport->setObjectName("importPlaylistBtn");
    m_btnImport->setCursor(Qt::PointingHandCursor);

    btnRow->addWidget(m_btnCreate, 1);
    btnRow->addWidget(m_btnImport, 1);

    // 分组标题：歌单
    auto* sidebarTitle = new QLabel("歌单", this);
    sidebarTitle->setObjectName("librarySidebarTitle");

    // 歌单列表
    m_sidebar = new QListWidget(this);
    m_sidebar->setObjectName("librarySidebar");
    m_sidebar->setContextMenuPolicy(Qt::CustomContextMenu);
    m_sidebar->setFixedWidth(260);

    // ✅ 开启拖拽排序
    m_sidebar->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sidebar->setDragEnabled(true);
    m_sidebar->setAcceptDrops(true);
    m_sidebar->setDropIndicatorShown(true);
    m_sidebar->setDefaultDropAction(Qt::MoveAction);
    m_sidebar->setDragDropMode(QAbstractItemView::InternalMove);

    sideLayout->addLayout(btnRow);
    sideLayout->addWidget(sidebarTitle, 0);
    sideLayout->addWidget(m_sidebar, 1);

    // ===== 右：标题区 + 搜索行 + 歌曲表 =====
    auto* rightWrap = new QWidget(this);
    auto* rightLayout = new QVBoxLayout(rightWrap);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    // 页面标题区
    m_pageTitle = new QLabel("音乐库", this);
    m_pageTitle->setObjectName("libraryPageTitle");

    m_pageSubtitle = new QLabel("", this);
    m_pageSubtitle->setObjectName("libraryPageSubtitle");

    auto* titleBox = new QVBoxLayout();
    titleBox->setSpacing(2);
    titleBox->addWidget(m_pageTitle, 0);
    titleBox->addWidget(m_pageSubtitle, 0);

    // 搜索 + 统计
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(12);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("搜索 标题 / 艺术家 …");
    m_searchInput->setObjectName("librarySearchInput");

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName("librarySummaryLabel");

    topRow->addWidget(m_searchInput, 1);
    topRow->addWidget(m_summaryLabel, 0);

    // 表格
    m_songTable = new QTableWidget(this);
    m_songTable->setObjectName("librarySongTable");
    m_songTable->setColumnCount(4);
    m_songTable->setHorizontalHeaderLabels({ "标题", "艺术家", "时长", "下载时间" });
    m_songTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_songTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_songTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_songTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_songTable->verticalHeader()->setVisible(false);
    m_songTable->verticalHeader()->setDefaultSectionSize(44); // 行高
    m_songTable->setShowGrid(false);                          // 现代风：去格线
    m_songTable->setWordWrap(false);
    m_songTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_songTable->setSelectionMode(QAbstractItemView::ExtendedSelection); // ✅ 多选
    m_songTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_songTable->setAlternatingRowColors(false);
    m_songTable->setSortingEnabled(true);
    m_songTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_songTable->setFrameShape(QFrame::NoFrame);

    // 右侧装配
    rightLayout->addLayout(titleBox);
    rightLayout->addLayout(topRow);
    rightLayout->addWidget(m_songTable, 1);

    // 整体布局
    root->addWidget(m_sidebarPanel);
    root->addWidget(rightWrap, 1);
    setLayout(root);

    // ✅ 空状态层（覆盖在表格上，跟随尺寸）
    m_emptyState = new QWidget(rightWrap);
    m_emptyState->setObjectName("libraryEmptyState");
    m_emptyState->hide();
    auto* esLayout = new QVBoxLayout(m_emptyState);
    esLayout->setAlignment(Qt::AlignCenter);
    esLayout->setSpacing(6);
    esLayout->setContentsMargins(0, 0, 0, 0);
    m_emptyTitle = new QLabel("这里空空如也", m_emptyState);
    m_emptyTitle->setAlignment(Qt::AlignCenter);
    m_emptyDesc = new QLabel("你可以下载音乐或导入歌单", m_emptyState);
    m_emptyDesc->setAlignment(Qt::AlignCenter);
    m_emptyDesc->setStyleSheet("color:#B8B8B8;");
    m_emptyClearBtn = new QPushButton("清除搜索", m_emptyState);
    m_emptyClearBtn->setVisible(false);
    connect(m_emptyClearBtn, &QPushButton::clicked, this, [this] {
        m_searchInput->clear();
        reloadSongs();
        });
    esLayout->addWidget(m_emptyTitle);
    esLayout->addWidget(m_emptyDesc);
    esLayout->addSpacing(8);
    esLayout->addWidget(m_emptyClearBtn, 0, Qt::AlignHCenter);

    // 初次几何同步
    if (m_songTable) {
        m_emptyState->setGeometry(m_songTable->geometry());
    }
}

/* ------------ 左侧歌单 ------------ */
void LibraryPage::reloadPlaylists() {
    m_sidebar->clear();

    // 固定项：我的音乐
    auto* my = new QListWidgetItem(kMyMusicText);
    my->setData(Qt::UserRole, QString(kMyMusicId));
    my->setToolTip("显示所有已下载音乐");
    QFont myFont = my->font(); myFont.setBold(true);
    my->setFont(myFont);
    m_sidebar->addItem(my);

    // 用户歌单
    const auto playlists = m_viewModel->getAllPlaylists();
    for (const auto& pl : playlists) {
        auto* it = new QListWidgetItem(pl.getName());
        it->setData(Qt::UserRole, pl.getId());
        it->setToolTip("歌单：" + pl.getName());
        m_sidebar->addItem(it);
    }
}

void LibraryPage::selectMyMusic() {
    for (int i = 0; i < m_sidebar->count(); ++i) {
        auto* it = m_sidebar->item(i);
        if (it->data(Qt::UserRole).toString().isEmpty()) {
            m_sidebar->setCurrentItem(it);
            break;
        }
    }
}

void LibraryPage::setupConnections() {
    // 左侧：点击与右键
    connect(m_sidebar, &QListWidget::itemClicked,
        this, &LibraryPage::onSidebarItemClicked);
    connect(m_sidebar, &QListWidget::customContextMenuRequested,
        this, &LibraryPage::onSidebarContextMenuRequested);

    // 拖拽完成后，保存顺序
    connect(m_sidebar->model(), &QAbstractItemModel::rowsMoved,
        this, &LibraryPage::onSidebarOrderChanged);

    // 顶部按钮
    connect(m_btnCreate, &QPushButton::clicked, this, &LibraryPage::actCreatePlaylist);
    connect(m_btnImport, &QPushButton::clicked, this, &LibraryPage::actImportPlaylist);

    // 右侧：双击/右键
    connect(m_songTable, &QTableWidget::cellDoubleClicked,
        this, &LibraryPage::onSongCellDoubleClicked);
    connect(m_songTable, &QTableWidget::customContextMenuRequested,
        this, &LibraryPage::onSongTableContextMenuRequested);

    // 数据变化
    connect(m_viewModel, &LibraryViewModel::songsChanged,
        this, &LibraryPage::onSongsChanged);
    connect(m_viewModel, &LibraryViewModel::playlistsChanged,
        this, &LibraryPage::onPlaylistsChanged);

    // 搜索防抖（200ms）
    m_searchDebounceTimer = new QTimer(this);
    m_searchDebounceTimer->setSingleShot(true);
    m_searchDebounceTimer->setInterval(200);
    connect(m_searchDebounceTimer, &QTimer::timeout, this, &LibraryPage::onSearchTimeout);
    connect(m_searchInput, &QLineEdit::textChanged, this, &LibraryPage::onSearchTextChanged);

    // 快捷键：F2 重命名、Delete 删除
    m_shortcutRename = new QShortcut(QKeySequence(Qt::Key_F2), m_sidebar);
    m_shortcutRename->setContext(Qt::WidgetWithChildrenShortcut);
    connect(m_shortcutRename, &QShortcut::activated, this, [this] {
        auto* it = m_sidebar->currentItem();
        if (!it) return;
        const QString pid = it->data(Qt::UserRole).toString();
        if (pid.isEmpty()) return;
        actRenamePlaylist(pid, it->text());
        });

    m_shortcutDelete = new QShortcut(QKeySequence::Delete, m_sidebar);
    m_shortcutDelete->setContext(Qt::WidgetWithChildrenShortcut);
    connect(m_shortcutDelete, &QShortcut::activated, this, [this] {
        auto* it = m_sidebar->currentItem();
        if (!it) return;
        const QString pid = it->data(Qt::UserRole).toString();
        if (pid.isEmpty()) return;
        actDeletePlaylist(pid, it->text());
        });

    // Ctrl+A 全选
    m_tblSelectAll = new QShortcut(QKeySequence::SelectAll, m_songTable);
    m_tblSelectAll->setContext(Qt::WidgetWithChildrenShortcut);
    connect(m_tblSelectAll, &QShortcut::activated, this, [this] {
        m_songTable->selectAll();
        });

    // Delete 批量删除
    m_tblDelete = new QShortcut(QKeySequence::Delete, m_songTable);
    m_tblDelete->setContext(Qt::WidgetWithChildrenShortcut);
    connect(m_tblDelete, &QShortcut::activated, this, [this] {
        const QStringList ids = selectedSongIdsFromView();
        if (!ids.isEmpty()) actDeleteSongs(ids);
        });

}

QString LibraryPage::currentPlaylistId() const {
    auto* it = m_sidebar->currentItem();
    return it ? it->data(Qt::UserRole).toString() : QString();
}

bool LibraryPage::inPlaylistMode() const {
    return !currentPlaylistId().isEmpty();
}

void LibraryPage::onSidebarItemClicked(QListWidgetItem* /*item*/) {
    m_searchInput->clear(); // 切换歌单时清空搜索
    reloadSongs();
}

void LibraryPage::onSidebarContextMenuRequested(const QPoint& pos) {
    QListWidgetItem* it = m_sidebar->itemAt(pos);
    if (!it) return; // 只对具体项弹出菜单；空白区域不弹

    const QString pid = it->data(Qt::UserRole).toString();
    const QString name = it->text();

    // “我的音乐”没有右键操作
    if (pid.isEmpty()) return;

    QMenu menu(this);

    // RUD + 导出（不再提供“新建”与“导入”，创建用按钮）
    QAction* actRename = menu.addAction("重命名歌单…");
    connect(actRename, &QAction::triggered, this, [=] { actRenamePlaylist(pid, name); });

    QAction* actDelete = menu.addAction("删除歌单");
    connect(actDelete, &QAction::triggered, this, [=] { actDeletePlaylist(pid, name); });

    menu.addSeparator();
    QAction* actExport = menu.addAction("导出该歌单为 JSON…");
    connect(actExport, &QAction::triggered, this, [=] { actExportPlaylist(pid, name); });

    menu.exec(m_sidebar->viewport()->mapToGlobal(pos));
}

/* ------------ 右侧歌曲表 ------------ */
void LibraryPage::reloadSongs() {
    const QString pid = currentPlaylistId();
    QList<Song> songs = pid.isEmpty()
        ? m_viewModel->getAllSongs()
        : m_viewModel->getPlaylistSongs(pid);

    loadSongs(songs);
}

void LibraryPage::loadSongs(const QList<Song>& songs) {
    m_currentSongs = songs;

    // 降低重绘抖动
    m_songTable->setUpdatesEnabled(false);
    m_songTable->setSortingEnabled(false);
    m_songTable->clearContents();
    m_songTable->setRowCount(m_currentSongs.size());

    // 计算总时长
    qlonglong totalDuration = 0; // 秒
    for (int i = 0; i < m_currentSongs.size(); ++i) {
        const Song& s = m_currentSongs.at(i);

        auto* titleItem = new QTableWidgetItem(s.getTitle());
        auto* artistItem = new QTableWidgetItem(s.getArtist());
        auto* durationItem = new QTableWidgetItem(formatDuration(s.getDurationSeconds()));
        auto* timeItem = new QTableWidgetItem(s.getDownloadDate().toString("yyyy-MM-dd HH:mm"));

        durationItem->setData(Qt::UserRole, static_cast<qlonglong>(s.getDurationSeconds()));
        timeItem->setData(Qt::UserRole, s.getDownloadDate().toSecsSinceEpoch());

        // 便于右键操作定位
        titleItem->setData(Qt::UserRole + 1, s.getId());

        m_songTable->setItem(i, 0, titleItem);
        m_songTable->setItem(i, 1, artistItem);
        m_songTable->setItem(i, 2, durationItem);
        m_songTable->setItem(i, 3, timeItem);

        totalDuration += s.getDurationSeconds(); // 累加
    }

    m_songTable->setSortingEnabled(true);
    m_songTable->setUpdatesEnabled(true);

    // 顶部统计：共 N 首 • 总时长 XX
    m_summaryLabel->setText(QString("共 %1 首 • 总时长 %2")
        .arg(m_currentSongs.size())
        .arg(humanizeDuration(totalDuration)));

    // 同步副标题（我的音乐 / 歌单名）
    updateHeaderText();

    // ✅ 更新空状态
    updateEmptyState();
}

void LibraryPage::updateHeaderText() {
    QString viewText;
    if (inPlaylistMode()) {
        const auto pl = findPlaylistById(currentPlaylistId());
        viewText = pl.getName().isEmpty() ? QStringLiteral("歌单") : pl.getName();
    }
    else {
        viewText = QStringLiteral("我的音乐");
    }
    m_pageSubtitle->setText(viewText);
}

QString LibraryPage::formatDuration(qlonglong seconds) const {
    if (seconds < 0) seconds = 0;
    qlonglong m = seconds / 60;
    qlonglong s = seconds % 60;
    return QString("%1:%2").arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));
}

QString LibraryPage::humanizeDuration(qlonglong seconds) const {
    if (seconds <= 0) return QStringLiteral("0:00");
    const qlonglong h = seconds / 3600;
    const qlonglong m = (seconds % 3600) / 60;
    const qlonglong s = seconds % 60;
    if (h > 0) return QStringLiteral("%1小时 %2分").arg(h).arg(m);
    return QStringLiteral("%1:%2").arg(m, 1, 10).arg(s, 2, 10, QLatin1Char('0'));
}

/* ---------- 歌曲右键菜单 ---------- */
void LibraryPage::onSongTableContextMenuRequested(const QPoint& pos) {
    // 视图下的选择集；若无选择但光标在一行，退化为单行选择
    QStringList sids = selectedSongIdsFromView();
    const QModelIndex hit = m_songTable->indexAt(pos);
    if (sids.isEmpty() && hit.isValid()) {
        if (auto* it = m_songTable->item(hit.row(), 0)) {
            const QString id = it->data(Qt::UserRole + 1).toString();
            if (!id.isEmpty()) sids << id;
        }
    }
    if (sids.isEmpty()) return;

    // 单选时用于“编辑元数据”的代表项
    Song rep;
    if (sids.size() == 1) {
        const QString only = sids.first();
        for (const auto& s : m_currentSongs) if (s.getId() == only) { rep = s; break; }
    }

    QMenu menu(this);

    // 添加到歌单（批量）
    QMenu* addMenu = menu.addMenu("添加到歌单");
    const auto pls = m_viewModel->getAllPlaylists();
    for (const auto& pl : pls) {
        QAction* act = addMenu->addAction(pl.getName());
        connect(act, &QAction::triggered, this, [=] { actAddToPlaylist(sids, pl.getId()); });
    }
    if (pls.isEmpty()) {
        QAction* na = addMenu->addAction("（暂无歌单）");
        na->setEnabled(false);
    }

    // 从当前歌单移除（批量，仅歌单视图）
    if (inPlaylistMode()) {
        QAction* actRm = menu.addAction("从当前歌单移除");
        connect(actRm, &QAction::triggered, this, [=] { actRemoveFromCurrentPlaylist(sids); });
    }

    menu.addSeparator();

    // 添加到播放队列（两种方式）
    QAction* actQNext = menu.addAction("添加到播放队列（下一首）");
    connect(actQNext, &QAction::triggered, this, [=] {
        const QList<Song> view = songsInViewOrder();
        QSet<QString> idset; idset.reserve(sids.size());
        for (const auto& id : sids) idset.insert(id);

        QList<Song> picked; picked.reserve(idset.size());
        for (const auto& s : view) if (idset.contains(s.getId())) picked << s;
        if (picked.isEmpty()) return;

        auto& ps = PlaybackService::instance();
        // 逆序插入，保证“第一首选中”的最先播放
        for (int i = picked.size() - 1; i >= 0; --i) {
            ps.addNextToQueue(picked[i]);
        }
        showToast(QString("已添加到播放队列（下一首） • %1 首").arg(picked.size()));
        });

    QAction* actQTail = menu.addAction("添加到播放队列（末尾）");
    connect(actQTail, &QAction::triggered, this, [=] {
        const QList<Song> view = songsInViewOrder();
        QSet<QString> idset; idset.reserve(sids.size());
        for (const auto& id : sids) idset.insert(id);

        QList<Song> picked; picked.reserve(idset.size());
        for (const auto& s : view) if (idset.contains(s.getId())) picked << s;
        if (picked.isEmpty()) return;

        auto& ps = PlaybackService::instance();
        // 使用仅有的 API 实现“追加到末尾”：
        // 最终队列 = 原队列 + 选中歌曲。要用 addNextToQueue 实现：
        // 步骤：保存原队列 → 清空 → 先倒序压入 picked（让它们暂时在前）→ 再倒序压入旧队列（恢复到前部）
        const QList<Song> oldQ = ps.getPlaybackQueue();
        ps.clearPlaybackQueue();

        for (int i = picked.size() - 1; i >= 0; --i) {
            ps.addNextToQueue(picked[i]);         // 暂时放到前面
        }
        for (int i = oldQ.size() - 1; i >= 0; --i) {
            ps.addNextToQueue(oldQ[i]);           // 恢复原队列到前部
        }
        showToast(QString("已添加到播放队列（末尾） • %1 首").arg(picked.size()));
        });

    menu.addSeparator();

    // 单选时允许编辑
    QAction* actEdit = menu.addAction("修改标题/艺术家…");
    actEdit->setEnabled(sids.size() == 1);
    connect(actEdit, &QAction::triggered, this, [=] { if (!rep.getId().isEmpty()) actEditSongMeta(rep); });

    // 批量删除
    QAction* actDel = menu.addAction(sids.size() > 1 ? QString("删除所选 %1 首…").arg(sids.size())
        : QString("删除该歌曲…"));
    connect(actDel, &QAction::triggered, this, [=] { actDeleteSongs(sids); });

    menu.exec(m_songTable->viewport()->mapToGlobal(pos));
}

/* ---------- 歌曲动作 ---------- */
void LibraryPage::actAddToPlaylist(const QString&) {
    // 兼容头文件中的单项接口（当前未使用）
}

void LibraryPage::actRemoveFromCurrentPlaylist(const QString& songId) {
    const QString pid = currentPlaylistId();
    if (pid.isEmpty()) return;
    m_viewModel->removeSongFromPlaylist(pid, songId);
    onSongsChanged();
}
void LibraryPage::actEditSongMeta(const Song& song) {
    bool ok1 = false, ok2 = false;
    QString newTitle = QInputDialog::getText(this, "编辑歌曲", "标题：",
        QLineEdit::Normal, song.getTitle(), &ok1);
    if (!ok1) return;
    QString newArtist = QInputDialog::getText(this, "编辑歌曲", "艺术家：",
        QLineEdit::Normal, song.getArtist(), &ok2);
    if (!ok2) return;

    if (newTitle.trimmed().isEmpty()) {
        QMessageBox::warning(this, "无效输入", "标题不能为空。");
        return;
    }
    m_viewModel->updateSong(song.getId(), newTitle.trimmed(), newArtist.trimmed());

    onSongsChanged();
}
void LibraryPage::actDeleteSong(const QString& songId) {
    auto ret = QMessageBox::question(
        this, "删除确认",
        "将删除本地音频文件并移除数据库中该歌曲及其关联。\n此操作不可恢复，确定继续？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        m_viewModel->deleteSong(songId);
        onSongsChanged();
        showToast("已删除 1 首歌曲");
    }
}

/* ---------- 歌单动作（创建=按钮，RUD=右键） ---------- */
void LibraryPage::actCreatePlaylist() {
    bool ok = false;
    const QString name = QInputDialog::getText(this, "新建歌单", "歌单名称：",
        QLineEdit::Normal, "", &ok).trimmed();
    if (!ok || name.isEmpty()) return;

    const QString id = m_viewModel->createPlaylist(name);
    if (id.isEmpty()) return;

    reloadPlaylists();
    // 选中新建的歌单
    for (int i = 0; i < m_sidebar->count(); ++i) {
        auto* it = m_sidebar->item(i);
        if (it->data(Qt::UserRole).toString() == id) {
            m_sidebar->setCurrentItem(it);
            break;
        }
    }
    reloadSongs();
}

Playlist LibraryPage::findPlaylistById(const QString& id) const {
    const auto pls = m_viewModel->getAllPlaylists();
    for (const auto& p : pls) if (p.getId() == id) return p;
    return Playlist();
}

void LibraryPage::actRenamePlaylist(const QString& playlistId, const QString& oldName) {
    const Playlist old = findPlaylistById(playlistId);

    bool ok = false;
    const QString name = QInputDialog::getText(this, "重命名歌单", "新名称：",
        QLineEdit::Normal, oldName, &ok).trimmed();
    if (!ok || name.isEmpty()) return;

    m_viewModel->updatePlaylist(playlistId, name, old.getDescription());
}

void LibraryPage::actDeletePlaylist(const QString& playlistId, const QString& name) {
    auto ret = QMessageBox::question(
        this, "删除歌单",
        QString("将删除歌单“%1”（不会删除歌曲本身）。继续？").arg(name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    m_viewModel->deletePlaylist(playlistId);
    selectMyMusic();
    reloadSongs();
}

void LibraryPage::actExportPlaylist(const QString& playlistId, const QString& name) {
    const QString path = QFileDialog::getSaveFileName(
        this, "导出歌单为 JSON", name + ".json", "JSON 文件 (*.json)");
    if (path.isEmpty()) return;
    m_viewModel->exportPlaylist(playlistId, path);
}

void LibraryPage::actImportPlaylist() {
    const QString path = QFileDialog::getOpenFileName(
        this, "选择歌单 JSON 文件", QString(), "JSON 文件 (*.json)");
    if (path.isEmpty()) return;

    auto data = m_viewModel->parseImportFile(path);
    if (data.songs.isEmpty()) {
        QMessageBox::warning(this, "导入失败", "文件格式无效或不包含歌曲。");
        return;
    }

    // 使用导出名为默认
    bool ok = false;
    QString name = QInputDialog::getText(
        this, "导入为新歌单", "歌单名称：",
        QLineEdit::Normal, data.playlistName.isEmpty() ? "导入歌单" : data.playlistName, &ok).trimmed();
    if (!ok || name.isEmpty()) return;

    const QString newId = m_viewModel->createPlaylist(name, data.playlistDescription);
    if (newId.isEmpty()) {
        QMessageBox::warning(this, "创建失败", "无法创建歌单。");
        return;
    }

    // 尝试在现有库中匹配歌曲（按 标题+艺术家）
    QStringList matchedIds;
    for (const auto& s : data.songs) {
        const auto candidates = m_viewModel->searchSongs(s.getTitle());
        for (const auto& c : candidates) {
            if (!s.getArtist().isEmpty() &&
                c.getArtist().compare(s.getArtist(), Qt::CaseInsensitive) != 0) {
                continue;
            }
            matchedIds << c.getId();
            break;
        }
    }
    if (!matchedIds.isEmpty()) {
        m_viewModel->addSongsToPlaylist(newId, matchedIds);
    }

    // 将“未匹配到”的歌曲（或全部再次核对 BV 号）提交到并行下载
    m_viewModel->importAndDownloadMissingSongs(newId, data.songs);

    reloadPlaylists();
    // 选中新歌单并显示
    for (int i = 0; i < m_sidebar->count(); ++i) {
        auto* it = m_sidebar->item(i);
        if (it->data(Qt::UserRole).toString() == newId) {
            m_sidebar->setCurrentItem(it);
            break;
        }
    }
    reloadSongs();

    const int total = data.songs.size();
    const int matched = matchedIds.size();
    const int toDownload = qMax(0, total - matched);
    const int maxC = AppConfig::instance().getMaxConcurrentDownloads();

    QMessageBox::information(
        this, "导入完成",
        QString("新歌单“%1”已创建。\n"
            "已匹配到本地歌曲：%2 首。\n"
            "已提交下载：%3 首（最大并行：%4）")
        .arg(name).arg(matched).arg(toDownload).arg(maxC));
}

/* -------- 数据变更刷新 -------- */
void LibraryPage::onSongsChanged() {
    const QString key = m_searchInput->text().trimmed();
    if (key.isEmpty()) reloadSongs();
    else onSearchTextChanged(key); // 会触发防抖后执行
}

void LibraryPage::onPlaylistsChanged() {
    const QString keepId = currentPlaylistId();
    reloadPlaylists();

    // 尝试保持当前选中（被删则回到“我的音乐”）
    bool kept = false;
    if (!keepId.isEmpty()) {
        for (int i = 0; i < m_sidebar->count(); ++i) {
            auto* it = m_sidebar->item(i);
            if (it->data(Qt::UserRole).toString() == keepId) {
                m_sidebar->setCurrentItem(it);
                kept = true;
                break;
            }
        }
    }
    if (!kept) selectMyMusic();

    reloadSongs();
}

/* -------- 搜索：防抖实现 -------- */
void LibraryPage::onSearchTextChanged(const QString& text) {
    m_searchQuery = text.trimmed(); // 记录搜索文本
    m_searchDebounceTimer->start(); // 重启定时器
}

void LibraryPage::onSearchTimeout() {
    if (m_searchQuery.isEmpty()) {
        reloadSongs();
        return;
    }
    if (inPlaylistMode()) {
        QList<Song> base = m_viewModel->getPlaylistSongs(currentPlaylistId());
        QList<Song> filtered;
        for (const auto& s : base) {
            if (s.getTitle().contains(m_searchQuery, Qt::CaseInsensitive) ||
                s.getArtist().contains(m_searchQuery, Qt::CaseInsensitive)) {
                filtered.append(s);
            }
        }
        loadSongs(filtered);
    }
    else {
        loadSongs(m_viewModel->searchSongs(m_searchQuery));
    }
}

QStringList LibraryPage::selectedSongIdsFromView() const {
    QStringList ids;
    const auto rows = m_songTable->selectionModel()->selectedRows();
    ids.reserve(rows.size());
    for (const auto& idx : rows) {
        int r = idx.row();
        if (r < 0 || r >= m_songTable->rowCount()) continue;
        if (auto* it = m_songTable->item(r, 0)) {
            const QString id = it->data(Qt::UserRole + 1).toString();
            if (!id.isEmpty()) ids << id;
        }
    }
    ids.removeDuplicates();
    return ids;
}

// 批量添加到歌单
void LibraryPage::actAddToPlaylist(const QStringList& songIds, const QString& playlistId) {
    if (songIds.isEmpty() || playlistId.isEmpty()) return;
    m_viewModel->addSongsToPlaylist(playlistId, songIds);
    if (playlistId == currentPlaylistId()) {
        onSongsChanged();
    }
    const auto pl = findPlaylistById(playlistId);
    showToast(QString("已添加到歌单“%1” • %2 首").arg(pl.getName()).arg(songIds.size()));
}

// 批量从当前歌单移除
void LibraryPage::actRemoveFromCurrentPlaylist(const QStringList& songIds) {
    const QString pid = currentPlaylistId();
    if (pid.isEmpty() || songIds.isEmpty()) return;
    for (const auto& id : songIds) m_viewModel->removeSongFromPlaylist(pid, id); // 你的接口是单个移除
    onSongsChanged();
    showToast(QString("已从当前歌单移除 • %1 首").arg(songIds.size()));
}

// 批量删除
void LibraryPage::actDeleteSongs(const QStringList& songIds) {
    if (songIds.isEmpty()) return;
    auto ret = QMessageBox::question(
        this, "删除确认",
        QString("将删除本地音频文件并移除数据库中 %1 首歌曲及其关联。\n此操作不可恢复，确定继续？")
        .arg(songIds.size()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    for (const auto& id : songIds) m_viewModel->deleteSong(id);

    onSongsChanged();
    showToast(QString("已删除 • %1 首").arg(songIds.size()));
}

QList<Song> LibraryPage::songsInViewOrder() const {
    QList<Song> result; result.reserve(m_songTable->rowCount());
    QHash<QString, Song> byId; byId.reserve(m_currentSongs.size());
    for (const auto& s : m_currentSongs) byId.insert(s.getId(), s);

    for (int r = 0; r < m_songTable->rowCount(); ++r) {
        if (auto* it = m_songTable->item(r, 0)) {
            const QString id = it->data(Qt::UserRole + 1).toString();
            if (byId.contains(id)) result << byId.value(id);
        }
    }
    return result;
}

void LibraryPage::onSongCellDoubleClicked(int row, int /*column*/) {
    if (row < 0 || row >= m_songTable->rowCount()) return;
    const QList<Song> viewList = songsInViewOrder();
    if (viewList.isEmpty()) return;
    // row 即“可见顺序”的起播索引
    emit requestPlaySongs(viewList, row);
}

/* ---------- 空状态 ---------- */
void LibraryPage::updateEmptyState() {
    if (!m_emptyState || !m_songTable) return;

    const bool empty = m_currentSongs.isEmpty();
    m_emptyState->setVisible(empty);
    if (!empty) return;

    // 几何更新 & 置顶
    m_emptyState->setGeometry(m_songTable->geometry());
    m_emptyState->raise();

    const bool bySearch = !m_searchQuery.isEmpty();
    if (bySearch) {
        m_emptyTitle->setText("没有匹配的歌曲");
        m_emptyDesc->setText("试试更短的关键词或不同拼写");
        m_emptyClearBtn->setText("清除搜索");
        m_emptyClearBtn->setVisible(true);
        // 确保按钮行为是“清除搜索”
        disconnect(m_emptyClearBtn, nullptr, nullptr, nullptr);
        connect(m_emptyClearBtn, &QPushButton::clicked, this, [this] {
            m_searchInput->clear();
            reloadSongs();
            });
    }
    else {
        m_emptyTitle->setText("这里空空如也");
        m_emptyDesc->setText("你可以下载音乐，或导入一个 JSON 歌单");
        m_emptyClearBtn->setText("导入歌单…");
        m_emptyClearBtn->setVisible(true);
        // 将按钮切换为导入行为
        disconnect(m_emptyClearBtn, nullptr, nullptr, nullptr);
        connect(m_emptyClearBtn, &QPushButton::clicked, this, &LibraryPage::actImportPlaylist);
    }
}

/* ---------- 轻提示（Toast） ---------- */
void LibraryPage::showToast(const QString& text) {
    if (!m_toast) {
        m_toast = new QWidget(this);
        m_toast->setObjectName("libraryToast");
        m_toast->setAttribute(Qt::WA_TranslucentBackground);
        m_toast->setStyleSheet("background: rgba(0,0,0,160); border-radius: 8px;");

        auto* lay = new QHBoxLayout(m_toast);
        lay->setContentsMargins(12, 8, 12, 8);
        lay->setSpacing(8);

        m_toastLabel = new QLabel(m_toast);
        m_toastLabel->setStyleSheet("color: white; font-weight: 600;");
        m_toastLabel->setText(text);
        lay->addWidget(m_toastLabel);

        m_toastFx = new QGraphicsOpacityEffect(m_toast);
        m_toast->setGraphicsEffect(m_toastFx);

        m_toastAnim = new QPropertyAnimation(m_toastFx, "opacity", this);
        m_toastAnim->setDuration(160);
    }

    m_toastLabel->setText(text);
    m_toast->adjustSize();

    // 定位：页面底部中间，留出一些边距
    const int x = (width() - m_toast->width()) / 2;
    const int y = height() - m_toast->height() - 24;
    m_toast->move(qMax(12, x), qMax(12, y));
    m_toast->show();
    m_toast->raise();

    // 先停止任何正在进行的动画
    if (m_toastAnim->state() == QAbstractAnimation::Running) {
        m_toastAnim->stop();
    }

    // 淡入
    m_toastFx->setOpacity(0.0);
    m_toastAnim->setStartValue(0.0);
    m_toastAnim->setEndValue(1.0);
    m_toastAnim->start();

    // 2 秒后淡出并隐藏
    QTimer::singleShot(2000, this, [this] {
        if (!m_toast || !m_toastFx || !m_toastAnim) return;
        if (m_toastAnim->state() == QAbstractAnimation::Running) {
            m_toastAnim->stop();
        }
        m_toastAnim->setDuration(220);
        m_toastAnim->setStartValue(1.0);
        m_toastAnim->setEndValue(0.0);
        connect(m_toastAnim, &QPropertyAnimation::finished, this, [this] {
            if (m_toastFx->opacity() == 0.0 && m_toast) m_toast->hide();
            });
        m_toastAnim->start();
        });
}

/* ---------- 拖拽排序持久化 ---------- */
void LibraryPage::onSidebarOrderChanged() {
    // 收集顺序（跳过“我的音乐”这个空 id）
    QStringList newOrder;
    newOrder.reserve(m_sidebar->count());
    for (int i = 0; i < m_sidebar->count(); ++i) {
        const QListWidgetItem* it = m_sidebar->item(i);
        const QString pid = it->data(Qt::UserRole).toString();
        if (!pid.isEmpty()) {
            newOrder << pid;
        }
    }

    if (newOrder.isEmpty()) return;

    // 尝试调用 ViewModel 的持久化方法（任选其一）
    bool ok = QMetaObject::invokeMethod(
        m_viewModel, "updatePlaylistOrder",
        Q_ARG(QStringList, newOrder))
        || QMetaObject::invokeMethod(
            m_viewModel, "reorderPlaylists",
            Q_ARG(QStringList, newOrder));

    if (!ok) {
        qWarning() << "LibraryPage: ViewModel 未实现 updatePlaylistOrder(QStringList) / reorderPlaylists(QStringList)";
        showToast("顺序已调整（未持久化：接口缺失）");
    }
    else {
        showToast("已保存歌单顺序");
    }
}

void LibraryPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // 跟随表格大小调整 Empty State 覆盖层
    if (m_songTable && m_emptyState) {
        // 两者同属 rightWrap，geometry 一致即可；若父不同可用 mapTo 计算
        QRect r = m_songTable->geometry();
        // 如果父级不同，稳妥做法如下（保留以防将来重构父子关系变动）：
        if (m_emptyState->parentWidget() && m_songTable->parentWidget() &&
            m_emptyState->parentWidget() != m_songTable->parentWidget()) {
            const QPoint tl = m_songTable->mapTo(m_emptyState->parentWidget(), QPoint(0, 0));
            r.moveTopLeft(tl);
        }
        m_emptyState->setGeometry(r);
    }

    // 重新摆放 Toast（居中偏下）
    if (m_toast && m_toast->isVisible()) {
        m_toast->adjustSize();
        const int x = (width() - m_toast->width()) / 2;
        const int y = height() - m_toast->height() - 24;
        m_toast->move(qMax(12, x), qMax(12, y));
        m_toast->raise();
    }
}
