# BiliMusicPlayer

## 项目描述

BiliMusicPlayer 是一款基于 JavaFX 开发的音乐播放器，专注于从 Bilibili 平台下载和播放音乐。该项目整合了 yt-dlp 和 ffmpeg 工具用于媒体资源的获取与处理，使用 VLCJ 作为底层媒体播放引擎，提供了简洁直观的用户界面和丰富的音乐管理功能。

主要功能包括：

- 从 Bilibili 下载音频资源（支持 BV 号或视频链接）
- 本地音乐库管理与播放
- 播放列表创建与管理
- 自定义下载路径设置
- 外部工具（yt-dlp/ffmpeg）配置与检测

## 技术栈

- **框架**：JavaFX（UI 开发）
- **构建工具**：Maven
- **媒体播放**：VLCJ（基于 VLC 媒体框架）
- **下载工具**：yt-dlp + ffmpeg
- **数据存储**：SQLite（播放列表与元数据管理）
- **JSON 处理**：Jackson
- **打包工具**：Launch4j（生成 Windows 可执行文件）

---

## 下载与安装 (推荐)

我们为您提供了**一站式**的 Windows 安装包，**内置了所有必需的运行时和工具**，让您无需额外配置即可轻松使用。

1. **访问发布页面**：请前往 [GitHub Releases 页面](https://github.com/Yuan-Xinping/BiliMusicPlayer/releases/latest) 下载最新版本的 `BiliMusicPlayer_Setup.exe` 安装包。
2. **直接下载 (v1.0.0)**：
   - BiliMusicPlayer_Setup_v1.0.0.exe
3. **运行安装包**：下载完成后，双击运行 `.exe` 安装包，按照提示完成安装。

**安装包内置组件：**

- Java 8 运行时环境 (JRE)
- yt-dlp 下载工具
- ffmpeg 媒体处理工具
- VLC 媒体播放器核心库（用于 VLCJ 播放）

---

## 系统要求

- **操作系统**：Windows 7 或更高版本。

---

## 从源码构建与运行

如果您是开发者或希望从源码编译并运行项目，请遵循以下步骤：

### 前提条件 (针对源码构建)

- **Java 8 JDK**：确保您的系统已安装 Java 8 JDK。
- **Maven**：用于项目构建。
- **VLC 媒体播放器**：VLCJ 依赖 VLC 媒体框架进行媒体解码，请确保已正确安装 VLC 播放器。

### 构建步骤

1. **克隆项目到本地**
   
   ```bash
   git clone https://github.com/Yuan-Xinping/BiliMusicPlayer.git
   cd BiliMusicPlayer
   ```
2. **使用 Maven 构建项目**
   
   ```bash
   mvn clean package
   ```
3. **运行生成的可执行文件**
   
   ```bash
   target/BiliMusicPlayer.exe
   ```

---

## 配置说明

首次启动时，程序会自动检测并配置所需的外部工具。**对于安装包用户，yt-dlp 和 ffmpeg 已内置并默认使用。**

- 下载目录默认设置为 `用户主目录/BiliMusicPlayer_Downloads`

您也可以通过设置界面自定义：

- 下载保存路径
- yt-dlp 可执行文件路径 (仅当您希望使用自定义版本时)
- ffmpeg 可执行文件路径 (仅当您希望使用自定义版本时)

## 功能使用

1. **下载音乐**：输入 Bilibili 视频 BV 号或链接，点击下载按钮
2. **管理本地音乐**：在 "本地音乐" 标签页浏览和播放已下载的音乐
3. **创建播放列表**：通过 "播放列表" 功能创建自定义歌单
4. **查看下载进度**：在 "下载管理" 页面监控当前下载任务

## 项目结构

- `src/main/java`：主要源代码
  - `controller`：UI 控制器
  - `model`：数据模型
  - `service`：业务逻辑服务
  - `util`：工具类
- `src/main/resources`：资源文件（FXML、CSS、图标等）
- `external_binaries`：外部工具目录
  - `win`：包含 Windows 平台所需的 yt-dlp 和 ffmpeg 可执行文件
- `pom.xml`：Maven 配置文件

## 注意事项

- 下载功能依赖网络连接和 Bilibili 平台的访问权限。
- 大型播放列表可能需要更长的加载时间。

## 版本信息

当前版本：1.0.0


