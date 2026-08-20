# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_library(tst_bench_deserialize_protobuf_base STATIC
    tst_bench_deserialize_protobuf_base.h
    tst_bench_deserialize_protobuf_base.cpp
)
qt_autogen_tools_initial_setup(tst_bench_deserialize_protobuf_base)
target_link_libraries(tst_bench_deserialize_protobuf_base PRIVATE Qt::Test)

set_target_properties(tst_bench_deserialize_protobuf_base PROPERTIES
    QT_USE_PROTOBUF_LIST_ALIASES FALSE)

qt6_add_protobuf(tst_bench_deserialize_protobuf_base
    PROTO_FILES
        data/bench.proto
)

qt_internal_add_benchmark(tst_bench_deserialize_protobuf
    SOURCES
        tst_bench_deserialize_protobuf.cpp
    INCLUDE_DIRECTORIES
        .
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_bench_deserialize_protobuf_base
)
set_target_properties(tst_bench_deserialize_protobuf PROPERTIES QT_USE_PROTOBUF_LIST_ALIASES FALSE)

qt_internal_add_benchmark(tst_bench_deserialize_protobuf_json
    SOURCES
        tst_bench_deserialize_protobuf_json.cpp
    INCLUDE_DIRECTORIES
        .
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_bench_deserialize_protobuf_base
)
set_target_properties(tst_bench_deserialize_protobuf_json
    PROPERTIES
        QT_USE_PROTOBUF_LIST_ALIASES FALSE
)
