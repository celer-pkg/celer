# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(hellorhi LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(hellorhi
    SOURCES
        hellorhi.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::CanvasPainter
)

qt_internal_add_resource(hellorhi "hellorhi"
    PREFIX
        /
    FILES
        qt.png
)
