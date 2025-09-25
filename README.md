 # BiliMusicPlayer - C++ Rewrite

一个基于 Qt6 和 C++20 的哔哩哔哩音乐播放器。

## 🎯 项目概述

采用 C++20 和 Qt6 技术栈的音乐播放器，提供下载和播放功能。

### 🏗️ 当前状态：Phase 4.2 - 智能播放系统完成

✅ **已完成**：
- 模块化 CMake 构建系统
- 数据库管理（SQLite）
- **下载功能**：单个下载、并发下载、进度监控
- **播放功能**：音频播放、播放列表、播放模式
- **智能功能**：播放队列、播放历史、智能推荐

🔄 **进行中**：用户界面实现

## 🛠️ 技术栈

- **语言**: C++20
- **框架**: Qt 6.x (Core, Widgets, Multimedia, Sql)
- **构建**: CMake 3.16+
- **数据库**: SQLite
- **外部工具**: yt-dlp, ffmpeg

## 🚀 快速开始

### 环境要求
- CMake 3.16+
- Qt6 (Widgets, Multimedia, Sql 模块)
- C++20 编译器
- yt-dlp 和 ffmpeg

### 构建步骤
```bash
git clone https://github.com/Yuan-Xinping/BiliMusicPlayer-cpp.git
cd BiliMusicPlayer-cpp
cmake -S . -B build
cmake --build build --config Debug
./build/app/Debug/BiliMusicPlayer.exe
```

## 📊 主要功能

### 📥 下载功能
- 单个/批量下载
- 多种音质选项
- 实时进度显示
- 并发下载管理

### 🎵 播放功能  
- 基础播放控制
- 多种播放模式
- 播放列表管理
- 音量和进度控制

### 🧠 智能功能
- 播放队列
- 播放历史记录
- 智能推荐
- 播放统计

## 🔧 使用示例

### 播放控制
```cpp
PlaybackService& player = PlaybackService::instance();
player.playSong(song);
player.setPlaybackMode(PlaybackMode::Shuffle);
```

### 下载管理
```cpp
DownloadService service;
service.downloadSong("BV1xx411c7mD");
```

## 📁 目录结构
```
├── app/           # 程序入口
├── core/          # 核心服务
├── service/       # 业务服务
├── data/          # 数据访问
├── common/        # 通用模块
└── infra/         # 基础设施
```

## 🔄 版本历史

- **Phase 4.2** - 智能播放系统完成
- **Phase 4.1** - 音频播放系统完成  
- **Stage 3** - 下载服务完成
- **Stage 2** - 数据层完成
- **Stage 1** - 项目架构搭建

---

