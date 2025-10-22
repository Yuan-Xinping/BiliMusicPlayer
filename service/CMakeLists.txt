# service/CMakeLists.txt
add_library(service STATIC
    "DownloadService.cpp"
    "DownloadService.h"
    "ConcurrentDownloadManager.cpp"
    "ConcurrentDownloadManager.h"
    "DownloadWorker.cpp"
    "DownloadWorker.h"
    "DownloadTaskState.h"
    "ConcurrentDownloadConfig.h"
    
    # 播放相关文件
    "AudioPlayer.cpp"
    "AudioPlayer.h"
    "PlaylistManager.cpp"
    "PlaylistManager.h"
    "PlaybackService.cpp"  
    "PlaybackHistory.h" 
    "PlaybackHistory.cpp" 
    "PlaybackQueue.h" 
    "PlaybackQueue.cpp" 

    # 音乐库服务
    "LibraryService.h"
    "LibraryService.cpp"
)

target_link_libraries(service PUBLIC 
    Qt6::Core 
    Qt6::Multimedia
    common 
    data 
    infra
)

if(TARGET Qt6::Multimedia)
    message(STATUS "service module: Qt6::Multimedia target found")
else()
    message(WARNING "service module: Qt6::Multimedia target NOT found")
endif()
