# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

find_package(Protobuf)

if(NOT Protobuf_FOUND)
    message(WARNING "Dependencies of Qt GRPC test server not found. Skipping.")
    return()
endif()


set(go_packages
    "google.golang.org/protobuf/cmd/protoc-gen-go@latest"
    "google.golang.org/grpc/cmd/protoc-gen-go-grpc@latest"
)

set(go_path ${CMAKE_CURRENT_BINARY_DIR}/go)
foreach(package ${go_packages})
    execute_process(COMMAND ${CMAKE_COMMAND} -E env GOPATH=${go_path} -- go install ${package})
endforeach()

set(server_sources main.go)
set(server_sources_copied ${server_sources})
list(TRANSFORM server_sources PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM server_sources_copied PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")

set(testserver_gen_output
    "${CMAKE_CURRENT_BINARY_DIR}/qtgrpc/tests/testservice_grpc.pb.go"
    "${CMAKE_CURRENT_BINARY_DIR}/qtgrpc/tests/testservice.pb.go"
)

set(extra_protoc_args "")
get_target_property(protoc_version WrapProtoc::WrapProtoc _qt_internal_protobuf_version)
if(protoc_version VERSION_GREATER_EQUAL "3.12" AND protoc_version VERSION_LESS "3.15")
    list(APPEND extra_protoc_args "--experimental_allow_proto3_optional")
endif()

add_custom_command(
    OUTPUT
        ${testserver_gen_output}
    COMMAND
        ${CMAKE_COMMAND} -E env "PATH=$ENV{PATH}:${go_path}/bin" "GOPATH=${go_path}" --
        $<TARGET_FILE:WrapProtoc::WrapProtoc>
        ${extra_protoc_args}
        --go_out=${CMAKE_CURRENT_BINARY_DIR}
        --go-grpc_out=${CMAKE_CURRENT_BINARY_DIR}
        -I${CMAKE_CURRENT_LIST_DIR}/../../../shared/proto
        ${CMAKE_CURRENT_LIST_DIR}/../../../shared/proto/testservice.proto
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/../../../shared/proto/testservice.proto
    VERBATIM
)

set(testserver_gen_module_output
    "${CMAKE_CURRENT_BINARY_DIR}/qtgrpc/tests/go.mod"
    "${CMAKE_CURRENT_BINARY_DIR}/qtgrpc/tests/go.sum"
)
add_custom_command(
    OUTPUT
        ${testserver_gen_module_output}
    COMMAND
        ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_BINARY_DIR}/qtgrpc/tests/go.mod"
    COMMAND
        ${CMAKE_COMMAND} -E env "GOPATH=${go_path}" -- ${go} mod init qtgrpc/tests
    COMMAND
        ${CMAKE_COMMAND} -E env "GOPATH=${go_path}" -- ${go} mod tidy
    DEPENDS
        ${testserver_gen_output}
    WORKING_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/qtgrpc/tests"
    VERBATIM
)

add_custom_command(
    OUTPUT
        ${server_sources_copied}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${server_sources} "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS
        ${server_sources}
    VERBATIM
)

set(testserver_module_init_output
    "${CMAKE_CURRENT_BINARY_DIR}/go.mod"
    "${CMAKE_CURRENT_BINARY_DIR}/go.sum"
)
add_custom_command(
    OUTPUT
        ${testserver_module_init_output}
    COMMAND
        ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_BINARY_DIR}/go.mod"
    COMMAND
        ${CMAKE_COMMAND} -E env "GOPATH=${go_path}" -- ${go} mod init grpc_testserver
    COMMAND
        ${CMAKE_COMMAND} -E env "GOPATH=${go_path}" -- ${go} mod edit -replace qtgrpc/tests=./qtgrpc/tests
    COMMAND
        ${CMAKE_COMMAND} -E env "GOPATH=${go_path}" -- ${go} mod tidy
    DEPENDS
        ${testserver_gen_module_output}
        ${server_sources_copied}
    VERBATIM
)


set(testserver_timestamp "${CMAKE_CURRENT_BINARY_DIR}/grpc_testserver_timestamp")
add_custom_command(
    OUTPUT "${testserver_timestamp}"
    COMMAND
        ${CMAKE_COMMAND} -E env "GOPATH=${go_path}" -- ${go} get
    COMMAND
        ${CMAKE_COMMAND} -E env "GOPATH=${go_path}" -- ${go} build
    COMMAND
        ${CMAKE_COMMAND} -E touch "${testserver_timestamp}"
    DEPENDS
        ${server_sources}
        ${testserver_module_init_output}
    VERBATIM
)

set(server_certs
    "${CMAKE_CURRENT_LIST_DIR}/../test_server/assets/cert.pem"
    "${CMAKE_CURRENT_LIST_DIR}/../test_server/assets/key.pem"
)

set(server_certs_output
    "${CMAKE_CURRENT_BINARY_DIR}/cert.pem"
    "${CMAKE_CURRENT_BINARY_DIR}/key.pem"
)

add_custom_command(
    OUTPUT ${server_certs_output}
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different ${server_certs} "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS
        ${server_certs}
    VERBATIM
)

add_custom_target(grpc_testserver_build ALL DEPENDS
    ${testserver_timestamp}
    ${server_certs_output}
)

add_executable(grpc_testserver IMPORTED)
add_dependencies(grpc_testserver grpc_testserver_build)
set_target_properties(grpc_testserver PROPERTIES
    IMPORTED_LOCATION
        "${CMAKE_CURRENT_BINARY_DIR}/grpc_testserver"
    IMPORTED_GLOBAL TRUE
)
