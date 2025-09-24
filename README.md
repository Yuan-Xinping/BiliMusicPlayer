 # BiliMusicPlayer - C++ Rewrite

一个基于 Qt6 和 C++20 的哔哩哔哩音乐播放器，具有现代化的模块化架构设计。

## 🎯 项目概述

BiliMusicPlayer-cpp 是原 Java 版本的完整重写，采用 C++20 和 Qt6 技术栈，提供更好的性能和原生桌面体验。

### 🏗️ 当前状态：Stage 3 - 下载服务完成

✅ **已完成功能**：
- 完整的模块化 CMake 构建系统
- 实体类定义（Song, Playlist）
- 配置管理系统（AppConfig）
- 数据库管理器（DatabaseManager）
- Repository 数据访问层（SongRepository, PlaylistRepository）
- **🔥 完整的下载服务系统**
  - 单个下载服务（DownloadService）
  - 并发下载管理器（ConcurrentDownloadManager）
  - 智能任务调度和状态管理
- 完整的单元测试覆盖

🔄 **进行中**：
- 音频播放服务

⏳ **计划中**：
- 用户界面实现
- 服务集成

## 🏛️ 架构设计

```
BiliMusicPlayer-cpp/
├── app/           # 应用程序入口和服务注册
├── ui/            # 用户界面层 (Qt Widgets)
├── viewmodel/     # 视图模型层 (MVVM架构)
├── service/       # 业务服务层 (下载管理、播放控制)
├── core/          # 核心业务逻辑层
├── data/          # 数据访问层 (Repository模式)
├── infra/         # 基础设施层 (工具类, 外部服务)
├── common/        # 通用模块 (实体类, 配置)
├── tests/         # 测试模块
├── examples/      # 示例程序
└── resources/     # 资源文件
```

### 📦 模块职责

| 模块 | 职责 | 依赖 |
|------|------|------|
| `common` | 基础实体类、配置管理 | Qt::Core |
| `data` | 数据库操作、Repository模式 | common, Qt::Sql |
| `infra` | 外部工具封装、文件系统、进程管理 | common |
| `service` | **业务服务（下载管理、并发控制）** | data, infra |
| `core` | 业务逻辑服务 | service, data |
| `viewmodel` | UI状态管理 | core |
| `ui` | 用户界面 | viewmodel, Qt::Widgets |
| `app` | 程序入口、依赖注入 | ui |

## 🛠️ 技术栈

- **语言**: C++20
- **框架**: Qt 6.x (Core, Widgets, Multimedia, Sql)
- **构建**: CMake 3.16+
- **数据库**: SQLite
- **多线程**: QThreadPool, QRunnable
- **外部工具**: yt-dlp, ffmpeg

## 🚀 快速开始

### 环境要求

- CMake 3.16+
- Qt6 (包含 Widgets, Multimedia, Sql 模块)
- C++20 兼容编译器 (MSVC 2019+, GCC 10+, Clang 10+)
- **yt-dlp** 和 **ffmpeg** 可执行文件

### 构建步骤

```bash
# 1. 克隆项目
git clone https://github.com/Yuan-Xinping/BiliMusicPlayer-cpp.git
cd BiliMusicPlayer-cpp

# 2. 配置构建
cmake -S . -B build

# 3. 编译
cmake --build build --config Debug

# 4. 运行测试
./build/tests/Debug/test_basic.exe             # 基础功能测试
./build/tests/Debug/test_repository.exe        # 数据库测试
./build/tests/Debug/test_downloader.exe        # 下载功能测试
./build/tests/Debug/test_concurrent_download.exe  # 并发下载测试
```

### 运行示例程序

```bash
# 并发下载示例
./build/examples/Debug/concurrent_download_example.exe
```

### Windows 特别说明

确保 Qt6 的 bin 目录在系统 PATH 中，或设置环境变量：
```powershell
$env:PATH += ";C:\Qt\6.9.2\msvc2022_64\bin"
```

## 🧪 测试

项目包含完整的测试套件：

```bash
# 运行所有测试
cd build/tests/Debug

# 基础功能测试
./test_basic.exe

# Repository 层测试
./test_repository.exe

# 二进制文件检测测试
./test_binaries.exe

# 单个下载测试
./test_downloader.exe

# 下载配置测试  
./test_download_config.exe

# 下载服务测试
./test_download_service.exe

# 并发下载测试
./test_concurrent_download.exe
```

### 测试覆盖范围

- ✅ 实体类 (Song, Playlist) 创建和操作
- ✅ 配置管理 (AppConfig) 加载保存
- ✅ 数据库连接和表结构创建
- ✅ 歌曲 CRUD 操作
- ✅ 播放列表管理
- ✅ 数据关联查询
- ✅ **外部工具进程管理**
- ✅ **单个下载功能**
- ✅ **并发下载管理**
- ✅ **下载配置系统**

## 📊 功能特性

### ✅ 已实现功能

#### 🎵 数据管理
- **歌曲管理**: 完整的 CRUD 操作，支持收藏、搜索
- **播放列表**: 创建、编辑播放列表，歌曲关联管理
- **数据库**: 自动创建和管理 SQLite 数据库
- **配置系统**: JSON 配置文件，自动加载默认设置

#### 📥 **下载功能** 🔥
- **单个下载**: 支持单首歌曲下载，多种音质选择
- **并发下载**: 智能并发管理，支持同时下载多个任务
- **下载预设**: 高质量MP3、中等质量MP3、小体积OPUS等预设配置
- **实时进度**: 实时下载进度监控和状态反馈
- **智能去重**: 自动检测并跳过已下载的歌曲
- **自动重试**: 失败任务智能重试机制
- **元数据解析**: 自动提取歌曲标题、艺术家、封面等信息
- **文件管理**: 智能文件命名和临时文件清理
- **统计监控**: 详细的下载统计和性能监控

#### 🔧 基础设施
- **跨平台路径**: 自动检测应用程序路径和数据目录
- **二进制工具检测**: 自动查找 yt-dlp 和 ffmpeg
- **进程管理**: 健壮的外部进程控制和监控
- **多线程**: 基于QThreadPool的高效并发处理
- **错误处理**: 完整的日志记录和错误提示

### ⏳ 计划功能

#### 🎵 播放功能
- [ ] 音频播放控制
- [ ] 播放模式（顺序、随机、单曲循环）
- [ ] 音量控制
- [ ] 播放历史

#### 🎨 用户界面
- [ ] 现代化主界面
- [ ] 音乐库管理界面
- [ ] 下载管理界面
- [ ] 设置界面

#### 📋 下载队列
- [ ] 下载队列持久化
- [ ] 队列优先级管理
- [ ] 下载任务调度

## 🎯 核心组件详解

### 📥 下载服务架构

```cpp
// 单个下载
DownloadService service;
service.downloadSong("BV1xx411c7mD", DownloadOptions::createPreset("high_quality_mp3"));

// 并发下载
ConcurrentDownloadManager manager;
manager.setConfig(ConcurrentDownloadConfig::getAggressive()); // 5个并发
QStringList playlist = {"BV1xx411c7mD", "BV1GJ411x7h7", "BV1ef4y1H7h5"};
manager.addBatchTasks(playlist, DownloadOptions::createPreset("medium_quality_mp3"));
```

### 🔧 下载预设配置

| 预设名称 | 格式 | 质量 | 适用场景 |
|----------|------|------|----------|
| `high_quality_mp3` | MP3 | 320kbps | 高音质收藏 |
| `medium_quality_mp3` | MP3 | 192kbps | 日常听歌 |
| `small_size_opus` | OPUS | 96kbps | 节省空间 |
| `lossless_flac` | FLAC | 无损 | 发烧友专用 |

### 📊 并发下载性能

- **最大并发数**: 1-10 个任务（可配置）
- **并发效率**: 实测 73%+ 的并发效率
- **智能调度**: 失败任务不影响其他任务执行
- **内存安全**: 完善的线程生命周期管理

## 📁 配置文件

应用程序会自动创建配置文件：

- **位置**: `~/BiliMusicPlayer/config.json`
- **数据库**: `%AppData%/BiliMusicPlayer/bili_music_player.db`
- **下载目录**: `~/BiliMusicPlayer_Downloads/`

### 默认配置示例

```json
{
    "downloadPath": "C:/Users/[用户名]/BiliMusicPlayer_Downloads",
    "ffmpegPath": "[应用目录]/bin/ffmpeg.exe",
    "ytDlpPath": "[应用目录]/bin/yt-dlp.exe",
    "maxConcurrentDownloads": 3,
    "downloadRetryCount": 2
}
```

## 🔄 版本历史

### Stage 3 (当前) - **下载服务完成** 🔥
- ✅ 完整的单个下载服务（DownloadService）
- ✅ 高效的并发下载管理器（ConcurrentDownloadManager）
- ✅ 多种下载预设配置
- ✅ 实时进度监控和统计
- ✅ 智能重试和错误处理
- ✅ 元数据自动解析
- ✅ 文件智能命名管理
- ✅ 完整的下载功能测试套件

### Stage 2 - 数据层完成
- ✅ 完整的 Repository 模式实现
- ✅ SQLite 数据库集成
- ✅ 配置管理系统
- ✅ 单元测试框架

### Stage 1 - 项目架构搭建
- ✅ CMake 构建系统
- ✅ 模块化架构设计
- ✅ Qt6 集成配置
- ✅ 基础类定义

## 🚀 使用场景

### 单首歌曲下载
```cpp
DownloadService service;
connect(&service, &DownloadService::downloadCompleted, [](const Song& song) {
    qDebug() << "下载完成:" << song.getTitle();
});
service.downloadSong("BV1xx411c7mD");
```

### 播放列表批量导入
```cpp
ConcurrentDownloadManager manager;
manager.setConfig(ConcurrentDownloadConfig::getDefault()); // 3个并发

// 自动检测已存在歌曲，仅下载新歌曲
QStringList playlistUrls = getPlaylistFromUser();
manager.addBatchTasks(playlistUrls);

// 实时监控下载进度
connect(&manager, &ConcurrentDownloadManager::statisticsUpdated, 
        [](const auto& stats) {
    qDebug() << QString("进度: %1% (%2/%3)")
                .arg(stats.overallProgress * 100, 0, 'f', 1)
                .arg(stats.completedTasks)
                .arg(stats.totalTasks);
});
```

## 🏆 项目亮点

- 🚀 **高性能**: 73%+ 的并发下载效率
- 🛡️ **健壮性**: 完善的错误处理和恢复机制
- 🔧 **可扩展**: 模块化设计，易于扩展新功能
- 📊 **可观测**: 详细的统计和监控信息
- 🎯 **用户友好**: 简单的API接口，丰富的反馈信息
- ⚡ **智能化**: 自动去重、智能重试、动态调度
- 🔒 **线程安全**: 完善的多线程同步机制

---

