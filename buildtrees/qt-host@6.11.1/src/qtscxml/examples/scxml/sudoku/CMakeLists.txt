# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(sudoku LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/scxml/sudoku")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Scxml Widgets)

qt_standard_project_setup()

qt_add_executable(sudoku
    main.cpp
    mainwindow.cpp mainwindow.h
)

set_target_properties(sudoku PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(sudoku PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Scxml
    Qt::Widgets
)

# Resources:
set(sudoku_resource_files
    "data/nearly-solved-sudoku.data"
    "data/sudoku.data"
    "sudoku.js"
)

qt6_add_resources(sudoku "sudoku"
    PREFIX
        "/"
    FILES
        ${sudoku_resource_files}
)

# Statecharts:
qt6_add_statecharts(sudoku
    sudoku.scxml
)

install(TARGETS sudoku
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
