# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(magic8ball_server LANGUAGES CXX)

# Find OpenSSL first since it's a transitive dependency that may not be available in CONFIG mode
find_package(OpenSSL)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

find_package(Protobuf)
find_package(gRPC)

if(CMAKE_CROSSCOMPILING)
    find_program(grpc_cpp_plugin grpc_cpp_plugin NO_CACHE)
elseif(TARGET gRPC::grpc_cpp_plugin)
    set(grpc_cpp_plugin $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else()
    set(grpc_cpp_plugin "")
endif()

if(NOT grpc_cpp_plugin OR NOT TARGET gRPC::grpc++)
    return()
endif()

if(MINGW)
    message(WARNING "${PROJECT_NAME} uses reference grpc++ library that doesn't officially support"
        " MinGW. Please use the MSVC compiler to build this example. The correct work is not"
        " guaranteed otherwise.")
endif()

# Avoid "Protobuf requires at least C++11." errors
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(proto_files "${CMAKE_CURRENT_LIST_DIR}/../proto/exampleservice.proto")
set(out_dir ${CMAKE_CURRENT_BINARY_DIR})

set(generated_files
    "${out_dir}/exampleservice.pb.h" "${out_dir}/exampleservice.pb.cc"
    "${out_dir}/exampleservice.grpc.pb.h" "${out_dir}/exampleservice.grpc.pb.cc")

add_custom_command(
    OUTPUT ${generated_files}
    COMMAND
    $<TARGET_FILE:WrapProtoc::WrapProtoc>
    ARGS
    --grpc_out "${out_dir}"
    --cpp_out "${out_dir}"
    -I "${CMAKE_CURRENT_LIST_DIR}/../proto/"
    --plugin=protoc-gen-grpc=${grpc_cpp_plugin}
    "${proto_files}"
    WORKING_DIRECTORY ${out_dir}
    DEPENDS "${proto_files}"
    COMMENT "Generating gRPC ${target} sources..."
    COMMAND_EXPAND_LISTS
    VERBATIM
)

set_source_files_properties(${generated_files} PROPERTIES GENERATED TRUE)

qt_add_executable(magic8ball_server
    ${generated_files}
    main.cpp
)

target_include_directories(magic8ball_server
    PRIVATE
    ${out_dir}
)

target_link_libraries(magic8ball_server
    PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++
)

install(TARGETS magic8ball_server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
