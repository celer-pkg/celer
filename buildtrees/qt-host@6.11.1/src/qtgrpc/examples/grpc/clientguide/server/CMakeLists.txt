# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL first since it's a transitive dependency that may not be available in CONFIG mode
find_package(OpenSSL)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

find_package(protobuf)
find_package(gRPC)
find_package(Qt6 COMPONENTS ProtobufTools)

if(CMAKE_CROSSCOMPILING)
    find_program(grpc_cpp_plugin grpc_cpp_plugin NO_CACHE)
elseif(TARGET gRPC::grpc_cpp_plugin)
    set(grpc_cpp_plugin $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else()
    set(grpc_cpp_plugin "")
endif()

if(NOT grpc_cpp_plugin OR NOT TARGET WrapProtoc::WrapProtoc
    OR NOT TARGET gRPC::grpc++)
    message(WARNING "Dependencies of ${PROJECT_NAME} not found. Skipping.")
    return()
endif()

if(MINGW)
    message(WARNING "${PROJECT_NAME} uses reference grpc++ library that doesn't officially support"
        " MinGW. Please use the MSVC compiler to build this example. Correct operation is not"
        " guaranteed otherwise.")
endif()

set(proto_files "${CMAKE_CURRENT_LIST_DIR}/../proto/clientguide.proto")
set(proto_out "${CMAKE_CURRENT_BINARY_DIR}")

set(generated_files
    "${proto_out}/clientguide.pb.h" "${proto_out}/clientguide.pb.cc"
    "${proto_out}/clientguide.grpc.pb.h" "${proto_out}/clientguide.grpc.pb.cc")

add_custom_command(
    OUTPUT ${generated_files}
    COMMAND $<TARGET_FILE:WrapProtoc::WrapProtoc>
    ARGS
        --grpc_out "${proto_out}"
        --cpp_out "${proto_out}"
        -I "${CMAKE_CURRENT_LIST_DIR}/../proto"
        --plugin=protoc-gen-grpc=${grpc_cpp_plugin}
        "${proto_files}"
    WORKING_DIRECTORY ${proto_out}
    DEPENDS "${proto_files}"
    COMMENT "Generating gRPC ${target} sources..."
    COMMAND_EXPAND_LISTS
    VERBATIM
)
set_source_files_properties(${generated_files} PROPERTIES GENERATED TRUE)

add_executable(clientguide_server
    main.cpp
    ${generated_files})

target_include_directories(clientguide_server PRIVATE ${proto_out})

target_link_libraries(clientguide_server
    PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++
)

install(TARGETS clientguide_server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
