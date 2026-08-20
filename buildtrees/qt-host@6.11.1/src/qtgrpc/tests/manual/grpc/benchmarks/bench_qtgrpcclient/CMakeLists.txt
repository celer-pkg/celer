# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(proto_out_dir "${CMAKE_CURRENT_BINARY_DIR}/include/proto/")
set(proto_out_include "${CMAKE_CURRENT_BINARY_DIR}/include/")

if (NOT TARGET Qt6::Grpc)
    message(WARNING "Dependencies of QtGrpc bench_qtgrpcclient not found. Skipping.")
    return()
endif()

qt_internal_add_executable(
    bench_qtgrpcclient
    SOURCES
        bench_qtgrpcclient.cpp
    LIBRARIES
        Qt::Grpc
        Qt::ProtobufWellKnownTypes
    INCLUDE_DIRECTORIES
        "${proto_out_include}"
        "${qrpcbench_common_include}"
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}"
)

qt_add_protobuf(bench_qtgrpcclient
    PROTO_FILES
        "${proto_path}"
    OUTPUT_DIRECTORY
        "${proto_out_dir}"
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufWellKnownTypes,QT_PROTO_INCLUDES>
)

qt_add_grpc(bench_qtgrpcclient CLIENT
    PROTO_FILES
        "${proto_path}"
    OUTPUT_DIRECTORY
        "${proto_out_dir}"
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufWellKnownTypes,QT_PROTO_INCLUDES>
)

target_compile_definitions(bench_qtgrpcclient PRIVATE QTGRPCCLIENT)

if(TARGET asyncbenchserver)
    add_dependencies(bench_qtgrpcclient asyncbenchserver)
endif()
