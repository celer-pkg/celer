# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_optional LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt6_add_protobuf(tst_protobuf_optional_gen
    PROTO_FILES
        ../../shared/data/proto/optional.proto
)
qt_autogen_tools_initial_setup(tst_protobuf_optional_gen)

qt_internal_add_test(tst_protobuf_optional
    SOURCES
        tst_protobuf_optional.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_optional_gen
)
qt_autogen_tools_initial_setup(tst_protobuf_optional)

qt_internal_add_test(tst_protobuf_serialization_optional
    SOURCES
        tst_protobuf_serialization_optional.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_optional_gen
)
qt_autogen_tools_initial_setup(tst_protobuf_serialization_optional)

qt_internal_add_test(tst_protobuf_deserialization_optional
    SOURCES
        tst_protobuf_deserialization_optional.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_optional_gen
)
qt_autogen_tools_initial_setup(tst_protobuf_deserialization_optional)
