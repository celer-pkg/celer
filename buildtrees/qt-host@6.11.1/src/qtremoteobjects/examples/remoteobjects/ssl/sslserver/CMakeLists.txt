# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(sslserver LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)

qt_standard_project_setup()

qt_add_executable(sslserver
    main.cpp
    timemodel.cpp timemodel.h
)

set_target_properties(sslserver PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

target_link_libraries(sslserver PRIVATE
    Qt::Core
    Qt::RemoteObjects
)

# Resources:
set(cert_resource_files
    "cert/rootCA.key"
    "cert/rootCA.pem"
    "cert/rootCA.srl"
    "cert/server.crt"
    "cert/server.key"
)

qt6_add_resources(sslserver "cert"
    PREFIX
        "/sslcert"
    BASE
        "cert"
    FILES
        ${cert_resource_files}
)

qt6_add_repc_sources(sslserver
    ../../timemodel.rep
)
