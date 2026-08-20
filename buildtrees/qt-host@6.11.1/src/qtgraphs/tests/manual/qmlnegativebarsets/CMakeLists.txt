# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlnegativebarsets LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlnegativebarsets
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlnegativebarsets PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qmlnegativebarsets_resource_files
    "qml/qmlnegativebarsets/main.qml"
)

qt6_add_resources(tst_qmlnegativebarsets "qmlnegativebarsets"
    PREFIX
        "/"
    FILES
        ${qmlnegativebarsets_resource_files}
)
