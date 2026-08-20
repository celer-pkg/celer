# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(embeddedsvgviewer LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/svg/embeddedsvgviewer")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg Widgets)

qt_add_executable(embeddedsvgviewer
    embeddedsvgviewer.cpp embeddedsvgviewer.h
    main.cpp
)

set_target_properties(embeddedsvgviewer PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(embeddedsvgviewer PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Svg
    Qt::Widgets
)

# Resources:
set(embeddedsvgviewer_resource_files
    "files/default.svg"
    "files/v-slider-handle.svg"
)

qt6_add_resources(embeddedsvgviewer "embeddedsvgviewer"
    PREFIX
        "/"
    FILES
        ${embeddedsvgviewer_resource_files}
)

install(TARGETS embeddedsvgviewer
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
