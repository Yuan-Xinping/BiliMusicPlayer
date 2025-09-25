// common/PlaybackState.h
#pragma once

enum class PlaybackState {
    Stopped,    // 停止状态
    Playing,    // 正在播放
    Paused,     // 暂停状态
    Loading,    // 加载中
    Error       // 错误状态
};
