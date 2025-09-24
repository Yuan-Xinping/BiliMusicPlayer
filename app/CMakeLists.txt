add_executable(BiliMusicPlayer
    "main.cpp"
    "BiliMusicPlayerApp.cpp"
    "BiliMusicPlayerApp.h"
    "ServiceRegistry.cpp"
    "ServiceRegistry.h"
)

set_target_properties(BiliMusicPlayer PROPERTIES
    AUTOMOC ON
)

target_link_libraries(BiliMusicPlayer PRIVATE
    ui
    Qt6::Widgets
    Qt6::Core
)
