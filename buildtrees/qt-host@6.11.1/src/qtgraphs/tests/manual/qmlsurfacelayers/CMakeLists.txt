# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlsurfacelayers LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlsurfacelayers
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlsurfacelayers PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qmlsurfacelayers_resource_files
    "qml/qmlsurfacelayers/main.qml"
    "qml/qmlsurfacelayers/ResizableRect.qml"
)

qt6_add_resources(tst_qmlsurfacelayers "qmlsurfacelayers"
    PREFIX
        "/"
    FILES
        ${qmlsurfacelayers_resource_files}
)
set(qmlsurfacelayers1_resource_files
    "layer_1.png"
    "layer_2.png"
    "layer_3.png"
)

qt6_add_resources(tst_qmlsurfacelayers "qmlsurfacelayers1"
    PREFIX
        "/heightmaps"
    FILES
        ${qmlsurfacelayers1_resource_files}
)
