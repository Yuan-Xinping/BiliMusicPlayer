#pragma once
#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include <QElapsedTimer>
#include <functional>
#include "../common/PlaybackState.h"

class AudioPlayer : public QObject {
    Q_OBJECT

public:
    explicit AudioPlayer(QObject* parent = nullptr);

    // 播放控制
    void play(const QString& filePath);
    void pause();
    void resume();
    void stop();

    // 位置和时长
    void setPosition(qint64 position);
    qint64 position() const;
    qint64 duration() const;

    // 音量控制
    void setVolume(int volume); // 0-100
    int volume() const;

    // 状态查询
    PlaybackState state() const;

signals:
    void stateChanged(PlaybackState state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void volumeChanged(int volume);
    void error(const QString& errorMessage);
    void finished();

private slots:
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void handlePlaybackStateChanged(QMediaPlayer::PlaybackState state);

private:
    PlaybackState convertState(QMediaPlayer::PlaybackState state) const;

    // 渐入/渐出
    void startFade(float from, float to, int durationMs, std::function<void()> done = {});
    void cancelFade();
    void setOutputVolumeRaw(float normalized); // 不触发 volumeChanged

    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    PlaybackState m_currentState;

    // 渐变相关
    QTimer m_fadeTimer;
    QElapsedTimer m_fadeClock;
    float m_fadeFrom = 0.0f;
    float m_fadeTo = 0.0f;
    int   m_fadeDurationMs = 0;
    std::function<void()> m_fadeDone;
    bool  m_isFading = false;

    // 逻辑音量（用户设置，0.0-1.0）
    float m_userVolume = 0.7f;
};
