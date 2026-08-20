# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(svgviewer LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg Widgets)

qt_standard_project_setup()

qt_add_executable(svgviewer
    exportdialog.cpp exportdialog.h
    main.cpp
    mainwindow.cpp mainwindow.h
    mainwindow.ui
    svgview.h svgview.cpp
)

qt_add_resources(svgviewer "files"
    FILES
        files/abstract_tree.svg
        files/black.png
        files/white.png
        files/chequered.png
)

set_target_properties(svgviewer PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(svgviewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    Qt6::Widgets
)
