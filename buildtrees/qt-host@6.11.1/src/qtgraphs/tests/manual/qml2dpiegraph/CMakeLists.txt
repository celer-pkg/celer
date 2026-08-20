# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qml2dpiegraph LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qml2dpiegraph
    GUI
    SOURCES
        main.cpp
)

target_link_libraries(tst_qml2dpiegraph PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qml2dpiegraph_resource_files
    "PieChart.qml"
)

qt_internal_add_resource(tst_qml2dpiegraph "qml2dpiegraph"
    PREFIX
        "/"
    FILES
        ${qml2dpiegraph_resource_files}
)

qt_add_qml_module(tst_qml2dpiegraph
    URI PieGraphTest
    VERSION 1.0
)
