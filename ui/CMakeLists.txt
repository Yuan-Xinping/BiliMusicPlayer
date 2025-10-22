set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(ui STATIC)

target_sources(ui PRIVATE
    # ========== 窗口 ==========
    windows/MainWindow.cpp
    windows/MainWindow.h
    windows/MainWindow.ui
    
    # ========== 组件 ==========
    components/PlaybackBar.h
    components/PlaybackBar.cpp
    components/HoverButton.h
    components/HoverButton.cpp
    components/DownloadTaskItem.h
    components/DownloadTaskItem.cpp
    
    # ========== 下载页面 ==========
    pages/DownloadManagerPage.h
    pages/DownloadManagerPage.cpp
    
    # ========== 设置页面 ==========
    pages/settings/SettingsPage.h
    pages/settings/SettingsPage.cpp
    pages/settings/DownloadSettingsWidget.h
    pages/settings/DownloadSettingsWidget.cpp
    pages/settings/DatabaseSettingsWidget.h
    pages/settings/DatabaseSettingsWidget.cpp
    pages/settings/ToolsSettingsWidget.h
    pages/settings/ToolsSettingsWidget.cpp
    pages/settings/UISettingsWidget.h
    pages/settings/UISettingsWidget.cpp
    pages/settings/AdvancedSettingsWidget.h
    pages/settings/AdvancedSettingsWidget.cpp

    # ========== 主题管理 ==========
    themes/ThemeManager.cpp
    themes/ThemeManager.h

    # ========== 资源 ==========
    resources.qrc
    themes/themes.qrc
)

find_package(Qt6 REQUIRED COMPONENTS Network)

target_link_libraries(ui PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Network
    common
    viewmodel
    service
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
