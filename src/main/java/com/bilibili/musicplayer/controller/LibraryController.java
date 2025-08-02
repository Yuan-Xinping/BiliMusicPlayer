// src/main/java/com/bilibili/musicplayer/controller/LibraryController.java
package com.bilibili.musicplayer.controller;
import com.bilibili.musicplayer.model.Song;
import com.bilibili.musicplayer.service.SongDAO;

import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Label;
import javafx.scene.control.TableColumn;
import javafx.scene.control.TableView;
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

    private SongDAO songDAO;
    private ObservableList<Song> songList;
    private MainController mainController;

    @Override
    public void initialize(URL location, ResourceBundle resources) {
        System.out.println("LibraryController initialized.");
        songDAO = new SongDAO();
        songList = FXCollections.observableArrayList();
        songTableView.setItems(songList);

        TableColumn<Song, String> titleCol = (TableColumn<Song, String>) songTableView.getColumns().get(0);
        titleCol.setCellValueFactory(new PropertyValueFactory<>("title"));

        TableColumn<Song, String> artistCol = (TableColumn<Song, String>) songTableView.getColumns().get(1);
        artistCol.setCellValueFactory(new PropertyValueFactory<>("artist"));

        TableColumn<Song, Long> durationCol = (TableColumn<Song, Long>) songTableView.getColumns().get(2);
        durationCol.setCellValueFactory(new PropertyValueFactory<>("durationSeconds"));

        TableColumn<Song, LocalDateTime> downloadDateCol = (TableColumn<Song, LocalDateTime>) songTableView.getColumns().get(3);
        downloadDateCol.setCellValueFactory(new PropertyValueFactory<>("downloadDate"));

        TableColumn<Song, String> filePathCol = (TableColumn<Song, String>) songTableView.getColumns().get(4);
        filePathCol.setCellValueFactory(new PropertyValueFactory<>("localFilePath"));

        songTableView.setOnMouseClicked(event -> {
            if (event.getButton().equals(MouseButton.PRIMARY) && event.getClickCount() == 2) {
                Song selectedSong = songTableView.getSelectionModel().getSelectedItem();
                if (selectedSong != null) {
                    System.out.println("双击播放歌曲: " + selectedSong.getTitle());
                    if (mainController != null && mainController.getPlaybackBarController() != null) {
                        mainController.getPlaybackBarController().playSong(selectedSong);
                    }
                }
            }
        });

        refreshSongs();
    }

    public VBox getView() {
        return rootView;
    }

    public void setMainController(MainController mainController) {
        this.mainController = mainController;
    }

    /**
     * 从数据库加载所有歌曲并更新 TableView。
     */
    public void refreshSongs() {
        System.out.println("Refreshing songs in LibraryView...");
        List<Song> songs = songDAO.getAllSongs();
        songList.clear();
        songList.addAll(songs);
        if (songs.isEmpty()) {
            statusLabel.setText("您的音乐库中还没有歌曲。");
        } else {
            statusLabel.setText("已加载 " + songs.size() + " 首歌曲。");
        }
    }

    /**
     * 添加一首新下载的歌曲到音乐库（并更新UI）。
     * 当 DownloadController 通知有新歌曲时调用此方法。
     * @param song 新下载的歌曲对象
     */
    public void addSong(Song song) {
        System.out.println("LibraryController received new song: " + song.getTitle());
        refreshSongs();
    }
}
