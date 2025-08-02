// src/main/java/com/bilibili/musicplayer/controller/LibraryController.java
package com.bilibili.musicplayer.controller;

import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.service.MediaPlayerService; // 导入 MediaPlayerService
import com.bilibili.musicplayer.service.SongDAO;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.collections.transformation.FilteredList; // 导入 FilteredList
import javafx.collections.transformation.SortedList;   // 导入 SortedList
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
import javafx.scene.control.TextField; // 导入 TextField
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.scene.input.MouseButton;
import javafx.scene.layout.VBox;

import java.net.URL;
import java.time.LocalDateTime;
import java.util.List;
import java.util.ResourceBundle;

public class LibraryController implements Initializable {

    @FXML private VBox rootView;
    @FXML private TableView<Song> songTableView;
    @FXML private Label statusLabel;
    @FXML private TextField searchField; // 绑定搜索框

    private SongDAO songDAO;
    private ObservableList<Song> masterData; // 存储所有歌曲的原始列表
    private FilteredList<Song> filteredData; // 用于过滤的列表
    private SortedList<Song> sortedData;     // 用于排序的列表

    private MediaPlayerService mediaPlayerService;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("LibraryController initialized.");
        songDAO = new SongDAO();
        masterData = FXCollections.observableArrayList(); // 初始化原始数据列表

        // 1. 创建 FilteredList，初始显示所有数据
        filteredData = new FilteredList<>(masterData, p -> true);

        // 2. 监听搜索框文本变化，更新过滤条件
        searchField.textProperty().addListener((observable, oldValue, newValue) -> {
            filteredData.setPredicate(song -> {
                // 如果搜索框为空，显示所有歌曲
                if (newValue == null || newValue.isEmpty()) {
                    return true;
                }

                // 将搜索关键词转换为小写，进行不区分大小写的匹配
                String lowerCaseFilter = newValue.toLowerCase();

                // 匹配歌曲标题或艺术家
                if (song.getTitle() != null && song.getTitle().toLowerCase().contains(lowerCaseFilter)) {
                    return true; // 匹配标题
                } else if (song.getArtist() != null && song.getArtist().toLowerCase().contains(lowerCaseFilter)) {
                    return true; // 匹配艺术家
                }
                return false; // 不匹配
            });
        });

        // 3. 创建 SortedList，并将其绑定到 TableView 的 comparator
        sortedData = new SortedList<>(filteredData);
        sortedData.comparatorProperty().bind(songTableView.comparatorProperty());

        // 4. 将 SortedList 设置为 TableView 的数据源
        songTableView.setItems(sortedData);

        // 双击播放逻辑
        songTableView.setOnMouseClicked(event -> {
            if (event.getButton().equals(MouseButton.PRIMARY) && event.getClickCount() == 2) {
                Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
                if (selectedSong != null) {
                    /*
                    System.out.println("\n--- LibraryController Click Debug Start ---");
                    System.out.println("Selected Song from TableView: [ID: " + selectedSong.getId() + ", Title: " + selectedSong.getTitle() + ", Path: " + selectedSong.getLocalFilePath() + "]");
                    System.out.println("sortedData size (before passing to service): " + sortedData.size());
                    System.out.println("sortedData content (what's passed to MediaPlayerService):");
                    for (int i = 0; i < sortedData.size(); i++) {
                        Song s = sortedData.get(i);
                        System.out.println("    sortedData[" + i + "]: [ID: " + s.getId() + ", Title: " + s.getTitle() + ", Path: " + s.getLocalFilePath() + "]");
                    }
                    System.out.println("--- LibraryController Click Debug End ---\n");
                    */
                    if (mediaPlayerService != null) {
                        // 将当前 TableView 中显示的所有歌曲（即 sortedData）作为播放列表
                        mediaPlayerService.setPlaylist(new java.util.ArrayList<>(sortedData));
                        mediaPlayerService.playSong(selectedSong);
                    } else {
                        System.err.println("MediaPlayerService is not set in LibraryController. Cannot play song.");
                    }
                }
            }
        });


        refreshSongs(); // 初始加载歌曲
    }

    public VBox getView() {
        return rootView;
    }

    /**
     * 设置 MediaPlayerService 实例。
     * 应该由 MainController 在初始化时调用。
     * @param mediaPlayerService MediaPlayerService 实例
     */
    public void setMediaPlayerService(MediaPlayerService mediaPlayerService) {
        this.mediaPlayerService = mediaPlayerService;
    }

    /**
     * 从数据库加载所有歌曲并更新 TableView。
     * 此方法会重新加载所有数据到 masterData，从而触发 filteredData 和 sortedData 的更新。
     */
    public void refreshSongs() {
        System.out.println("Refreshing songs in LibraryView...");
        List<Song> songs = songDAO.getAllSongs();
        masterData.clear(); // 清空原始数据
        masterData.addAll(songs); // 添加所有新数据
        if (songs.isEmpty()) {
            statusLabel.setText("您的音乐库中还没有歌曲。");
        } else {
            statusLabel.setText("已加载 " + masterData.size() + " 首歌曲。"); // 使用 masterData.size() 因为它是原始数据
        }
    }

    /**
     * 添加一首新下载的歌曲到音乐库（并更新UI）。
     * 当 DownloadController 通知有新歌曲时调用此方法。
     * 直接添加到 masterData，FilteredList 和 SortedList 会自动响应。
     * @param song 新下载的歌曲对象
     */
    public void addSong(Song song) {
        System.out.println("LibraryController received new song: " + song.getTitle());
        // 确保在 JavaFX Application Thread 上更新 ObservableList
        javafx.application.Platform.runLater(() -> {
            masterData.add(song);
            statusLabel.setText("已加载 " + masterData.size() + " 首歌曲。"); // 更新总数
        });
    }
}
