cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(viewmodel STATIC
    DownloadViewModel.cpp
    LibraryViewModel.cpp
)

target_link_libraries(viewmodel PUBLIC 
    Qt6::Core
    Qt6::Widgets
    common
    service
)

target_include_directories(viewmodel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(viewmodel PUBLIC cxx_std_20)
