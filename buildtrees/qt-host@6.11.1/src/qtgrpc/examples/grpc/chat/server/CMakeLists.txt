# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(qtgrpc_chat_server LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()
set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/grpc/chat")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenSSL first since it's a transitive dependency that may not be available in CONFIG mode
find_package(OpenSSL)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

find_package(Qt6 COMPONENTS ProtobufTools ProtobufQtCoreTypes)
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
        OR NOT TARGET gRPC::grpc++ OR NOT TARGET Qt6::ProtobufQtCoreTypes
        OR NOT TARGET gRPC::grpc++_reflection)
    message(WARNING "Dependencies of ${PROJECT_NAME} not found. Skipping.")
    return()
endif()

if(MINGW)
    message(WARNING "${PROJECT_NAME} uses reference grpc++ library that doesn't officially support"
        " MinGW. Please use the MSVC compiler to build this example. Correct operation is not"
        " guaranteed otherwise.")
endif()

set(proto_files
    "${CMAKE_CURRENT_LIST_DIR}/../proto/chatmessages.proto"
    "${CMAKE_CURRENT_LIST_DIR}/../proto/qtgrpcchat.proto"
    "$<TARGET_PROPERTY:Qt6::ProtobufQtCoreTypes,QT_PROTO_INCLUDES>/QtCore/QtCore.proto"
)
set(proto_out "${CMAKE_CURRENT_BINARY_DIR}")

set(generated_files
    "${proto_out}/chatmessages.pb.h" "${proto_out}/chatmessages.pb.cc"
    "${proto_out}/qtgrpcchat.pb.h" "${proto_out}/qtgrpcchat.pb.cc"
    "${proto_out}/QtCore/QtCore.pb.h" "${proto_out}/QtCore/QtCore.pb.cc"
    "${proto_out}/qtgrpcchat.grpc.pb.h" "${proto_out}/qtgrpcchat.grpc.pb.cc"
)

set(proto_includes
    "-I${CMAKE_CURRENT_LIST_DIR}/../proto/"
    "-I$<TARGET_PROPERTY:Qt6::ProtobufQtCoreTypes,QT_PROTO_INCLUDES>"
)

add_custom_command(
    OUTPUT ${generated_files}
    COMMAND
    $<TARGET_FILE:WrapProtoc::WrapProtoc>
    ARGS
    --grpc_out "${proto_out}"
    --cpp_out "${proto_out}"
    --plugin=protoc-gen-grpc=${grpc_cpp_plugin}
    ${proto_includes}
    ${proto_files}
    WORKING_DIRECTORY ${proto_out}
    DEPENDS "${proto_files}"
    COMMENT "Generating gRPC ${target} sources..."
    COMMAND_EXPAND_LISTS
    VERBATIM
)

set_source_files_properties(${generated_files} PROPERTIES GENERATED TRUE)

add_executable(qtgrpc_chat_server
    main.cpp
    ${generated_files}
)

target_include_directories(qtgrpc_chat_server PRIVATE ${proto_out})
target_link_libraries(qtgrpc_chat_server PRIVATE gRPC::grpc++ gRPC::grpc++_reflection)

install(TARGETS qtgrpc_chat_server
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
