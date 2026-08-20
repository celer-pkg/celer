# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_grpc_server LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt6_add_protobuf(tst_grpc_server_qtgrpc_gen
    PROTO_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto/testservice.proto
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/qt_grpc_generated"
)

qt6_add_grpc(tst_grpc_server_qtgrpc_gen CLIENT
    PROTO_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/proto/testservice.proto
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/qt_grpc_generated"
)

qt_autogen_tools_initial_setup(tst_grpc_server_qtgrpc_gen)

qt_internal_add_test(tst_grpc_server
    SOURCES
        tst_grpc_server.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared
    LIBRARIES
        Qt::Test
        Qt::Core
        Qt::Grpc
        tst_grpc_server_qtgrpc_gen
)

set(assets_resource_files "../shared/assets/testfile")
qt_add_resources(tst_grpc_server
    "assets"
    PREFIX
        "/"
    FILES
        ${assets_resource_files}
)
