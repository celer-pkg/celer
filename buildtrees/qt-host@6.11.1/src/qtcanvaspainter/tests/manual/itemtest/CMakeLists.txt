# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(itemtest LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(itemtest
    SOURCES
        main.cpp
        itemtest.h
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::CanvasPainter
)

qt_internal_add_resource(itemtest "itemtest"
    PREFIX
        /
    BASE
        ../shared
    FILES
        ../shared/quitlogo.png
)

qt_internal_add_resource(itemtest "itemtest_qml"
    PREFIX
        /
    FILES
        main.qml
)
