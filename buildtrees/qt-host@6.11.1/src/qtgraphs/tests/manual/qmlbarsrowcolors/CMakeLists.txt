# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlbarsrowcolors LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlbarsrowcolors
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlbarsrowcolors PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qmlbarsrowcolors_resource_files
    "qml/qmlbarsrowcolors/Axes.qml"
    "qml/qmlbarsrowcolors/Data.qml"
    "qml/qmlbarsrowcolors/main.qml"
)

qt6_add_resources(tst_qmlbarsrowcolors "qmlbarsrowcolors"
    PREFIX
        "/"
    FILES
        ${qmlbarsrowcolors_resource_files}
)
