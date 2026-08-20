# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_grpc_client_deadline LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)

    include("../shared/test_server/CMakeLists.txt")
    include("../shared/client_service/CMakeLists.txt")
    include("../shared/client_test_common/CMakeLists.txt")
endif()

if(NOT TARGET grpc_testserver)
    return()
endif()

qt_internal_add_test(tst_grpc_client_deadline
    SOURCES
        tst_grpc_client_deadline.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../shared
    DEFINES
        TEST_GRPC_SERVER_PATH="$<TARGET_FILE:grpc_testserver>"
    LIBRARIES
        Qt::Test
        Qt::Core
        Qt::Grpc
        tst_grpc_client_qtgrpc_gen
        tst_grpc_client_test_common
)

add_dependencies(tst_grpc_client_deadline grpc_testserver)
