# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(colorpaletteserver LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/httpserver/${PROJECT_NAME}")

find_package(Qt6 REQUIRED COMPONENTS HttpServer Gui Concurrent)

qt_standard_project_setup()

qt_add_executable(colorpaletteserver
    apibehavior.h
    types.h
    utils.h
    main.cpp
)

qt_add_resources(colorpaletteserver "assets"
    PREFIX "/"
    FILES
    assets/colors.json
    assets/users.json
    assets/sessions.json
    assets/img/1-image.jpg
    assets/img/2-image.jpg
    assets/img/3-image.jpg
    assets/img/4-image.jpg
    assets/img/5-image.jpg
    assets/img/6-image.jpg
    assets/img/7-image.jpg
    assets/img/8-image.jpg
    assets/img/9-image.jpg
    assets/img/10-image.jpg
    assets/img/11-image.jpg
    assets/img/12-image.jpg
)

target_link_libraries(colorpaletteserver PRIVATE
    Qt::HttpServer
    Qt::Gui
    Qt::Concurrent
)

install(TARGETS colorpaletteserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
