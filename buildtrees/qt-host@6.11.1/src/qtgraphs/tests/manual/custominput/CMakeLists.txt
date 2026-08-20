# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_custominput LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_custominput
    GUI
    SOURCES
        main.cpp
        scatterdatamodifier.cpp scatterdatamodifier.h
)

target_link_libraries(tst_custominput PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Graphs
    Qt::GraphsWidgets
)

set(custominput_resource_files
    "data/data.txt"
)

qt6_add_resources(tst_custominput "custominput"
    PREFIX
        "/"
    FILES
        ${custominput_resource_files}
)
