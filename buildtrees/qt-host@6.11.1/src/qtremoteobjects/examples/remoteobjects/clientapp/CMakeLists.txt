# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(clientapp LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/remoteobjects/clientapp")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick RemoteObjects)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(clientapp
    WIN32
    MACOSX_BUNDLE
    main.cpp
)

qt_add_qml_module(clientapp
    VERSION 1.0
    URI "TimeExample"
    SOURCES
        plugin.h
    QML_FILES
        Clock.qml
        Plugins.qml
        plugins0.qml
        plugins1.qml
        plugins2.qml
    RESOURCES
        center.png
        clock.png
        hour.png
        minute.png
)

target_link_libraries(clientapp PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::RemoteObjects
)

qt_add_repc_replicas(clientapp
    ../timemodel.rep
)

install(TARGETS clientapp
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
