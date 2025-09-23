# BiliMusicPlayer - C++ Rewrite

## 关于此分支

`cpp-rewrite` 是 BiliMusicPlayer C++ 重写计划的第一个里程碑。它使用 **C++20**、**Qt6** 和 **CMake** 从零开始搭建了一个现代化的桌面应用程序框架。

此阶段的目标是：
-   定义清晰的模块化架构。
-   配置好完整的 CMake 构建系统。
-   创建所有核心模块、类和接口的占位符。
-   确保整个空项目可以被成功编译，为后续的功能填充做好准备。

## 架构概览

项目被划分为多个高内聚、低耦合的模块，每个模块都有明确的职责：

-   **`app`**: 程序入口，负责应用的初始化、服务注册和主窗口的创建。
-   **`ui`**: 包含所有的 UI 界面 (`.ui` 文件)、窗口类和自定义控件。
-   **`viewmodel`**: 视图模型层，作为 UI 和业务逻辑之间的桥梁，负责处理 UI 状态和用户交互。
-   **`core`**: 核心业务逻辑层，实现如播放、下载、歌单管理等核心服务。
-   **`data`**: 数据持久化层，负责数据库的交互，提供数据仓储接口（Repository）。
-   **`infra`**: 基础设施层，封装与外部工具（如 `yt-dlp`, `ffmpeg`）的交互、文件系统操作、日志记录等底层功能。
-   **`common`**: 通用模块，定义了项目范围内共享的数据结构（如 `Song`, `Playlist`）和枚举。
-   **`tests`**: 单元测试和集成测试模块。

## 技术栈

-   **语言**: C++20
-   **框架**: Qt 6.x (Core, Widgets, Multimedia, Sql)
-   **构建系统**: CMake (>= 3.16)
-   **编译器**: 任何支持 C++20 的编译器 (MSVC, GCC, Clang)

## 如何构建

在开始之前，请确保你已经安装了 **CMake** 和 **Qt6**（包含 Widgets, Multimedia, Sql 模块）。

1.  **克隆仓库并切换到此分支**:
    ```bash
    git clone https://github.com/Yuan-Xinping/BiliMusicPlayer-cpp.git
    cd BiliMusicPlayer-cpp
    git checkout cpp-rewrite-stage1
    ```

2.  **配置 CMake 项目**:
    在项目根目录运行以下命令，它会创建一个 `build` 目录来存放所有构建产物。
    ```bash
    cmake -S . -B build
    ```
    *如果你在使用 Visual Studio，可以指定生成器，例如：*
    ```bash
    cmake -S . -B build -G "Visual Studio 17 2022"
    ```

3.  **编译项目**:
    ```bash
    cmake --build build
    ```

4.  **运行**:
    编译成功后，可执行文件将位于 `build/app/` 目录下。

## 当前状态

### 已完成 (Stage 1)
-   [x] 完整的 CMake 构建系统配置。
-   [x] 所有模块的目录结构和 `CMakeLists.txt` 文件。
-   [x] 所有核心类和接口的头文件及空的实现。
-   [x] 一个可以显示和关闭的空主窗口 (`MainWindow`)。
-   [x] 项目可以成功编译链接，生成一个可执行文件。

### 未实现
-   所有具体的业务逻辑。
-   服务之间的依赖注入。
-   配置文件的加载与解析。
-   数据库的初始化与连接。
-   任何实际的 UI 设计和功能。

## 下一步 (Stage 2)

-   实现服务注册与依赖注入容器。
-   实现配置文件的加载与应用。
-   初始化数据库连接和表结构。
-   开始填充 `infra` 层的功能，如 `Logger` 和 `ProcessRunner`。
