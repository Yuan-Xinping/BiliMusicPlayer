file(GLOB_RECURSE CORE_SOURCES "*.cpp" "*.h")

add_library(core STATIC ${CORE_SOURCES})

target_link_libraries(core PUBLIC
    data
    infra
    Qt6::Core
)
