add_executable(BiliMusicPlayer
    "main.cpp"
    "BiliMusicPlayerApp.cpp"
    "ServiceRegistry.cpp"
)

if(MSVC)
    target_compile_options(BiliMusicPlayer PRIVATE /utf-8)
endif()

set_target_properties(BiliMusicPlayer PROPERTIES
    AUTOMOC ON
)

target_link_libraries(BiliMusicPlayer PRIVATE
    ui
    service
    viewmodel
    Qt6::Widgets
    Qt6::Core
)
