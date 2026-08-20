# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(WIN32)
    add_library(tst_grpc_client_qtgrpc_gen STATIC)
endif()

qt6_add_protobuf(tst_grpc_client_qtgrpc_gen
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/qt_grpc_generated"
    PROTO_FILES
        ${CMAKE_CURRENT_LIST_DIR}/../../../shared/proto/testservice.proto
)

qt6_add_grpc(tst_grpc_client_qtgrpc_gen CLIENT
    PROTO_FILES
        ${CMAKE_CURRENT_LIST_DIR}/../../../shared/proto/testservice.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_grpc_generated"
)

qt_autogen_tools_initial_setup(tst_grpc_client_qtgrpc_gen)
