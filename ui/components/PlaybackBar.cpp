#include "PlaybackBar.h"
#include "HoverButton.h"

#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

PlaybackBar::PlaybackBar(QWidget* parent)
    : QWidget(parent)
    , m_isPlaying(false)
    , m_playMode(0)
    , m_currentPosition(0)
    , m_totalDuration(0)
{
    setupUI();
    setupStyles();
}

void PlaybackBar::setupUI()
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 0, 20, 0);
    mainLayout->setSpacing(20);
    mainLayout->setAlignment(Qt::AlignVCenter);

    // 左侧：歌曲信息区域
    auto* songInfoSection = new QWidget();
    songInfoSection->setFixedWidth(280);
    songInfoSection->setFixedHeight(46);

    auto* songInfoLayout = new QHBoxLayout(songInfoSection);
    songInfoLayout->setContentsMargins(16, 8, 16, 8);
    songInfoLayout->setSpacing(0);

    auto* musicIcon = new QLabel("🎵");
    musicIcon->setStyleSheet("font-size: 20px; color: #FB7299;");
    musicIcon->setFixedWidth(30);
    musicIcon->setAlignment(Qt::AlignCenter);

    auto* textContainer = new QWidget();
    auto* textLayout = new QVBoxLayout(textContainer);
    textLayout->setContentsMargins(12, 0, 0, 0);
    textLayout->setSpacing(0);

    m_songTitle = new QLabel("未播放歌曲");
    m_songTitle->setObjectName("songTitle");

    m_artistName = new QLabel("未知艺术家");
    m_artistName->setObjectName("artistName");

    textLayout->addWidget(m_songTitle);
    textLayout->addWidget(m_artistName);

    songInfoLayout->addWidget(musicIcon);
    songInfoLayout->addWidget(textContainer, 1);

    // 中间：播放控制区域
    auto* controlSection = new QWidget();
    controlSection->setMinimumWidth(400);

    auto* controlLayout = new QVBoxLayout(controlSection);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(4);

    auto* buttonWidget = new QWidget();
    buttonWidget->setFixedHeight(52);

    auto* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(20);

    auto* modeButton = new HoverButton("顺序播放");
    modeButton->setObjectName("modeButton");
    modeButton->setFixedSize(90, 32);
    modeButton->setHoverSize(QSize(96, 36));
    modeButton->setToolTip("当前：顺序播放\n点击切换播放模式");
    m_modeButton = modeButton;

    auto* previousButton = new HoverButton("⏮");
    previousButton->setObjectName("controlButton");
    previousButton->setFixedSize(40, 40);
    previousButton->setHoverSize(QSize(44, 44));
    m_previousBtn = previousButton;

    auto* playPauseButton = new HoverButton("▶");
    playPauseButton->setObjectName("playPauseButton");
    playPauseButton->setFixedSize(52, 52);
    playPauseButton->setHoverSize(QSize(58, 58));
    m_playPauseBtn = playPauseButton;

    auto* nextButton = new HoverButton("⏭");
    nextButton->setObjectName("controlButton");
    nextButton->setFixedSize(40, 40);
    nextButton->setHoverSize(QSize(44, 44));
    m_nextBtn = nextButton;

    auto* playPauseShadow = new QGraphicsDropShadowEffect(this);
    playPauseShadow->setBlurRadius(28);
    playPauseShadow->setOffset(0, 6);
    playPauseShadow->setColor(QColor(251, 114, 153, 90));
    m_playPauseBtn->setGraphicsEffect(playPauseShadow);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_modeButton);
    buttonLayout->addWidget(m_previousBtn);
    buttonLayout->addWidget(m_playPauseBtn);
    buttonLayout->addWidget(m_nextBtn);
    buttonLayout->addStretch();

    auto* progressWidget = new QWidget();
    progressWidget->setFixedHeight(18);

    auto* progressLayout = new QHBoxLayout(progressWidget);
    progressLayout->setContentsMargins(0, 4, 0, 0);
    progressLayout->setSpacing(12);

    m_currentTimeLabel = new QLabel("0:00");
    m_currentTimeLabel->setObjectName("timeLabel");
    m_currentTimeLabel->setFixedWidth(52);
    m_currentTimeLabel->setAlignment(Qt::AlignCenter);

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setObjectName("positionSlider");
    m_positionSlider->setMinimum(0);
    m_positionSlider->setMaximum(100);
    m_positionSlider->setFixedHeight(14);

    m_totalTimeLabel = new QLabel("0:00");
    m_totalTimeLabel->setObjectName("timeLabel");
    m_totalTimeLabel->setFixedWidth(52);
    m_totalTimeLabel->setAlignment(Qt::AlignCenter);

    progressLayout->addWidget(m_currentTimeLabel);
    progressLayout->addWidget(m_positionSlider);
    progressLayout->addWidget(m_totalTimeLabel);

    controlLayout->addWidget(buttonWidget);
    controlLayout->addWidget(progressWidget);

    // 右侧：音量和播放列表
    auto* volumeSection = new QWidget();
    volumeSection->setFixedWidth(200);
    volumeSection->setFixedHeight(46);

    auto* volumeLayout = new QHBoxLayout(volumeSection);
    volumeLayout->setContentsMargins(12, 8, 16, 8);
    volumeLayout->setSpacing(12);

    auto* volumeGroup = new QWidget();
    auto* volumeGroupLayout = new QHBoxLayout(volumeGroup);
    volumeGroupLayout->setContentsMargins(0, 0, 0, 0);
    volumeGroupLayout->setSpacing(8);

    auto* volumeButton = new HoverButton("🔊");
    volumeButton->setObjectName("volumeButton");
    volumeButton->setFixedSize(36, 36);
    volumeButton->setHoverSize(QSize(40, 40));
    m_volumeBtn = volumeButton;

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setObjectName("volumeSlider");
    m_volumeSlider->setMinimum(0);
    m_volumeSlider->setMaximum(100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setFixedWidth(90);
    m_volumeSlider->setFixedHeight(16);

    volumeGroupLayout->addWidget(m_volumeBtn);
    volumeGroupLayout->addWidget(m_volumeSlider);

    auto* playlistButton = new HoverButton("📃");
    playlistButton->setObjectName("playlistButton");
    playlistButton->setToolTip("播放列表");
    playlistButton->setFixedSize(36, 36);
    playlistButton->setHoverSize(QSize(40, 40));
    m_playlistBtn = playlistButton;

    volumeLayout->addWidget(volumeGroup);
    volumeLayout->addStretch();
    volumeLayout->addWidget(m_playlistBtn);

    mainLayout->addWidget(songInfoSection);
    mainLayout->addWidget(controlSection, 1);
    mainLayout->addWidget(volumeSection);

    setFixedHeight(90);

    connect(m_playPauseBtn, &QPushButton::clicked, this, &PlaybackBar::playPauseClicked);
    connect(m_previousBtn, &QPushButton::clicked, this, &PlaybackBar::previousClicked);
    connect(m_nextBtn, &QPushButton::clicked, this, &PlaybackBar::nextClicked);
    connect(m_positionSlider, &QSlider::valueChanged, this, &PlaybackBar::onPositionSliderChanged);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &PlaybackBar::onVolumeSliderChanged);
    connect(m_modeButton, &QPushButton::clicked, this, &PlaybackBar::onModeButtonClicked);
}

void PlaybackBar::setupStyles()
{
}

void PlaybackBar::setSong(const Song& song)
{
    m_currentSong = song;
    m_songTitle->setText(song.getTitle());
    m_artistName->setText(song.getArtist());
}

void PlaybackBar::setDuration(int seconds)
{
    m_totalDuration = seconds;
    m_positionSlider->setMaximum(seconds);
    m_totalTimeLabel->setText(formatTime(seconds));
}

void PlaybackBar::setPosition(int seconds)
{
    m_currentPosition = seconds;
    m_positionSlider->setValue(seconds);
    m_currentTimeLabel->setText(formatTime(seconds));
}

void PlaybackBar::setVolume(int volume)
{
    m_volumeSlider->setValue(volume);

    if (volume == 0) {
        m_volumeBtn->setText("🔇");
    }
    else if (volume < 33) {
        m_volumeBtn->setText("🔈");
    }
    else if (volume < 66) {
        m_volumeBtn->setText("🔉");
    }
    else {
        m_volumeBtn->setText("🔊");
    }
}

void PlaybackBar::setPlaybackState(bool isPlaying)
{
    m_isPlaying = isPlaying;

    if (isPlaying) {
        m_playPauseBtn->setText("⏸");
        qDebug() << "切换到暂停图标";
    }
    else {
        m_playPauseBtn->setText("▶");
        qDebug() << "切换到播放图标";
    }

    m_playPauseBtn->update();
}

void PlaybackBar::setPlayMode(int mode)
{
    if (mode < 0) mode = 0;
    if (mode > 3) mode = 3;
    if (m_playMode == mode) return;
    m_playMode = mode;
    updateModeButtonDisplay();
}

void PlaybackBar::onPositionSliderChanged(int value)
{
    if (value != m_currentPosition) {
        m_currentPosition = value;
        m_currentTimeLabel->setText(formatTime(value));
        emit positionChanged(value);
    }
}

void PlaybackBar::onVolumeSliderChanged(int value)
{
    setVolume(value);
    emit volumeChanged(value);
}

void PlaybackBar::onModeButtonClicked()
{
    m_playMode = (m_playMode + 1) % 4;  // 0,1,2,3 循环切换
    updateModeButtonDisplay();
    emit playModeChanged(m_playMode);
}

void PlaybackBar::updateModeButtonDisplay()
{
    QString modeText;
    QString tooltip;
    QString nextMode;

    switch (m_playMode) {
    case 0:  // 顺序播放
        modeText = "顺序播放";
        nextMode = "随机播放";
        break;
    case 1:  // 随机播放
        modeText = "随机播放";
        nextMode = "单曲循环";
        break;
    case 2:  // 单曲循环
        modeText = "单曲循环";
        nextMode = "列表循环";
        break;
    case 3:  // 列表循环
        modeText = "列表循环";
        nextMode = "顺序播放";
        break;
    }

    tooltip = QString("当前：%1\n点击切换到：%2").arg(modeText, nextMode);

    m_modeButton->setText(modeText);
    m_modeButton->setToolTip(tooltip);

    qDebug() << "播放模式切换为：" << modeText;
}

void PlaybackBar::updateTimeLabels()
{
    m_currentTimeLabel->setText(formatTime(m_currentPosition));
    m_totalTimeLabel->setText(formatTime(m_totalDuration));
}

QString PlaybackBar::formatTime(int seconds) const
{
    int mins = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(mins).arg(secs, 2, 10, QChar('0'));
}
