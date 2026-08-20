# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_json_basic LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt6_add_protobuf(tst_protobuf_json_basic_types_gen
    PROTO_FILES
        ../basic/proto/basicmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
qt_autogen_tools_initial_setup(tst_protobuf_json_basic_types_gen)

qt_internal_add_test(tst_protobuf_serialization_json_basic_types
    SOURCES
        tst_protobuf_serialization_json_basictypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_basic_types_gen
)

qt_internal_add_test(tst_protobuf_deserialization_json_basictypes
    SOURCES
        tst_protobuf_deserialization_json_basictypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_basic_types_gen
)

qt6_add_protobuf(tst_protobuf_json_repeatedtypes_gen
    PROTO_FILES
        ../basic/proto/repeatedmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
target_link_libraries(tst_protobuf_json_repeatedtypes_gen
    PUBLIC
        tst_protobuf_json_basic_types_gen
)
qt_autogen_tools_initial_setup(tst_protobuf_json_repeatedtypes_gen)

qt_internal_add_test(tst_protobuf_serialization_json_repeatedtypes
    SOURCES
        tst_protobuf_serialization_json_repeatedtypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_repeatedtypes_gen
)

qt_internal_add_test(tst_protobuf_deserialization_json_repeatedtypes
    SOURCES
        tst_protobuf_deserialization_json_repeatedtypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_repeatedtypes_gen
)

qt6_add_protobuf(tst_protobuf_json_maptypes_gen
    PROTO_FILES
        ../basic/proto/mapmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
qt_autogen_tools_initial_setup(tst_protobuf_json_maptypes_gen)
target_link_libraries(tst_protobuf_json_maptypes_gen
    PUBLIC
        tst_protobuf_json_basic_types_gen
)

qt_internal_add_test(tst_protobuf_serialization_json_maptypes
    SOURCES
        tst_protobuf_serialization_json_maptypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_maptypes_gen
)

qt_internal_add_test(tst_protobuf_deserialization_json_maptypes
    SOURCES
        tst_protobuf_deserialization_json_maptypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_maptypes_gen
)

qt6_add_protobuf(tst_protobuf_json_enumtypes_gen
    PROTO_FILES
        ../enums/enummessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)
qt_autogen_tools_initial_setup(tst_protobuf_json_enumtypes_gen)

qt_internal_add_test(tst_protobuf_serialization_json_enumtypes
    SOURCES
        tst_protobuf_serialization_json_enumtypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_enumtypes_gen
)

qt_internal_add_test(tst_protobuf_deserialization_json_enumtypes
    SOURCES
        tst_protobuf_deserialization_json_enumtypes.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        tst_protobuf_json_enumtypes_gen
)
