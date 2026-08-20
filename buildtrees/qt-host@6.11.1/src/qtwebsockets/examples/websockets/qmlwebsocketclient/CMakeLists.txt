# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qmlwebsocketclient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/websockets/qmlwebsocketclient")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick WebSockets)

qt_add_executable(qmlwebsocketclient
    main.cpp
)

set_target_properties(qmlwebsocketclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE FALSE
)

target_link_libraries(qmlwebsocketclient PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::WebSockets
)

# Resources:
set(data_resource_files
    "qml/qmlwebsocketclient/main.qml"
)

qt6_add_resources(qmlwebsocketclient "data"
    PREFIX
        "/"
    FILES
        ${data_resource_files}
)

install(TARGETS qmlwebsocketclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
