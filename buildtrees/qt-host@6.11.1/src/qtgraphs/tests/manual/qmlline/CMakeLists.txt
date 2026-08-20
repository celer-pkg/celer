# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmllines LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmllines
    GUI
    SOURCES
    main.cpp
)

target_link_libraries(tst_qmllines PUBLIC
    Qt::Quick
    Qt::Graphs
)

set(qmllines_resource_files
    "qml/main.qml"
)

qt_internal_add_resource(tst_qmllines "qmllines"
    PREFIX
    "/"
    FILES
    ${qmllines_resource_files}
)

qt_add_qml_module(tst_qmllines
    URI qmllines
    VERSION 1.0
    QML_FILES qml/main.qml
)
