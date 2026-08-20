# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Find OpenSSL first since it's a transitive dependency that may not be available in CONFIG mode
find_package(OpenSSL)

set(_qt_grpc_backup_prefer_config ${CMAKE_FIND_PACKAGE_PREFER_CONFIG})
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

find_package(Protobuf)
find_package(gRPC)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ${_qt_grpc_backup_prefer_config})
unset(_qt_grpc_backup_prefer_config)

if(CMAKE_CROSSCOMPILING)
    find_program(grpc_cpp_plugin grpc_cpp_plugin NO_CACHE)
elseif(TARGET gRPC::grpc_cpp_plugin)
    set(grpc_cpp_plugin $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else()
    set(grpc_cpp_plugin "")
endif()

if(NOT grpc_cpp_plugin OR NOT TARGET WrapProtoc::WrapProtoc
    OR NOT TARGET gRPC::grpc++)
    message(WARNING "Dependencies of 'grpc_mock_server' not found. Skipping.")
    return()
endif()

add_library(grpc_mock_server STATIC mockserver.h mockserver.cpp)
target_include_directories(grpc_mock_server
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}"
)

target_link_libraries(grpc_mock_server
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc++
        Qt::Test
)

