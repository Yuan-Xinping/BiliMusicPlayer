#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include "../../common/entities/Song.h"

class HoverButton;

class PlaybackBar : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackBar(QWidget* parent = nullptr);

    void setSong(const Song& song);
    void setDuration(int seconds);
    void setPosition(int seconds);
    void setVolume(int volume);
    void setPlaybackState(bool isPlaying);
    void setPlayMode(int mode); //服务 -> UI 同步播放模式（0 顺序、1 随机、2 单曲、3 列表）

signals:
    void playPauseClicked();
    void previousClicked();
    void nextClicked();
    void positionChanged(int position);
    void volumeChanged(int volume);
    void playModeChanged(int mode);

private slots:
    void onPositionSliderChanged(int value);
    void onVolumeSliderChanged(int value);
    void onModeButtonClicked();

private:
    void setupUI();
    void setupStyles();
    void updateTimeLabels();
    void updateModeButtonDisplay();
    QString formatTime(int seconds) const;

    QLabel* m_songTitle = nullptr;
    QLabel* m_artistName = nullptr;

    HoverButton* m_previousBtn = nullptr;
    HoverButton* m_playPauseBtn = nullptr;
    HoverButton* m_nextBtn = nullptr;
    HoverButton* m_modeButton = nullptr;

    QSlider* m_positionSlider = nullptr;
    QLabel* m_currentTimeLabel = nullptr;
    QLabel* m_totalTimeLabel = nullptr;

    HoverButton* m_volumeBtn = nullptr;
    QSlider* m_volumeSlider = nullptr;
    HoverButton* m_playlistBtn = nullptr;

    bool m_isPlaying = false;
    int m_playMode = 0;
    int m_currentPosition = 0;
    int m_totalDuration = 0;
    Song m_currentSong;
};
