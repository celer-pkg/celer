# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlvolume LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlvolume
    GUI
    SOURCES
        datasource.cpp datasource.h
        main.cpp
    )
target_link_libraries(tst_qmlvolume PUBLIC
    Qt::Gui
    Qt::Graphs
    )

set(qmlvolume_resource_files
    "qml/qmlvolume/main.qml"
    )

qt_internal_add_resource(tst_qmlvolume "qmlvolume"
    PREFIX
    "/"
    FILES
    ${qmlvolume_resource_files}
    )
