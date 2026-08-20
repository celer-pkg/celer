# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(textobject LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/svg/richtext/textobject")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Svg Widgets)

qt_add_executable(textobject
    main.cpp
    svgtextobject.cpp svgtextobject.h
    window.cpp window.h
)

set_target_properties(textobject PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(textobject PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Svg
    Qt::Widgets
)

# Resources:
set(resources_resource_files
    "files/heart.svg"
)

qt6_add_resources(textobject "resources"
    PREFIX
        "/"
    FILES
        ${resources_resource_files}
)

install(TARGETS textobject
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
