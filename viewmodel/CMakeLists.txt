file(GLOB_RECURSE VIEWMODEL_SOURCES "*.cpp" "*.h")

add_library(viewmodel STATIC ${VIEWMODEL_SOURCES})

target_link_libraries(viewmodel PUBLIC
    Qt6::Core
)
