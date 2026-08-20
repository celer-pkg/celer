# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(2dpainting_port LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_manual_test(2dpainting_port
    SOURCES
        glwidget.cpp glwidget.h
        helper.cpp helper.h
        main.cpp
        widget.cpp widget.h
        window.cpp window.h
        canvasglwidget.cpp canvasglwidget.h
    LIBRARIES
        Qt::Gui
        Qt::Widgets
        Qt::OpenGLWidgets
        Qt::CanvasPainter
)
