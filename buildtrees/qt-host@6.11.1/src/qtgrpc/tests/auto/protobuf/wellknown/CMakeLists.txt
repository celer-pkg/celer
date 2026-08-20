# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_wellknown LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_protobuf_any
    SOURCES
        tst_protobuf_any.cpp
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        Qt::ProtobufWellKnownTypes
)

qt6_add_protobuf(tst_protobuf_any
    PROTO_FILES
        ../../shared/data/proto/anymessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufWellKnownTypes,QT_PROTO_INCLUDES>
)
qt_autogen_tools_initial_setup(tst_protobuf_any)

qt_internal_add_test(tst_protobuf_timestamp
    SOURCES
        tst_protobuf_timestamp.cpp
    LIBRARIES
        Qt::Test
        Qt::ProtobufWellKnownTypes
)

qt6_add_protobuf(tst_protobuf_timestamp
    PROTO_FILES
        ../../shared/data/proto/timestampmessages.proto
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufWellKnownTypes,QT_PROTO_INCLUDES>
    EXTRA_NAMESPACE
        MyTopLevelNamespace
)
qt_autogen_tools_initial_setup(tst_protobuf_timestamp)

qt_internal_add_test(tst_protobuf_duration
    SOURCES
        tst_protobuf_duration.cpp
    LIBRARIES
        Qt::Test
        Qt::ProtobufWellKnownTypes
)
qt6_add_protobuf(tst_protobuf_duration
    PROTO_FILES
        proto/durationtest.proto
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufWellKnownTypes,QT_PROTO_INCLUDES>
)

qt_autogen_tools_initial_setup(tst_protobuf_duration)
