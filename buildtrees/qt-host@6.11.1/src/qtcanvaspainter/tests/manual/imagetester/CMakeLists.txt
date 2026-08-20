# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(imagetester LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(imagetester
    SOURCES
        main.cpp
        imagetestingitem.cpp
        imagetestingitem.h
        imagetestingrenderer.cpp
        imagetestingrenderer.h
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::CanvasPainter
)

qt_add_resources(imagetester "resources"
    PREFIX
        /
    FILES
        main.qml
)
