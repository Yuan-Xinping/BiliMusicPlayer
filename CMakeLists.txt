cmake_minimum_required(VERSION 3.16)

project(BiliMusicPlayer LANGUAGES CXX)

# --- Project-wide Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(NOMINMAX)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_VS_GLOBALS 
        "CharacterSet=Unicode"
        "UseOfMfc=false"
    )
endif()

# --- Qt6 Configuration ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
    set(CMAKE_AUTOMOC_OPTIONS "-DQT_NO_KEYWORDS")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Sql)

# --- Add Subdirectories (Modules) ---
add_subdirectory(common)
add_subdirectory(data)
add_subdirectory(infra)
add_subdirectory(core)
add_subdirectory(viewmodel)
add_subdirectory(ui)
add_subdirectory(app)
add_subdirectory(tests)

if(WIN32)
    # 定义源和目标路径
    set(EXTERNAL_BINARIES_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/external_binaries/win")
    set(EXTERNAL_BINARIES_TARGET "${CMAKE_BINARY_DIR}/bin")
    
    # 创建目标目录
    file(MAKE_DIRECTORY ${EXTERNAL_BINARIES_TARGET})
    
    # 复制二进制文件
    if(EXISTS "${EXTERNAL_BINARIES_SOURCE}/yt-dlp.exe")
        configure_file(
            "${EXTERNAL_BINARIES_SOURCE}/yt-dlp.exe"
            "${EXTERNAL_BINARIES_TARGET}/yt-dlp.exe"
            COPYONLY
        )
        message(STATUS "Copied yt-dlp.exe to build directory")
    endif()
    
    if(EXISTS "${EXTERNAL_BINARIES_SOURCE}/ffmpeg.exe")
        configure_file(
            "${EXTERNAL_BINARIES_SOURCE}/ffmpeg.exe"
            "${EXTERNAL_BINARIES_TARGET}/ffmpeg.exe"
            COPYONLY
        )
        message(STATUS "Copied ffmpeg.exe to build directory")
    endif()
endif()