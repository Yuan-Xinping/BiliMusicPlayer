add_library(data STATIC
    "DatabaseManager.cpp"
    "DatabaseManager.h"
    "PlaylistRepository.cpp"
    "PlaylistRepository.h"
    "SongRepository.cpp"
    "SongRepository.h"
)

target_link_libraries(data PUBLIC Qt6::Core Qt6::Sql common)