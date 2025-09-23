add_executable(BiliMusicPlayer
    "main.cpp"
    "BiliMusicPlayerApp.cpp"
    "BiliMusicPlayerApp.h"
    "ServiceRegistry.cpp"
    "ServiceRegistry.h"
)

target_link_libraries(BiliMusicPlayer PRIVATE
    ui
    Qt6::Widgets
)
