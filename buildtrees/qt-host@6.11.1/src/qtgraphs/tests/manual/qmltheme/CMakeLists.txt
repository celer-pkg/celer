# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmltheme LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmltheme
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmltheme PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qmltheme_resource_files
    "qml/qmltheme/main.qml"
    "qml/qmltheme/GraphModifiers.qml"
    "qml/qmltheme/BackgroundModifiers.qml"
    "layer_1.png"
)

qt6_add_resources(tst_qmltheme "qmltheme"
    PREFIX
        "/"
    FILES
        ${qmltheme_resource_files}
)
