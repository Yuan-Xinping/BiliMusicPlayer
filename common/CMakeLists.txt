add_library(common STATIC
    
    "AppConfig.h"
    "AppConfig.cpp"
    "PlaybackMode.cpp"
    "PlaybackMode.h"
    "entities/Playlist.h"
    "entities/Playlist.cpp"
    "entities/Song.h"
    "entities/Song.cpp"
)

target_link_libraries(common PUBLIC Qt6::Core)
