# BiliMusicPlayer - C++ Rewrite

一个基于 Qt6 和 C++20 的哔哩哔哩音乐播放器，具有现代化的模块化架构设计。

## 🎯 项目概述

BiliMusicPlayer-cpp 是原 Java 版本的完整重写，采用 C++20 和 Qt6 技术栈，提供更好的性能和原生桌面体验。

### 🏗️ 当前状态：Stage 2 - 数据层与基础服务完成

✅ **已完成功能**：
- 完整的模块化 CMake 构建系统
- 实体类定义（Song, Playlist）
- 配置管理系统（AppConfig）
- 数据库管理器（DatabaseManager）
- Repository 数据访问层（SongRepository, PlaylistRepository）
- 完整的单元测试覆盖

🔄 **进行中**：
- 下载服务实现
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
├── core/          # 核心业务逻辑层
├── data/          # 数据访问层 (Repository模式)
├── infra/         # 基础设施层 (工具类, 外部服务)
├── common/        # 通用模块 (实体类, 配置)
├── tests/         # 测试模块
└── resources/     # 资源文件
```

### 📦 模块职责

| 模块 | 职责 | 依赖 |
|------|------|------|
| `common` | 基础实体类、配置管理 | Qt::Core |
| `data` | 数据库操作、Repository模式 | common, Qt::Sql |
| `infra` | 外部工具封装、文件系统 | common |
| `core` | 业务逻辑服务 | data, infra |
| `viewmodel` | UI状态管理 | core |
| `ui` | 用户界面 | viewmodel, Qt::Widgets |
| `app` | 程序入口、依赖注入 | ui |

## 🛠️ 技术栈

- **语言**: C++20
- **框架**: Qt 6.x (Core, Widgets, Multimedia, Sql)
- **构建**: CMake 3.16+
- **数据库**: SQLite
- **外部工具**: yt-dlp, ffmpeg

## 🚀 快速开始

### 环境要求

- CMake 3.16+
- Qt6 (包含 Widgets, Multimedia, Sql 模块)
- C++20 兼容编译器 (MSVC 2019+, GCC 10+, Clang 10+)

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
./build/tests/Debug/test_basic.exe        # 基础功能测试
./build/tests/Debug/test_repository.exe   # 数据库测试
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
```

### 测试覆盖范围

- ✅ 实体类 (Song, Playlist) 创建和操作
- ✅ 配置管理 (AppConfig) 加载保存
- ✅ 数据库连接和表结构创建
- ✅ 歌曲 CRUD 操作
- ✅ 播放列表管理
- ✅ 数据关联查询

## 📊 功能特性

### 已实现功能

#### 🎵 数据管理
- **歌曲管理**: 完整的 CRUD 操作，支持收藏、搜索
- **播放列表**: 创建、编辑播放列表，歌曲关联管理
- **数据库**: 自动创建和管理 SQLite 数据库
- **配置系统**: JSON 配置文件，自动加载默认设置

#### 🔧 基础设施
- **跨平台路径**: 自动检测应用程序路径和数据目录
- **二进制工具检测**: 自动查找 yt-dlp 和 ffmpeg
- **错误处理**: 完整的日志记录和错误提示

### 计划功能

#### 📥 下载功能
- [ ] B站视频音频提取
- [ ] 批量下载管理
- [ ] 下载进度显示
- [ ] 音频格式转换

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

## 📁 配置文件

应用程序会自动创建配置文件：

- **位置**: `~/BiliMusicPlayer/config.json`
- **数据库**: `%AppData%/BiliMusicPlayer/bili_music_player.db`

### 默认配置示例

```json
{
    "downloadPath": "C:/Users/[用户名]/BiliMusicPlayer_Downloads",
    "ffmpegPath": "[应用目录]/bin/ffmpeg.exe",
    "ytDlpPath": "[应用目录]/bin/yt-dlp.exe"
}
```

## 🔄 版本历史

### Stage 2 (当前) - 数据层完成
- ✅ 完整的 Repository 模式实现
- ✅ SQLite 数据库集成
- ✅ 配置管理系统
- ✅ 单元测试框架

### Stage 1 - 项目架构搭建
- ✅ CMake 构建系统
- ✅ 模块化架构设计
- ✅ Qt6 集成配置
- ✅ 基础类定义