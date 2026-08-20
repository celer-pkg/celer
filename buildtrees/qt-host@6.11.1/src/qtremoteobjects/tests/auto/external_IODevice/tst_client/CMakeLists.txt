# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## tst_external_IODevice Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_external_IODevice LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_external_IODevice
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    SOURCES
        tst_client.cpp
    LIBRARIES
        Qt::RemoteObjects
)

# Resources:
set(cert_resource_files
    "../cert/client.crt"
    "../cert/client.key"
    "../cert/rootCA.key"
    "../cert/rootCA.pem"
    "../cert/rootCA.srl"
    "../cert/server.crt"
    "../cert/server.key"
)

qt_internal_add_resource(tst_external_IODevice "cert"
    PREFIX
        "/sslcert"
    BASE
        "../cert"
    FILES
        ${cert_resource_files}
)

qt6_add_repc_replicas(tst_external_IODevice
    ../pingpong.rep
)
