add_library(common STATIC
    "AppConfig.cpp"
    "AppConfig.h"
    "PlaybackMode.cpp"
    "PlaybackMode.h"
    "entities/Playlist.h"
    "entities/Song.h"
)

target_link_libraries(common PUBLIC Qt6::Core)
