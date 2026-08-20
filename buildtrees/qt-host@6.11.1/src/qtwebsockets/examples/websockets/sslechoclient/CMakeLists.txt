# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(sslechoclient LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/websockets/sslechoclient")

find_package(Qt6 REQUIRED COMPONENTS WebSockets)

qt_add_executable(sslechoclient
    main.cpp
    sslechoclient.cpp sslechoclient.h
)

qt_add_resources(sslechoclient "cert"
    BASE ../sslechoserver
    FILES ../sslechoserver/localhost.cert
)

set_target_properties(sslechoclient PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

target_link_libraries(sslechoclient PUBLIC
    Qt::WebSockets
)

install(TARGETS sslechoclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
