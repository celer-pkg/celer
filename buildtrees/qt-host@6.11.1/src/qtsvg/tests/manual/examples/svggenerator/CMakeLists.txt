# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(svggenerator LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/svg/svggenerator")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg Widgets)

qt_add_executable(svggenerator
    displaywidget.cpp displaywidget.h
    forms/window.ui
    main.cpp
    window.cpp window.h
)

set_target_properties(svggenerator PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_include_directories(svggenerator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(svggenerator PROPERTIES
                      AUTOUIC_SEARCH_PATHS
                      "${CMAKE_CURRENT_SOURCE_DIR}/forms")
target_link_libraries(svggenerator PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Svg
    Qt::Widgets
)

# Resources:
set(svggenerator_resource_files
    "resources/shapes.dat"
)

qt6_add_resources(svggenerator "svggenerator"
    PREFIX
        "/"
    FILES
        ${svggenerator_resource_files}
)

install(TARGETS svggenerator
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
