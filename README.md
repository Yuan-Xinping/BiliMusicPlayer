 # 📖 BiliMusicPlayer - README

<div align="center">

![BiliMusicPlayer](https://img.shields.io/badge/BiliMusicPlayer-v1.0.0-FB7299?style=for-the-badge&logo=bilibili)
![Qt](https://img.shields.io/badge/Qt-6.9.2-41CD52?style=for-the-badge&logo=qt)
![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=cplusplus)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

**一款基于 Qt6 的现代化 B 站音乐下载与管理工具**

[功能特性](#-功能特性) • [快速开始](#-快速开始) • [使用指南](#-使用指南) • [开发计划](#-开发计划) • [贡献指南](#-贡献指南)

</div>

---

## 🎯 项目简介

**BiliMusicPlayer** 是一款专为 Bilibili 音乐爱好者打造的跨平台桌面应用，支持从 B 站下载音频、管理本地音乐库、在线播放等功能。

### ✨ 核心亮点

- 🎵 **高品质下载** - 支持 320kbps MP3、原始音质等多种格式
- 📁 **智能管理** - 自动去重、元数据解析、文件规范命名
- 🎨 **现代化 UI** - 深色主题，哔哩哔哩风格设计
- 🔧 **高度可配置** - 自定义下载路径、音质、数据库位置等
- 💾 **SQLite 数据库** - 轻量级本地存储，快速检索

---

## 🚀 功能特性

### ✅ 已实现功能

| 功能模块 | 描述 | 状态 |
|---------|------|------|
| 🎵 **音频下载** | 支持 BV 号/URL 输入，自动下载音频 | ✅ 已完成 |
| 📊 **下载管理** | 实时进度显示、队列管理、历史记录 | ✅ 已完成 |
| 🔍 **智能去重** | 基于 BV 号的重复检测，避免重复下载 | ✅ 已完成 |
| 📝 **元数据解析** | 自动提取标题、作者、时长、封面等信息 | ✅ 已完成 |
| 💾 **数据库存储** | SQLite 本地存储，支持快速查询 | ✅ 已完成 |
| 🎨 **现代化界面** | 哔哩哔哩风格深色主题，流畅动画 | ✅ 已完成 |

### 🚧 计划中功能（v1.1.0）

| 功能模块 | 描述 | 优先级 |
|---------|------|--------|
| ⚙️ **设置页面** | 配置下载路径、音质、数据库位置等 | 🔥 高 |
| 🎵 **音乐播放器** | 本地播放、播放列表、播放控制 | 🔥 高 |
| 🔍 **音乐库管理** | 搜索、分类、标签、播放列表管理 | 🔥 高 |
| 📋 **批量下载** | 支持批量导入 BV 号列表 | 🟡 中 |
| 📱 **跨平台适配** | macOS、Linux 版本适配 | 🔵 低 |

---

## 📦 技术栈

```
前端界面：Qt 6.9.2 (Widgets)
编程语言：C++ 17
数据库：  SQLite 3
音频处理：yt-dlp + FFmpeg
构建工具：CMake 3.20+
编译器：  MSVC 2022 / GCC / Clang
```

### 🏗️ 项目架构

```
BiliMusicPlayer/
├── app/                    # 应用程序入口
│   ├── main.cpp
│   ├── BiliMusicPlayerApp.cpp
│   └── ServiceRegistry.cpp
│
├── ui/                     # 用户界面层
│   ├── pages/              # 主要页面
│   │   ├── DownloadManagerPage.cpp  # 下载管理页
│   │   ├── MusicLibraryPage.cpp     # 音乐库页
│   │   └── SettingsPage.cpp         # 设置页（计划中）
│   ├── components/         # UI 组件
│   │   ├── DownloadTaskItem.cpp     # 下载任务卡片
│   │   └── SideNavigation.cpp       # 侧边导航栏
│   └── MainWindow.cpp      # 主窗口
│
├── viewmodel/              # 视图模型层（MVVM）
│   ├── DownloadViewModel.cpp
│   └── MusicLibraryViewModel.cpp
│
├── service/                # 业务逻辑层
│   ├── DownloadService.cpp          # 下载服务
│   └── MusicPlayerService.cpp       # 播放服务（计划中）
│
├── data/                   # 数据层
│   ├── models/             # 数据模型
│   │   └── Song.cpp
│   ├── repositories/       # 数据访问
│   │   └── SongRepository.cpp
│   └── DatabaseManager.cpp # 数据库管理
│
├── infra/                  # 基础设施层
│   ├── ProcessRunner.cpp   # 进程管理
│   ├── YtDlpClient.cpp     # yt-dlp 客户端
│   └── MetadataParser.cpp  # 元数据解析
│
└── common/                 # 公共组件
    ├── AppConfig.cpp       # 应用配置管理
    └── Logger.cpp          # 日志系统
```

---

## 🛠️ 快速开始

### 📋 环境要求

- **操作系统**: Windows 10/11
- **Qt 版本**: 6.9.2 或更高
- **CMake**: 3.20 或更高
- **编译器**: 
  - Windows: MSVC 2022


### 📥 安装依赖

#### 1. 安装 Qt 6.9.2

**Windows:**
```bash
# 从官网下载在线安装器
https://www.qt.io/download-qt-installer

# 安装时选择组件：
# - Qt 6.9.2 (MSVC 2022 64-bit)
# - Qt Multimedia
# - CMake
```

#### 2. 安装 yt-dlp 和 FFmpeg

**Windows:**
```powershell
# 使用 winget
winget install yt-dlp
winget install FFmpeg

# 或使用 Chocolatey
choco install yt-dlp ffmpeg
```

### 🔨 编译步骤

```bash
# 1. 克隆仓库
git clone -b cpp-rewrite https://github.com/Yuan-Xinping/BiliMusicPlayer
cd BiliMusicPlayer

# 2. 创建构建目录
mkdir build && cd build

# 3. 配置 CMake（根据你的 Qt 安装路径调整）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.9.2/msvc2022_64"

# 4. 编译
cmake --build . --config Release

# 5. 运行
./Release/BiliMusicPlayer.exe  # Windows
```

### ⚡ Visual Studio 配置（推荐 Windows 用户）

1. 安装 **Visual Studio 2022** + **Qt VS Tools** 扩展
2. 打开 `CMakeLists.txt` 作为项目
3. 配置 Qt 路径：**工具 → 选项 → CMake → CMake 设置**
4. 按 `F5` 直接运行

---

## 📖 使用指南

### 1️⃣ 首次启动配置

首次启动时，程序会自动创建配置文件和数据库：

```
Windows:
C:\Users\YourName\BiliMusicPlayer_Downloads\  # 下载目录
C:\Users\YourName\BiliMusicPlayer_Data\       # 数据库目录
```

### 2️⃣ 下载音乐

1. 打开 **下载管理** 页面
2. 输入 B 站视频 **BV 号** 或 **完整 URL**：
   ```
   示例：
   BV1xQ4y1n7iy
   https://www.bilibili.com/video/BV1xQ4y1n7iy
   ```
3. 选择音质（默认：高品质 MP3 320kbps）
4. 点击 **🚀 开始下载**

### 3️⃣ 查看下载进度

- **实时进度条**：显示当前下载百分比
- **队列管理**：查看所有待下载任务
- **历史记录**：查看已完成/失败的任务

### 4️⃣ 打开下载文件夹

点击底部 **📁 打开下载文件夹** 按钮，快速访问下载目录。

### 5️⃣ 音乐库管理

- 自动导入下载的音乐
- 查看歌曲元数据（标题、艺术家、时长）
- 点击播放（需要实现播放器模块）

---

## ⚙️ 配置文件说明

### 应用配置 (`app_config.json`)

```json
{
  "download_path": "C:/Users/YourName/BiliMusicPlayer_Downloads",
  "database_path": "C:/Users/YourName/BiliMusicPlayer_Data/bilimusic.db",
  "yt_dlp_path": "C:/Program Files/yt-dlp/yt-dlp.exe",
  "ffmpeg_path": "C:/Program Files/ffmpeg/bin/ffmpeg.exe",
  "default_quality": "high_quality_mp3",
  "default_format": "mp3"
}
```

### 音质预设说明

| 预设名称 | 音质 | 格式 | 码率 |
|---------|------|------|------|
| `high_quality_mp3` | 高品质 MP3 | mp3 | 320kbps |
| `standard_mp3` | 标准 MP3 | mp3 | 192kbps |
| `best_quality` | 最佳音质 | m4a/opus | 原始 |

---

## 🎨 界面预览

### 下载管理页面
```
┌─────────────────────────────────────────────────────────┐
│  📥 新建下载                                            │
│  ┌───────────────────────────────────────────────────┐  │
│  │ BV号/URL: [BV1xQ4y1n7iy_____________________]     │  │
│  │ 音质选择: [🎵 高品质 MP3 (320kbps) ▼]            │  │
│  │ [🚀 开始下载]  [📋 批量下载]                     │  │
│  └───────────────────────────────────────────────────┘  │
│                                                           │
│  ⏳ 下载队列 | 📋 下载历史                               │
│  ┌───────────────────────────────────────────────────┐  │
│  │ 🎵 【AI艾克】《罗生门(Follow)》完整纯享版          │  │
│  │ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░ 65%                        │  │
│  │ 下载中: 2.5 MB/s | 剩余 3 秒                       │  │
│  └───────────────────────────────────────────────────┘  │
│                                                           │
│  💤 等待任务 | 📁 C:\Users\...\Downloads                │
│  [📁 打开下载文件夹]  [🗑️ 清空历史]                   │
└─────────────────────────────────────────────────────────┘
```

---

## 🗺️ 开发计划

### 📅 版本规划

#### v1.0.0 (当前版本) ✅
- [x] 基础下载功能
- [x] 下载队列管理
- [x] SQLite 数据库集成
- [x] 元数据解析
- [x] 智能去重

#### v1.1.0 (下一版本) 🚧
- [ ] **设置页面** - 重点功能
  - [ ] 默认音质配置
  - [ ] 文件格式选择
  - [ ] 下载路径设置
  - [ ] 数据库路径设置
  - [ ] yt-dlp/FFmpeg 路径配置
  - [ ] 主题切换（亮/暗色）
- [ ] 基础音乐播放器
- [ ] 音乐库搜索功能
- [ ] 音乐播放表

#### v1.2.0 (计划中)
- [ ] 批量下载支持
- [ ] 播放列表管理
- [ ] 快捷键支持

#### v2.0.0
- [ ] 云同步功能
- [ ] linux端适配

---

## 🔧 下一步开发：设置页面

### 设计草图

```cpp
// ui/pages/SettingsPage.h
class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget* parent = nullptr);

private:
    // 下载设置
    QLineEdit* m_downloadPathInput;       // 下载路径
    QComboBox* m_defaultQualityCombo;     // 默认音质
    QComboBox* m_defaultFormatCombo;      // 默认格式
    
    // 数据库设置
    QLineEdit* m_databasePathInput;       // 数据库路径
    
    // 工具路径设置
    QLineEdit* m_ytDlpPathInput;          // yt-dlp 路径
    QLineEdit* m_ffmpegPathInput;         // FFmpeg 路径
    
    // 界面设置
    QComboBox* m_themeCombo;              // 主题选择
    QCheckBox* m_autoStartCheckbox;       // 开机自启动
    
    // 按钮
    QPushButton* m_saveBtn;               // 保存设置
    QPushButton* m_resetBtn;              // 恢复默认
    
    void setupUI();
    void loadCurrentSettings();
    void saveSettings();
    void resetToDefaults();
};
```

### 功能需求清单

#### 1. **下载设置**
- ✅ 默认音质选择（高品质/标准/最佳音质）
- ✅ 默认格式选择（MP3/M4A/FLAC）
- ✅ 下载路径配置（带浏览按钮）
- ⚡ 下载完成后自动播放
- ⚡ 同时下载任务数限制

#### 2. **数据库设置**
- ✅ 数据库路径配置
- ⚡ 数据库备份/恢复
- ⚡ 清空数据库按钮

#### 3. **工具路径设置**
- ✅ yt-dlp 可执行文件路径
- ✅ FFmpeg 可执行文件路径
- ⚡ 自动检测按钮

#### 4. **界面设置**
- ⚡ 主题切换（亮色/暗色）
- ⚡ 字体大小调整
- ⚡ 开机自启动

#### 5. **高级设置**
- ⚡ 代理设置（HTTP/SOCKS5）
- ⚡ 日志级别配置
- ⚡ 缓存清理

---

## 🤝 贡献指南

欢迎贡献代码、报告 Bug 或提出新功能建议！

### 提交 Issue

```markdown
### 问题描述
简要描述遇到的问题

### 重现步骤
1. 打开应用
2. 点击...
3. 看到错误...

### 预期行为
应该发生什么

### 实际行为
实际发生了什么

### 环境信息
- OS: Windows 11
- Qt 版本: 6.9.2
- 应用版本: 1.0.0
```

### 代码规范

- 使用 **4 空格缩进**（Qt 官方风格）
- 类名使用 **大驼峰** (例如：`DownloadService`)
- 成员变量使用 `m_` 前缀 (例如：`m_downloadPath`)
- 私有成员使用 **小驼峰** (例如：`downloadPath`)

---

## 📄 许可证

本项目基于 **MIT License** 开源。

---

## 📞 联系方式

---

## 🙏 致谢

- [Qt](https://www.qt.io/) - 强大的跨平台 UI 框架
- [yt-dlp](https://github.com/yt-dlp/yt-dlp) - 视频下载工具
- [FFmpeg](https://ffmpeg.org/) - 音视频处理库
- [Bilibili](https://www.bilibili.com/) - 灵感来源

---

<div align="center">

**⭐ 如果这个项目对你有帮助，请给一个 Star！⭐**

Made with ❤️ by [ ]

</div>

---

## 🔖 版本历史

### v1.0.0 (2025-10-16)
- 🎉 首次发布
- ✅ 实现基础下载功能
- ✅ 实现下载管理页面
- ✅ 集成 SQLite 数据库
- ✅ 支持元数据解析
- ✅ 实现智能去重

---

**Happy Coding! 🎵**

