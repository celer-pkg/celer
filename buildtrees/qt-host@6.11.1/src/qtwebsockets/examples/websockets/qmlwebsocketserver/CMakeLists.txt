# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qmlwebsocketserver LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/websockets/qmlwebsocketserver")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick WebSockets)

qt_add_executable(qmlwebsocketserver
    main.cpp
)

set_target_properties(qmlwebsocketserver PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE FALSE
)

target_link_libraries(qmlwebsocketserver PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::WebSockets
)

# Resources:
set(data_resource_files
    "qml/qmlwebsocketserver/main.qml"
)

qt6_add_resources(qmlwebsocketserver "data"
    PREFIX
        "/"
    FILES
        ${data_resource_files}
)

install(TARGETS qmlwebsocketserver
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
