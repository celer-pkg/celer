# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(WrapProtoc)
find_package(Protobuf)
find_package(gRPC)

if(CMAKE_CROSSCOMPILING)
    find_program(grpc_cpp_plugin grpc_cpp_plugin NO_CACHE)
elseif(TARGET gRPC::grpc_cpp_plugin)
    set(grpc_cpp_plugin $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
else()
    set(grpc_cpp_plugin "")
endif()

if(NOT grpc_cpp_plugin OR NOT TARGET WrapProtoc::WrapProtoc
    OR NOT TARGET gRPC::grpc++)
    message(WARNING "Dependencies of QtGrpc benchmark-server not found. Skipping.")
    return()
endif()

set(proto_out_dir "${CMAKE_CURRENT_BINARY_DIR}/include/proto/")
set(proto_out_include "${CMAKE_CURRENT_BINARY_DIR}/include/")

set(generated_files
    "${proto_out_dir}/bench.grpc.pb.h"
    "${proto_out_dir}/bench.grpc.pb.cc"
    "${proto_out_dir}/bench.pb.h"
    "${proto_out_dir}/bench.pb.cc"
)

set(extra_protoc_args "")
get_target_property(protoc_version WrapProtoc::WrapProtoc _qt_internal_protobuf_version)
if(protoc_version VERSION_GREATER_EQUAL "3.12" AND protoc_version VERSION_LESS "3.15")
    list(APPEND extra_protoc_args "--experimental_allow_proto3_optional")
endif()

add_custom_command(
    OUTPUT ${generated_files}
    COMMAND
        $<TARGET_FILE:WrapProtoc::WrapProtoc>
    ARGS
        "${extra_protoc_args}"
        --grpc_out "${proto_out_dir}"
        --cpp_out "${proto_out_dir}"
        -I "${proto_include}"
        --plugin=protoc-gen-grpc=${grpc_cpp_plugin}
        "${proto_path}"
    WORKING_DIRECTORY "${proto_out_dir}"
    DEPENDS
        "${proto_path}"
        $<TARGET_FILE:WrapProtoc::WrapProtoc>
        ${grpc_cpp_plugin}
    COMMENT "Generating gRPC ${target} sources..."
    COMMAND_EXPAND_LISTS
    VERBATIM
)

set_source_files_properties(${generated_files} PROPERTIES GENERATED TRUE)

qt_internal_add_executable(
    asyncbenchserver
    SOURCES
        asyncbenchserver.cpp
        "${generated_files}"
    LIBRARIES
        protobuf::libprotobuf
        gRPC::grpc++
        Qt::Core
    INCLUDE_DIRECTORIES
        "${proto_out_include}"
        "${qrpcbench_common_include}"
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}"
)
