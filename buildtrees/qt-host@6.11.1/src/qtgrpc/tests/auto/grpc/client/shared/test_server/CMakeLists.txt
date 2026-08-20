# Copyright (C) 2022 The Qt Company Ltd.
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
    message(WARNING "Dependencies of Qt GRPC test server not found. Skipping.")
    return()
endif()

set(proto_files "${CMAKE_CURRENT_LIST_DIR}/../../../shared/proto/testservice.proto")
set(out_dir ${CMAKE_CURRENT_BINARY_DIR})

set(generated_files
    "${out_dir}/testservice.pb.h" "${out_dir}/testservice.pb.cc"
    "${out_dir}/testservice.grpc.pb.h" "${out_dir}/testservice.grpc.pb.cc")

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
        ${extra_protoc_args}
        --grpc_out "${out_dir}"
        --cpp_out "${out_dir}"
        -I "${CMAKE_CURRENT_LIST_DIR}/../../../shared/proto/"
        --plugin=protoc-gen-grpc=${grpc_cpp_plugin}
        "${proto_files}"
    WORKING_DIRECTORY ${out_dir}
    DEPENDS "${proto_files}" $<TARGET_FILE:WrapProtoc::WrapProtoc>
    COMMENT "Generating gRPC ${target} sources..."
    COMMAND_EXPAND_LISTS
    VERBATIM
)

set_source_files_properties(${generated_files} PROPERTIES GENERATED TRUE)
add_library(grpc_testserverrunner_gen STATIC ${generated_files})
target_include_directories(grpc_testserverrunner_gen
    PRIVATE
        ${out_dir}
        WrapgRPC_INCLUDE_PATH
)

target_link_libraries(grpc_testserverrunner_gen
    PRIVATE
        protobuf::libprotobuf
        gRPC::grpc++
)

add_library(grpc_testserverrunner
    STATIC
        ${CMAKE_CURRENT_LIST_DIR}/testserverrunner.cpp
        ${CMAKE_CURRENT_LIST_DIR}/testserverrunner.h
)

target_include_directories(grpc_testserverrunner
    PRIVATE
        ${out_dir}
        ${CMAKE_CURRENT_LIST_DIR}/../../../shared
)

target_link_libraries(grpc_testserverrunner
    PRIVATE
        grpc_testserverrunner_gen
        gRPC::grpc++
        Qt6::Core
)

set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/assets/cert.pem"
    PROPERTIES QT_RESOURCE_ALIAS assets/cert.pem)
set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/assets/key.pem"
    PROPERTIES QT_RESOURCE_ALIAS assets/key.pem)

qt_add_resources(grpc_testserverrunner
    "keys"
    PREFIX
        "/"
    FILES
        "${CMAKE_CURRENT_LIST_DIR}/assets/cert.pem"
        "${CMAKE_CURRENT_LIST_DIR}/assets/key.pem"
)

qt_internal_add_executable(grpc_testserver
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/testserver.cpp
    LIBRARIES
        grpc_testserverrunner
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    INSTALL_DIRECTORY "${INSTALL_TESTSDIR}/shared"
)
