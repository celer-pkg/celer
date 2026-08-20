# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if (NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_manual_dialogAsPopupWindow LANGUAGES C CXX)
    find_package(Qt6BuildInternals COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(tst_manual_dialogAsPopupWindow
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::QuickControls2
)

set(qml_resource_files
    "Main.qml"
)

qt_internal_add_resource(tst_manual_dialogAsPopupWindow "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
