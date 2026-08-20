cmake_minimum_required(VERSION 3.16)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(featcheck
    GUI
    SOURCES
        main.cpp
)

target_link_libraries(featcheck PRIVATE
    Qt::Core
    Qt::Gui
    Qt::LottiePrivate
)
