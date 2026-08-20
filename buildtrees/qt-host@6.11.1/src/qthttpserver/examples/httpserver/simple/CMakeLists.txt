# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(simple LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/httpserver/${PROJECT_NAME}")

find_package(Qt6 REQUIRED COMPONENTS HttpServer)

if(ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS Gui)
endif()

qt_standard_project_setup()

qt_add_executable(simple
    main.cpp
)

target_link_libraries(simple PRIVATE
    Qt6::HttpServer
)

if(ANDROID)
    target_link_libraries(simple PRIVATE
        Qt::Gui
    )
endif()

qt_add_resources(simple "assets"
    PREFIX
        "/"
    FILES
        assets/certificate.crt
        assets/private.key
        assets/qt-logo.png
)

install(TARGETS simple
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
