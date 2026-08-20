# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(desktopservices LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/svg/embedded/desktopservices")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(desktopservices
    contenttab.cpp contenttab.h
    desktopwidget.cpp desktopwidget.h
    linktab.cpp linktab.h
    main.cpp
)

set_target_properties(desktopservices PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(desktopservices PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
)

# Resources:
set(desktopservices_resource_files
    "resources/browser.png"
    "resources/message.png"
    "resources/music.png"
    "resources/photo.png"
)

qt6_add_resources(desktopservices "desktopservices"
    PREFIX
        "/"
    FILES
        ${desktopservices_resource_files}
)

install(TARGETS desktopservices
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
