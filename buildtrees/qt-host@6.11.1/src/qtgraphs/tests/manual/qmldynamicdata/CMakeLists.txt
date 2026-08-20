# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmldynamicdata LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(tst_qmldynamicdata
    GUI
    SOURCES
        main.cpp
    )
target_link_libraries(tst_qmldynamicdata PUBLIC
    Qt::Gui
    Qt::Graphs
    )

set(qmldynamicdata_resource_files
    "qml/qmldynamicdata/main.qml"
    )

qt_internal_add_resource(tst_qmldynamicdata "qmldynamicdata"
    PREFIX
    "/"
    FILES
    ${qmldynamicdata_resource_files}
    )
