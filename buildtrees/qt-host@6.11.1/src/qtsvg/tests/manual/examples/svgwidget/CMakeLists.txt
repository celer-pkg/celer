# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(svgwidget LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/svg/svgwidget")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg SvgWidgets Widgets)

qt_add_executable(svgwidget
    main.cpp
)

set_target_properties(svgwidget PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(svgwidget PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Svg
    Qt::SvgWidgets
    Qt::Widgets
)

# Resources:
set(svgwidget_resource_files
    "files/bubbles.svg"
    "files/spheres.svg"
)

qt6_add_resources(svgwidget "svgwidget"
    PREFIX
        "/"
    FILES
        ${svgwidget_resource_files}
)

install(TARGETS svgwidget
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
