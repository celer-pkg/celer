# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlcustominput LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlcustominput
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlcustominput PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qmlcustominput_resource_files
    "qml/qmlcustominput/Data.qml"
    "qml/qmlcustominput/main.qml"
)

qt6_add_resources(tst_qmlcustominput "qmlcustominput"
    PREFIX
        "/"
    FILES
        ${qmlcustominput_resource_files}
)
