#include "AudioPlayer.h"
#include <QDebug>
#include <QFileInfo>
#include <QUrl>

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

    // 渐变定时器
    connect(&m_fadeTimer, &QTimer::timeout, this, [this]() {
        if (!m_isFading) return;
        qint64 elapsed = m_fadeClock.elapsed();
        float t = m_fadeDurationMs > 0 ? qMin(1.0f, float(elapsed) / float(m_fadeDurationMs)) : 1.0f;
        float v = m_fadeFrom + (m_fadeTo - m_fadeFrom) * t;
        setOutputVolumeRaw(v);
        if (t >= 1.0f) {
            auto cb = std::move(m_fadeDone);
            cancelFade();
            if (cb) cb();
        }
        });

    qDebug() << "AudioPlayer initialized";
}

void AudioPlayer::play(const QString& filePath) {
    if (filePath.isEmpty()) {
        emit error("播放文件路径为空");
        return;
    }
    if (!QFileInfo::exists(filePath)) {
        emit error(QString("文件不存在: %1").arg(filePath));
        return;
    }

    qDebug() << "AudioPlayer: 播放文件:" << filePath
        << "qstate(before)=" << int(m_player->playbackState())
        << "userVol=" << m_userVolume
        << "outVol=" << m_audioOutput->volume();

    cancelFade();
    // 先设为 0，再渐入到用户音量
    setOutputVolumeRaw(0.0f);

    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    m_player->setSource(fileUrl);
    m_player->play();

    // 渐入
    startFade(0.0f, m_userVolume, 150);
}

void AudioPlayer::pause() {
    qDebug() << "AudioPlayer: pause() called."
        << "qstate(before)=" << int(m_player->playbackState())
        << "outVol=" << m_audioOutput->volume();

    // 渐出再暂停
    cancelFade();
    float cur = m_audioOutput->volume();
    startFade(cur, 0.0f, 120, [this]() {
        m_player->pause();
        // 暂停后保持输出音量低，等待 resume 渐入
        setOutputVolumeRaw(0.0f);
        qDebug() << "AudioPlayer: 暂停播放 -> QMediaPlayer::pause() done."
            << "qstate(now)=" << int(m_player->playbackState());
        });

    // 观察 80ms 后底层状态
    QTimer::singleShot(80, this, [this]() {
        qDebug() << "AudioPlayer: pause() post-80ms snapshot."
            << "qstate=" << int(m_player->playbackState())
            << "outVol=" << m_audioOutput->volume();
        });
}

void AudioPlayer::resume() {
    qDebug() << "AudioPlayer: resume() called."
        << "qstate(before)=" << int(m_player->playbackState())
        << "outVol=" << m_audioOutput->volume()
        << "userVol=" << m_userVolume;

    cancelFade();
    m_player->play();
    // 渐入
    float cur = m_audioOutput->volume();
    startFade(cur, m_userVolume, 120);
    qDebug() << "AudioPlayer: 恢复播放 -> QMediaPlayer::play() requested.";

    QTimer::singleShot(80, this, [this]() {
        qDebug() << "AudioPlayer: resume() post-80ms snapshot."
            << "qstate=" << int(m_player->playbackState())
            << "outVol=" << m_audioOutput->volume();
        });
}

void AudioPlayer::stop() {
    qDebug() << "AudioPlayer: stop() called."
        << "qstate(before)=" << int(m_player->playbackState())
        << "outVol=" << m_audioOutput->volume();

    // 渐出再停止
    cancelFade();
    float cur = m_audioOutput->volume();
    startFade(cur, 0.0f, 120, [this]() {
        m_player->stop();
        setOutputVolumeRaw(m_userVolume); // 复位到用户音量，下一次 play 再从 0 渐入
        qDebug() << "AudioPlayer: 停止播放 -> QMediaPlayer::stop() done."
            << "qstate(now)=" << int(m_player->playbackState());
        });
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

    m_userVolume = normalizedVolume;

    // 若未在渐变中，直接设置并发出信号
    if (!m_isFading) {
        setOutputVolumeRaw(normalizedVolume);
    }
    emit volumeChanged(clampedVolume);

    qDebug() << "AudioPlayer: 设置音量:" << clampedVolume
        << "qstate=" << int(m_player->playbackState());
}

int AudioPlayer::volume() const {
    return static_cast<int>(m_userVolume * 100.0f);
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
        qDebug() << "AudioPlayer: 播放状态变化:" << int(newState)
            << "qstate(raw)=" << int(state);
    }
    else {
        // 即使逻辑状态未变，也打印底层状态方便定位
        qDebug() << "AudioPlayer: 播放状态回调（未变化）"
            << "logic=" << int(newState)
            << "qstate(raw)=" << int(state);
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

// 私有：渐变
void AudioPlayer::startFade(float from, float to, int durationMs, std::function<void()> done) {
    qDebug() << "AudioPlayer: startFade from=" << from << "to=" << to << "durMs=" << durationMs;
    m_fadeFrom = from;
    m_fadeTo = to;
    m_fadeDurationMs = durationMs;
    m_fadeDone = std::move(done);
    m_isFading = true;
    m_fadeClock.restart();
    if (!m_fadeTimer.isActive()) m_fadeTimer.start();
}

void AudioPlayer::cancelFade() {
    bool active = m_fadeTimer.isActive();
    if (active) m_fadeTimer.stop();
    m_isFading = false;
    m_fadeDone = {};
    qDebug() << "AudioPlayer: cancelFade activeWas=" << active;
}

void AudioPlayer::setOutputVolumeRaw(float normalized) {
    m_audioOutput->setVolume(normalized);
    // 仅在端点打印，避免刷屏
    if (normalized <= 0.001f || normalized >= 0.999f) {
        qDebug() << "AudioPlayer: setOutputVolumeRaw ->" << normalized;
    }
}
