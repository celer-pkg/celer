# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(graphprinting LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui GuiPrivate Quick Graphs PrintSupport)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(graphprinting
    main.cpp
)
set_target_properties(graphprinting PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(graphprinting PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Graphs
    Qt::PrintSupport
    Qt::GuiPrivate
)

qt6_add_qml_module(graphprinting
    URI GraphPrintingExample
    SOURCES
        graphprinter.h graphprinter.cpp
    QML_FILES
        qml/graphprinting/Main.qml
        qml/graphprinting/Graph2D.qml
        qml/graphprinting/Graph3D.qml
    RESOURCES
        qml/graphprinting/box_left.svg
        qml/graphprinting/box_left_fill.svg
        qml/graphprinting/documents.svg
        qml/graphprinting/documents_fill.svg
        qml/graphprinting/flatten.svg
        qml/graphprinting/flatten_square_fill.svg
        qml/graphprinting/folder.svg
        qml/graphprinting/folder_fill.svg
        qml/graphprinting/print.svg
        qml/graphprinting/print_fill.svg
)

install(TARGETS graphprinting
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET graphprinting
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})
