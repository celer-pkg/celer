# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_syntax LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt6_add_protobuf(tst_protobuf_syntax_qtprotobuf_gen
    PROTO_FILES
        ../../shared/data/proto/syntax.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
    ALLOW_MUTABLE_GETTER_CONFLICTS
)
qt_autogen_tools_initial_setup(tst_protobuf_syntax_qtprotobuf_gen)

qt_internal_add_test(tst_protobuf_syntax
    SOURCES
        tst_protobuf_syntax.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_syntax_qtprotobuf_gen
)

set_target_properties(tst_protobuf_syntax_qtprotobuf_gen PROPERTIES
    QT_USE_PROTOBUF_LIST_ALIASES FALSE
)
