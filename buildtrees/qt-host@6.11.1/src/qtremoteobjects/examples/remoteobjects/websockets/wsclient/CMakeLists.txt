# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(wsclient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/remoteobjects/websockets/wsclient")

find_package(Qt6 REQUIRED COMPONENTS Core Gui RemoteObjects WebSockets Widgets)

qt_add_executable(wsclient
    ../common/websocketiodevice.cpp ../common/websocketiodevice.h
    main.cpp
)

set_target_properties(wsclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE FALSE
)

target_include_directories(wsclient PUBLIC
    ../common
)

target_link_libraries(wsclient PUBLIC
    Qt::Core
    Qt::Gui
    Qt::RemoteObjects
    Qt::WebSockets
    Qt::Widgets
)

# Resources:
set(cert_resource_files
    "../common/cert/client.crt"
    "../common/cert/client.key"
    "../common/cert/rootCA.key"
    "../common/cert/rootCA.pem"
    "../common/cert/rootCA.srl"
    "../common/cert/server.crt"
    "../common/cert/server.key"
)

qt6_add_resources(wsclient "cert"
    PREFIX
        "/sslcert"
    BASE
        "../common/cert"
    FILES
        ${cert_resource_files}
)

install(TARGETS wsclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
