// service/AudioPlayer.cpp
#include "AudioPlayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QObject* parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_currentState(PlaybackState::Stopped)
{
    // 设置音频输出
    m_player->setAudioOutput(m_audioOutput);

    // 连接基本信号
    connect(m_player, &QMediaPlayer::playbackStateChanged,
        this, &AudioPlayer::handlePlaybackStateChanged);
    connect(m_player, &QMediaPlayer::positionChanged,
        this, &AudioPlayer::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
        this, &AudioPlayer::durationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
        this, &AudioPlayer::handleMediaStatusChanged);

    qDebug() << "AudioPlayer initialized";
}

void AudioPlayer::play(const QString& filePath) {
    if (filePath.isEmpty()) {
        emit error("播放文件路径为空");
        return;
    }

    qDebug() << "AudioPlayer: 播放文件:" << filePath;

    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    m_player->setSource(fileUrl);
    m_player->play();
}

void AudioPlayer::pause() {
    m_player->pause();
    qDebug() << "AudioPlayer: 暂停播放";
}

void AudioPlayer::resume() {
    m_player->play();
    qDebug() << "AudioPlayer: 恢复播放";
}

void AudioPlayer::stop() {
    m_player->stop();
    qDebug() << "AudioPlayer: 停止播放";
}

void AudioPlayer::setPosition(qint64 position) {
    m_player->setPosition(position);
    qDebug() << "AudioPlayer: 设置播放位置:" << position;
}

qint64 AudioPlayer::position() const {
    return m_player->position();
}

qint64 AudioPlayer::duration() const {
    return m_player->duration();
}

void AudioPlayer::setVolume(int volume) {
    int clampedVolume = qBound(0, volume, 100);
    float normalizedVolume = clampedVolume / 100.0f;

    m_audioOutput->setVolume(normalizedVolume);
    emit volumeChanged(clampedVolume);  // 主动发射音量变化信号

    qDebug() << "AudioPlayer: 设置音量:" << clampedVolume;
}

int AudioPlayer::volume() const {
    return static_cast<int>(m_audioOutput->volume() * 100);
}

PlaybackState AudioPlayer::state() const {
    return m_currentState;
}

void AudioPlayer::handleMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    qDebug() << "AudioPlayer: 媒体状态变化:" << status;

    switch (status) {
    case QMediaPlayer::EndOfMedia:
        emit finished();
        qDebug() << "AudioPlayer: 播放完成";
        break;

    case QMediaPlayer::InvalidMedia:
        emit error("无效的媒体文件");
        break;

    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadingMedia:
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::StalledMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        // 这些状态暂时不处理
        break;
    }
}

void AudioPlayer::handlePlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    PlaybackState newState = convertState(state);
    if (m_currentState != newState) {
        m_currentState = newState;
        emit stateChanged(newState);
        qDebug() << "AudioPlayer: 播放状态变化:" << (int)newState;
    }
}

PlaybackState AudioPlayer::convertState(QMediaPlayer::PlaybackState state) const {
    switch (state) {
    case QMediaPlayer::PlayingState:
        return PlaybackState::Playing;
    case QMediaPlayer::PausedState:
        return PlaybackState::Paused;
    case QMediaPlayer::StoppedState:
    default:
        return PlaybackState::Stopped;
    }
}
