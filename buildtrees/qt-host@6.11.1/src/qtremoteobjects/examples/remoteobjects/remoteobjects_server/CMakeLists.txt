# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(remoteobjects_server LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/remoteobjects/remoteobjects_server")

find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)

qt_add_executable(remoteobjects_server
    main.cpp
    timemodel.cpp timemodel.h
)

set_target_properties(remoteobjects_server PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(remoteobjects_server PUBLIC
    Qt::Core
    Qt::RemoteObjects
)

qt6_add_repc_sources(remoteobjects_server
    ../timemodel.rep
)

install(TARGETS remoteobjects_server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
