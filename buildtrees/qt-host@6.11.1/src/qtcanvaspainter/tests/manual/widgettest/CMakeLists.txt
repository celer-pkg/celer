# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(widgettest LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(widgettest
    SOURCES
        widgettest.cpp
    LIBRARIES
        Qt::Gui
        Qt::Widgets
        Qt::CanvasPainter
)

qt_internal_add_resource(widgettest "widgettest"
    PREFIX
        /
    BASE
        ../shared
    FILES
        ../shared/quitlogo.png
)
