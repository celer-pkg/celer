# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(delayedencoding LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/svg/draganddrop/delayedencoding")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg SvgWidgets Widgets)

qt_add_executable(delayedencoding
    main.cpp
    mimedata.cpp mimedata.h
    sourcewidget.cpp sourcewidget.h
)

set_target_properties(delayedencoding PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(delayedencoding PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Svg
    Qt::SvgWidgets
    Qt::Widgets
)

# Resources:
set(delayedencoding_resource_files
    "images/drag.png"
    "images/example.svg"
)

qt6_add_resources(delayedencoding "delayedencoding"
    PREFIX
        "/"
    FILES
        ${delayedencoding_resource_files}
)

install(TARGETS delayedencoding
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
