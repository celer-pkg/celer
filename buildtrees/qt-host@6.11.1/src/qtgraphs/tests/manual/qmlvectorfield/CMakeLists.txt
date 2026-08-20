# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlvectorfield LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlvectorfield
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlvectorfield PUBLIC
    Qt::Gui
    Qt::Graphs
    Qt::GraphsWidgets
    )

qt_internal_add_resource(tst_qmlvectorfield "qmlvectorfield"
    PREFIX
    "/"
    FILES
    "qml/qmlvectorfield/main.qml"
    )
