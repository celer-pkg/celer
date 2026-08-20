# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_renderslicetoimage LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_renderslicetoimage
    GUI
    SOURCES
        bargraph.cpp bargraph.h
        bargraphmodifier.cpp bargraphmodifier.h
        bargraphwidget.cpp bargraphwidget.h
        main.cpp
        surfacegraph.cpp surfacegraph.h
        surfacegraphmodifier.cpp surfacegraphmodifier.h
        surfacegraphwidget.cpp surfacegraphwidget.h
)

target_link_libraries(tst_renderslicetoimage PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Graphs
    Qt::GraphsWidgets
)

set(renderslicetoimage_resource_files
)

qt6_add_resources(tst_renderslicetoimage "renderslicetoimage"
    PREFIX
        "/"
    FILES
        ${renderslicetoimage_resource_files}
)
