# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    project(tst_qmlsubpieslices LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt_internal_add_manual_test(tst_qmlsubpieslices
    GUI
    SOURCES
        main.cpp
    )

target_link_libraries(tst_qmlsubpieslices PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::Graphs
)

set(qmlsubpieslices_resource_files
    "qml/qmlsubpieslices/main.qml"
)

qt6_add_resources(tst_qmlsubpieslices "qmlsubpieslices"
    PREFIX
        "/"
    FILES
        ${qmlsubpieslices_resource_files}
)
