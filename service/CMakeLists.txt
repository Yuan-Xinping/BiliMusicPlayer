add_library(service STATIC
    "DownloadService.cpp"
    "DownloadService.h"
    "ConcurrentDownloadManager.cpp"
    "ConcurrentDownloadManager.h"
    "DownloadWorker.cpp"
    "DownloadWorker.h"
    "DownloadTaskState.h"
    "ConcurrentDownloadConfig.h"
)

target_link_libraries(service PUBLIC Qt6::Core common data infra)
