add_library(infra STATIC
    "ProcessRunner.cpp"
    "ProcessRunner.h"
    "YtDlpClient.cpp"
    "YtDlpClient.h"
    "DownloadConfig.h" 
    "DownloadConfig.cpp" 
    "MetadataParser.h"
    "MetadataParser.cpp" 
    )

target_link_libraries(infra PUBLIC
    common
    Qt6::Core
)
