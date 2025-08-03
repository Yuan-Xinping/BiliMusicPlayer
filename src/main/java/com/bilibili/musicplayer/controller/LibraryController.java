// src/main/java/com/bilibili/musicplayer/controller/LibraryController.java
package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.model.Playlist;
import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.service.MediaPlayerService;
import com.bilibili.musicplayer.service.PlaylistDAO;
import com.bilibili.musicplayer.service.SongDAO;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.collections.transformation.FilteredList;
import javafx.collections.transformation.SortedList;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.*;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.input.MouseButton;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.stage.FileChooser;
import javafx.stage.Stage;
import javafx.scene.layout.GridPane;
import javafx.geometry.Insets;
import javafx.application.Platform;

import com.fasterxml.jackson.databind.ObjectMapper; // 导入 Jackson
import com.fasterxml.jackson.databind.SerializationFeature; // 用于美化输出

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.List;
import java.util.Optional;
import java.util.ResourceBundle;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors; // 用于 Java 8 Stream API

public class LibraryController implements Initializable {

    @FXML private HBox rootView;
    @FXML private VBox librarySidebar;
    @FXML private Button btnLocalMusic;
    @FXML private Button btnFavorites;
    @FXML private Button btnCreatePlaylist;
    @FXML private ListView<Playlist> playlistListView;

    @FXML private Label currentViewTitle;
    @FXML private TableView<Song> songTableView;
    @FXML private Label statusLabel;
    @FXML private TextField searchField;
    @FXML private Button btnExport;

    private SongDAO songDAO;
    private PlaylistDAO playlistDAO;
    private ObservableList<Song> masterData;
    private FilteredList<Song> filteredData;
    private SortedList<Song> sortedData;

    private MediaPlayerService mediaPlayerService;
    private final ObjectMapper objectMapper = new ObjectMapper(); // NEW: Jackson ObjectMapper 实例

    // 当前选中的视图类型
    private enum CurrentView {
        LOCAL_MUSIC, FAVORITES, PLAYLIST
    }
    private CurrentView currentView = CurrentView.LOCAL_MUSIC;
    private Playlist currentSelectedPlaylist = null;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("LibraryController initialized.");
        songDAO = new SongDAO();
        playlistDAO = new PlaylistDAO();
        masterData = FXCollections.observableArrayList();

        objectMapper.enable(SerializationFeature.INDENT_OUTPUT);

        filteredData = new FilteredList<>(masterData, p -> true);
        sortedData = new SortedList<>(filteredData);
        sortedData.comparatorProperty().bind(songTableView.comparatorProperty());
        songTableView.setItems(sortedData);

        TableColumn<Song, Long> durationCol = null;
        for (TableColumn<Song, ?> col : songTableView.getColumns()) {
            if ("时长".equals(col.getText())) {
                durationCol = (TableColumn<Song, Long>) col;
                break;
            }
        }

        if (durationCol != null) {
            durationCol.setCellValueFactory(new PropertyValueFactory<>("durationSeconds"));
            durationCol.setCellFactory(column -> new TableCell<Song, Long>() {
                @Override
                protected void updateItem(Long item, boolean empty) {
                    super.updateItem(item, empty);
                    if (empty || item == null) {
                        setText(null);
                    } else {
                        long minutes = item / 60;
                        long seconds = item % 60;
                        setText(String.format("%02d:%02d", minutes, seconds));
                    }
                }
            });
        } else {
            System.err.println("Warning: '时长' column not found in TableView. Duration formatting might not work.");
        }

        searchField.textProperty().addListener((observable, oldValue, newValue) -> applyFilter(newValue));

        btnLocalMusic.setOnAction(event -> showLocalMusic());
        btnFavorites.setOnAction(event -> showFavorites());
        btnCreatePlaylist.setOnAction(event -> createNewPlaylist());
        btnExport.setOnAction(event -> handleExportCurrentView()); // NEW: 导出按钮事件绑定

        playlistListView.getSelectionModel().selectedItemProperty().addListener((obs, oldVal, newVal) -> {
            if (newVal != null) {
                showPlaylist(newVal);
            }
        });

        songTableView.setOnMouseClicked(event -> {
            if (event.getButton().equals(MouseButton.PRIMARY) && event.getClickCount() == 2) {
                Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
                if (selectedSong != null) {
                    if (mediaPlayerService != null) {
                        mediaPlayerService.setPlaylist(new java.util.ArrayList<>(sortedData));
                        mediaPlayerService.playSong(selectedSong);
                    } else {
                        System.err.println("MediaPlayerService is not set in LibraryController. Cannot play song.");
                    }
                }
            }
        });

        songTableView.setContextMenu(createSongContextMenu());
        playlistListView.setContextMenu(createPlaylistContextMenu());

        loadPlaylists();
        showLocalMusic();
    }

    public HBox getView() {
        return rootView;
    }

    public void setMediaPlayerService(MediaPlayerService mediaPlayerService) {
        this.mediaPlayerService = mediaPlayerService;
    }

    public void refreshSongs() {
        System.out.println("Refreshing songs in LibraryView based on current view: " + currentView);
        switch (currentView) {
            case LOCAL_MUSIC:
                showLocalMusic();
                break;
            case FAVORITES:
                showFavorites();
                break;
            case PLAYLIST:
                if (currentSelectedPlaylist != null) {
                    showPlaylist(currentSelectedPlaylist);
                } else {
                    showLocalMusic();
                }
                break;
        }
        loadPlaylists();
    }

    public void addSong(Song song) {
        System.out.println("LibraryController received new song: " + song.getTitle());
        Platform.runLater(() -> {
            // 仅在当前视图为本地音乐时直接添加，否则 refreshSongs 会在后续处理
            if (currentView == CurrentView.LOCAL_MUSIC) {
                masterData.add(song);
            }
            refreshSongs(); // 刷新以确保正确的排序/过滤和状态更新
        });
    }

    private void showLocalMusic() {
        currentView = CurrentView.LOCAL_MUSIC;
        currentSelectedPlaylist = null;
        currentViewTitle.setText("本地音乐");
        masterData.clear();
        masterData.addAll(songDAO.getAllSongs());
        updateStatusLabel(masterData.size());
        applyFilter(searchField.getText());
        highlightSidebarButton(btnLocalMusic);
    }

    private void showFavorites() {
        currentView = CurrentView.FAVORITES;
        currentSelectedPlaylist = null;
        currentViewTitle.setText("我喜欢");
        masterData.clear();
        masterData.addAll(songDAO.getFavoriteSongs());
        updateStatusLabel(masterData.size());
        applyFilter(searchField.getText());
        highlightSidebarButton(btnFavorites);
    }

    private void showPlaylist(Playlist playlist) {
        currentView = CurrentView.PLAYLIST;
        currentSelectedPlaylist = playlist;
        currentViewTitle.setText(playlist.getName());
        masterData.clear();
        masterData.addAll(playlistDAO.getSongsInPlaylist(playlist.getId()));
        updateStatusLabel(masterData.size());
        applyFilter(searchField.getText());
        highlightPlaylistInListView(playlist);
    }

    private void updateStatusLabel(int count) {
        if (count == 0) {
            statusLabel.setText("当前视图中没有歌曲。");
        } else {
            statusLabel.setText("已加载 " + count + " 首歌曲。");
        }
    }

    private void applyFilter(String searchText) {
        filteredData.setPredicate(song -> {
            if (searchText == null || searchText.isEmpty()) {
                return true;
            }
            String lowerCaseFilter = searchText.toLowerCase();
            return (song.getTitle() != null && song.getTitle().toLowerCase().contains(lowerCaseFilter)) ||
                    (song.getArtist() != null && song.getArtist().toLowerCase().contains(lowerCaseFilter));
        });
    }

    private void highlightSidebarButton(Button activeButton) {
        btnLocalMusic.getStyleClass().remove("selected");
        btnFavorites.getStyleClass().remove("selected");
        playlistListView.getSelectionModel().clearSelection();
        activeButton.getStyleClass().add("selected");
    }

    private void highlightPlaylistInListView(Playlist playlist) {
        btnLocalMusic.getStyleClass().remove("selected");
        btnFavorites.getStyleClass().remove("selected");
        playlistListView.getSelectionModel().select(playlist);
    }

    // NEW: 处理导出按钮点击事件
    @FXML
    private void handleExportCurrentView() {
        List<Song> songsToExport;
        String defaultFileNamePrefix;

        switch (currentView) {
            case LOCAL_MUSIC:
                songsToExport = songDAO.getAllSongs();
                defaultFileNamePrefix = "LocalMusic";
                break;
            case FAVORITES:
                songsToExport = songDAO.getFavoriteSongs();
                defaultFileNamePrefix = "Favorites";
                break;
            case PLAYLIST:
                if (currentSelectedPlaylist != null) {
                    songsToExport = playlistDAO.getSongsInPlaylist(currentSelectedPlaylist.getId());
                    // 清理播放列表名称，使其适合作为文件名
                    defaultFileNamePrefix = currentSelectedPlaylist.getName().replaceAll("[^a-zA-Z0-9-_.]", "_");
                } else {
                    statusLabel.setText("请选择一个播放列表进行导出。");
                    return;
                }
                break;
            default:
                statusLabel.setText("无法导出当前视图。");
                return;
        }

        if (songsToExport.isEmpty()) {
            statusLabel.setText("当前视图中没有歌曲可供导出。");
            return;
        }

        FileChooser fileChooser = new FileChooser();
        fileChooser.setTitle("导出歌曲信息");
        fileChooser.setInitialFileName(defaultFileNamePrefix + "_Export.json");
        fileChooser.getExtensionFilters().add(new FileChooser.ExtensionFilter("JSON Files", "*.json"));

        Stage stage = (Stage) rootView.getScene().getWindow();
        File file = fileChooser.showSaveDialog(stage);

        if (file != null) {
            try {
                // 将 Song 对象映射为 Map 列表，以便序列化为 JSON
                List<Map<String, String>> exportData = songsToExport.stream()
                        .map(song -> {
                            Map<String, String> songMap = new HashMap<>();
                            songMap.put("id", song.getId());
                            songMap.put("title", song.getTitle());
                            songMap.put("artist", song.getArtist());
                            songMap.put("bilibiliUrl", song.getBilibiliUrl());
                            songMap.put("coverUrl", song.getCoverUrl());
                            songMap.put("durationSeconds", String.valueOf(song.getDurationSeconds())); // 转换为字符串
                            songMap.put("isFavorite", String.valueOf(song.isFavorite())); // 转换为字符串
                            songMap.put("localFilePath", song.getLocalFilePath());
                            songMap.put("downloadDate", song.getDownloadDate().toString());
                            return songMap;
                        })
                        .collect(Collectors.toList());

                objectMapper.writeValue(file, exportData);
                statusLabel.setText("成功导出 " + songsToExport.size() + " 首歌曲到: " + file.getAbsolutePath());
            } catch (IOException e) {
                statusLabel.setText("导出失败: " + e.getMessage());
                e.printStackTrace();
                Platform.runLater(() -> {
                    Alert alert = new Alert(Alert.AlertType.ERROR);
                    alert.setTitle("导出错误");
                    alert.setHeaderText("导出歌曲信息时发生错误");
                    alert.setContentText("无法保存文件: " + e.getMessage());
                    alert.showAndWait();
                });
            }
        } else {
            statusLabel.setText("导出操作已取消。");
        }
    }


    private ContextMenu createSongContextMenu() {
        ContextMenu contextMenu = new ContextMenu();

        MenuItem playItem = new MenuItem("播放");
        playItem.setOnAction(event -> {
            Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
            if (selectedSong != null && mediaPlayerService != null) {
                mediaPlayerService.setPlaylist(new java.util.ArrayList<>(sortedData));
                mediaPlayerService.playSong(selectedSong);
            }
        });

        MenuItem toggleFavoriteItem = new MenuItem();
        toggleFavoriteItem.setOnAction(event -> {
            Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
            if (selectedSong != null) {
                boolean newFavoriteStatus = !selectedSong.isFavorite();
                selectedSong.setFavorite(newFavoriteStatus);
                if (songDAO.saveSong(selectedSong)) {
                    statusLabel.setText("歌曲 '" + selectedSong.getTitle() + "' 已" + (newFavoriteStatus ? "添加至" : "移出") + "我喜欢。");
                    refreshSongs();
                } else {
                    statusLabel.setText("更新歌曲收藏状态失败。");
                }
            }
        });

        MenuItem renameItem = new MenuItem("重命名...");
        renameItem.setOnAction(event -> {
            Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
            if (selectedSong != null) {
                showRenameSongDialog(selectedSong);
            }
        });

        Menu addToPlaylistMenu = new Menu("添加到播放列表");

        MenuItem removeFromPlaylist = new MenuItem("从播放列表移除");
        removeFromPlaylist.setOnAction(event -> {
            Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
            if (selectedSong != null && currentView == CurrentView.PLAYLIST && currentSelectedPlaylist != null) {
                if (playlistDAO.removeSongFromPlaylist(currentSelectedPlaylist.getId(), selectedSong.getId())) {
                    refreshSongs();
                    statusLabel.setText("已将 '" + selectedSong.getTitle() + "' 从 '" + currentSelectedPlaylist.getName() + "' 移除。");
                } else {
                    statusLabel.setText("从播放列表移除歌曲失败。");
                }
            } else {
                statusLabel.setText("此操作仅在播放列表视图中可用。");
            }
        });

        MenuItem deleteItem = new MenuItem("从音乐库删除 (及本地文件)");
        deleteItem.setOnAction(event -> {
            Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
            if (selectedSong != null) {
                showDeleteSongConfirmation(selectedSong);
            }
        });

        contextMenu.setOnShowing(event -> {
            Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
            boolean isSongSelected = (selectedSong != null);

            playItem.setDisable(!isSongSelected);

            toggleFavoriteItem.setDisable(!isSongSelected);
            if (isSongSelected) {
                toggleFavoriteItem.setText(selectedSong.isFavorite() ? "取消收藏" : "添加到我喜欢");
            } else {
                toggleFavoriteItem.setText("添加到我喜欢");
            }

            renameItem.setDisable(!isSongSelected);

            addToPlaylistMenu.setDisable(!isSongSelected);
            if (isSongSelected) {
                addToPlaylistMenu.getItems().clear();
                List<Playlist> playlists = playlistDAO.getAllPlaylists();
                if (playlists.isEmpty()) {
                    MenuItem noPlaylists = new MenuItem("无播放列表");
                    noPlaylists.setDisable(true);
                    addToPlaylistMenu.getItems().add(noPlaylists);
                } else {
                    for (Playlist playlist : playlists) {
                        MenuItem playlistItem = new MenuItem(playlist.getName());
                        playlistItem.setOnAction(e -> {
                            if (playlistDAO.isSongInPlaylist(playlist.getId(), selectedSong.getId())) {
                                statusLabel.setText("歌曲 '" + selectedSong.getTitle() + "' 已存在于 '" + playlist.getName() + "'。");
                            } else if (playlistDAO.addSongToPlaylist(playlist.getId(), selectedSong.getId())) {
                                statusLabel.setText("已将 '" + selectedSong.getTitle() + "' 添加到 '" + playlist.getName() + "'");
                                if (currentView == CurrentView.PLAYLIST && currentSelectedPlaylist != null && currentSelectedPlaylist.getId().equals(playlist.getId())) {
                                    refreshSongs();
                                }
                            } else {
                                statusLabel.setText("添加歌曲 '" + selectedSong.getTitle() + "' 到 '" + playlist.getName() + "' 失败。");
                            }
                        });
                        addToPlaylistMenu.getItems().add(playlistItem);
                    }
                }
            }

            removeFromPlaylist.setVisible(currentView == CurrentView.PLAYLIST);
            removeFromPlaylist.setDisable(!isSongSelected || currentView != CurrentView.PLAYLIST || currentSelectedPlaylist == null);

            deleteItem.setDisable(!isSongSelected);
        });


        contextMenu.getItems().addAll(playItem, new SeparatorMenuItem(), toggleFavoriteItem, renameItem, addToPlaylistMenu, removeFromPlaylist, new SeparatorMenuItem(), deleteItem);
        return contextMenu;
    }

    private ContextMenu createPlaylistContextMenu() {
        ContextMenu contextMenu = new ContextMenu();

        MenuItem renameItem = new MenuItem("重命名播放列表...");
        renameItem.setOnAction(event -> {
            Playlist selectedPlaylist = playlistListView.getSelectionModel().getSelectedItem();
            if (selectedPlaylist != null) {
                showRenamePlaylistDialog(selectedPlaylist);
            }
        });

        MenuItem deleteItem = new MenuItem("删除播放列表");
        deleteItem.setOnAction(event -> {
            Playlist selectedPlaylist = playlistListView.getSelectionModel().getSelectedItem();
            if (selectedPlaylist != null) {
                showDeletePlaylistConfirmation(selectedPlaylist);
            }
        });

        contextMenu.getItems().addAll(renameItem, deleteItem);
        return contextMenu;
    }


    private void showRenameSongDialog(Song song) {
        Dialog<Song> dialog = new Dialog<>();
        dialog.setTitle("重命名歌曲");
        dialog.setHeaderText("修改歌曲信息");

        ButtonType saveButtonType = new ButtonType("保存", ButtonBar.ButtonData.OK_DONE);
        dialog.getDialogPane().getButtonTypes().addAll(saveButtonType, ButtonType.CANCEL);

        GridPane grid = new GridPane();
        grid.setHgap(10);
        grid.setVgap(10);
        grid.setPadding(new Insets(20, 150, 10, 10));

        TextField titleField = new TextField(song.getTitle());
        TextField artistField = new TextField(song.getArtist());

        Platform.runLater(titleField::requestFocus);

        grid.add(new Label("标题:"), 0, 0);
        grid.add(titleField, 1, 0);
        grid.add(new Label("艺术家:"), 0, 1);
        grid.add(artistField, 1, 1);

        dialog.getDialogPane().setContent(grid);

        dialog.setResultConverter(dialogButton -> {
            if (dialogButton == saveButtonType) {
                song.setTitle(titleField.getText());
                song.setArtist(artistField.getText());
                return song;
            }
            return null;
        });

        Optional<Song> result = dialog.showAndWait();
        result.ifPresent(updatedSong -> {
            if (songDAO.saveSong(updatedSong)) {
                statusLabel.setText("歌曲 '" + updatedSong.getTitle() + "' 已更新。");
                refreshSongs();
            } else {
                statusLabel.setText("更新歌曲失败。");
            }
        });
    }

    private void showDeleteSongConfirmation(Song song) {
        Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
        alert.setTitle("删除歌曲");
        alert.setHeaderText("确认删除歌曲?");
        alert.setContentText("您确定要从音乐库中删除 '" + song.getTitle() + "' 吗？\n同时会删除本地文件: " + song.getLocalFilePath());

        Optional<ButtonType> result = alert.showAndWait();
        if (result.isPresent() && result.get() == ButtonType.OK) {
            if (songDAO.deleteSong(song.getId())) {
                File localFile = new File(song.getLocalFilePath());
                if (localFile.exists()) {
                    if (localFile.delete()) {
                        statusLabel.setText("歌曲 '" + song.getTitle() + "' 已成功删除。");
                    } else {
                        statusLabel.setText("歌曲 '" + song.getTitle() + "' 已从数据库删除，但本地文件删除失败。");
                    }
                } else {
                    statusLabel.setText("歌曲 '" + song.getTitle() + "' 已从数据库删除，本地文件不存在。");
                }
                refreshSongs();
            } else {
                statusLabel.setText("从数据库删除歌曲失败。");
            }
        }
    }

    private void createNewPlaylist() {
        TextInputDialog dialog = new TextInputDialog();
        dialog.setTitle("创建新播放列表");
        dialog.setHeaderText("请输入播放列表名称");
        dialog.setContentText("名称:");

        Platform.runLater(() -> dialog.getEditor().requestFocus());

        Optional<String> result = dialog.showAndWait();
        result.ifPresent(name -> {
            if (!name.trim().isEmpty()) {
                Playlist newPlaylist = new Playlist(name.trim(), "");
                if (playlistDAO.createPlaylist(newPlaylist)) {
                    statusLabel.setText("播放列表 '" + newPlaylist.getName() + "' 已创建。");
                    loadPlaylists();
                    showPlaylist(newPlaylist);
                } else {
                    statusLabel.setText("创建播放列表失败，可能名称已存在。");
                }
            } else {
                statusLabel.setText("播放列表名称不能为空。");
            }
        });
    }

    private void showRenamePlaylistDialog(Playlist playlist) {
        TextInputDialog dialog = new TextInputDialog(playlist.getName());
        dialog.setTitle("重命名播放列表");
        dialog.setHeaderText("请输入新的播放列表名称");
        dialog.setContentText("新名称:");

        Platform.runLater(() -> dialog.getEditor().requestFocus());

        Optional<String> result = dialog.showAndWait();
        result.ifPresent(newName -> {
            if (!newName.trim().isEmpty() && !newName.trim().equals(playlist.getName())) {
                String oldName = playlist.getName();
                playlist.setName(newName.trim());
                if (playlistDAO.updatePlaylist(playlist)) {
                    statusLabel.setText("播放列表 '" + oldName + "' 已重命名为 '" + newName.trim() + "'。");
                    loadPlaylists();
                    if (currentSelectedPlaylist != null && currentSelectedPlaylist.getId().equals(playlist.getId())) {
                        currentViewTitle.setText(newName.trim());
                    }
                } else {
                    statusLabel.setText("重命名播放列表失败，可能名称已存在。");
                }
            }
        });
    }

    private void showDeletePlaylistConfirmation(Playlist playlist) {
        Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
        alert.setTitle("删除播放列表");
        alert.setHeaderText("确认删除播放列表?");
        alert.setContentText("您确定要删除播放列表 '" + playlist.getName() + "' 吗？\n此操作将不可逆，但不会删除其中的歌曲文件。");

        Optional<ButtonType> result = alert.showAndWait();
        if (result.isPresent() && result.get() == ButtonType.OK) {
            if (playlistDAO.deletePlaylist(playlist.getId())) {
                statusLabel.setText("播放列表 '" + playlist.getName() + "' 已成功删除。");
                loadPlaylists();
                if (currentSelectedPlaylist != null && currentSelectedPlaylist.getId().equals(playlist.getId())) {
                    showLocalMusic();
                }
            } else {
                statusLabel.setText("删除播放列表失败。");
            }
        }
    }

    private void loadPlaylists() {
        Platform.runLater(() -> {
            List<Playlist> playlists = playlistDAO.getAllPlaylists();
            playlistListView.getItems().setAll(playlists);
        });
    }
}
