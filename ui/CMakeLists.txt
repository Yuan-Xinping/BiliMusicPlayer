set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(ui STATIC)

target_sources(ui PRIVATE
    windows/MainWindow.cpp
    windows/MainWindow.ui
    components/PlaybackBar.cpp
    components/HoverButton.cpp
    resources.qrc
    pages/DownloadManagerPage.h 
    pages/DownloadManagerPage.cpp 
    components/DownloadTaskItem.h 
    components/DownloadTaskItem.cpp)

target_link_libraries(ui PUBLIC
    viewmodel
    common
    Qt6::Widgets
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
