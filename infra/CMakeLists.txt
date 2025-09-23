add_library(infra STATIC
    "FfmpegClient.cpp"
    "FfmpegClient.h"
    "FileSystemHelper.cpp"
    "FileSystemHelper.h"
    "Logger.cpp"
    "Logger.h"
    "ProcessRunner.cpp"
    "ProcessRunner.h"
    "YtDlpClient.cpp"
    "YtDlpClient.h"
)

target_link_libraries(infra PUBLIC
    common
    Qt6::Core
)
