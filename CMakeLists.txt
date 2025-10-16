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

find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Multimedia
    Sql
)

message(STATUS "Qt6_FOUND: ${Qt6_FOUND}")
message(STATUS "Qt6_VERSION: ${Qt6_VERSION}")

# --- Add Subdirectories (Modules) ---
add_subdirectory(common)
add_subdirectory(data)
add_subdirectory(infra)
add_subdirectory(service)
add_subdirectory(viewmodel)
add_subdirectory(ui)
add_subdirectory(app)

if(WIN32)
    set(EXTERNAL_BINARIES_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/external_binaries/win")
    set(EXTERNAL_BINARIES_TARGET "${CMAKE_BINARY_DIR}/bin")
    
    file(MAKE_DIRECTORY ${EXTERNAL_BINARIES_TARGET})
    
    set(BINARY_FILES "yt-dlp.exe" "ffmpeg.exe")
    
    foreach(BINARY_FILE ${BINARY_FILES})
        if(EXISTS "${EXTERNAL_BINARIES_SOURCE}/${BINARY_FILE}")
            configure_file(
                "${EXTERNAL_BINARIES_SOURCE}/${BINARY_FILE}"
                "${EXTERNAL_BINARIES_TARGET}/${BINARY_FILE}"
                COPYONLY
            )
            message(STATUS "Copied ${BINARY_FILE} to ${EXTERNAL_BINARIES_TARGET}")
        else()
            message(WARNING "Binary file not found: ${EXTERNAL_BINARIES_SOURCE}/${BINARY_FILE}")
        endif()
    endforeach()
    
    add_custom_target(copy_binaries ALL
        COMMENT "Ensuring external binaries are up to date"
    )
    
    foreach(BINARY_FILE ${BINARY_FILES})
        if(EXISTS "${EXTERNAL_BINARIES_SOURCE}/${BINARY_FILE}")
            add_custom_command(TARGET copy_binaries POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${EXTERNAL_BINARIES_SOURCE}/${BINARY_FILE}"
                "${EXTERNAL_BINARIES_TARGET}/${BINARY_FILE}"
                COMMENT "Updating ${BINARY_FILE}"
            )
        endif()
    endforeach()
endif()
