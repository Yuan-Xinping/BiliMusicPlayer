add_library(ui STATIC
    "windows/MainWindow.cpp"
    "windows/MainWindow.h"
    "windows/MainWindow.ui"
    "resources.qrc"
)

target_link_libraries(ui PUBLIC
    viewmodel
    Qt6::Widgets
)
