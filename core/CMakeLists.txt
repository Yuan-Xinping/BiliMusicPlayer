# core/CMakeLists.txt
add_library(core STATIC
    "PlaybackService.h"
)

# 设置目标属性
target_include_directories(core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖库
target_link_libraries(core PUBLIC 
    Qt6::Core
    Qt6::Multimedia
    common
    service
    data
)

# 编译器特性要求
target_compile_features(core PUBLIC cxx_std_17)

# 设置编译器警告级别
if(MSVC)
    target_compile_options(core PRIVATE /W4)
else()
    target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 预编译头文件支持 (可选)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    target_precompile_headers(core PRIVATE
        <QObject>
        <QList>
        <QString>
    )
endif()

# 设置输出目录
set_target_properties(core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Debug 配置的额外设置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(core PRIVATE 
        DEBUG_BUILD=1
        QT_QML_DEBUG=1
    )
endif()
