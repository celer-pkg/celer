# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(sslcppclient LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)

qt_standard_project_setup()

qt_add_executable(sslcppclient
    main.cpp
)

set_target_properties(sslcppclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE FALSE
)

target_link_libraries(sslcppclient PRIVATE
    Qt::Core
    Qt::RemoteObjects
)

# Resources:
set(cert_resource_files
    "../sslserver/cert/client.crt"
    "../sslserver/cert/client.key"
    "../sslserver/cert/rootCA.key"
    "../sslserver/cert/rootCA.pem"
    "../sslserver/cert/rootCA.srl"
)

qt6_add_resources(sslcppclient "cert"
    PREFIX
        "/sslcert"
    BASE
        "../sslserver/cert"
    FILES
        ${cert_resource_files}
)

qt6_add_repc_replicas(sslcppclient
    ../../timemodel.rep
)
