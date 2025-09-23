cmake_minimum_required(VERSION 3.16)

project(BiliMusicPlayer LANGUAGES CXX)

# --- Project-wide Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# --- Qt6 Configuration ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Sql)

# --- Add Subdirectories (Modules) ---
# Order matters for clarity, but CMake resolves the dependency graph.
add_subdirectory(common)
add_subdirectory(data)
add_subdirectory(infra)
add_subdirectory(core)
add_subdirectory(viewmodel)
add_subdirectory(ui)
add_subdirectory(app)
add_subdirectory(tests)
