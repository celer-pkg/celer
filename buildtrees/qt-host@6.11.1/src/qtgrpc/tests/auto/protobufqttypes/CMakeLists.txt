# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_protobufqttypes LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_protobuf_qtcoretypes
    SOURCES
        qtprotobufqttypescoretest.cpp
    INCLUDE_DIRECTORIES
        ../protobuf/shared
    LIBRARIES
        Qt::Test
        Qt::Protobuf
        Qt::ProtobufQtCoreTypes
        Qt::Core
        Qt::CorePrivate
    DEFINES
        PROTOC_EXECUTABLE="$<TARGET_FILE:WrapProtoc::WrapProtoc>"
)

qt_add_protobuf(tst_protobuf_qtcoretypes
    PROTO_FILES
        ../shared/data/proto/qtcoretypes.proto
        qtcoretypesnested.proto
    PROTO_INCLUDES
        $<TARGET_PROPERTY:Qt6::ProtobufQtCoreTypes,QT_PROTO_INCLUDES>
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

qt_autogen_tools_initial_setup(tst_protobuf_qtcoretypes)

if(QT_FEATURE_gui)
    qt_internal_add_test(tst_protobuf_qtguitypes
        SOURCES
            qtprotobufqttypesguitest.cpp
        INCLUDE_DIRECTORIES
            ../protobuf/shared
        LIBRARIES
            Qt::Test
            Qt::Protobuf
            Qt::ProtobufQtGuiTypes
            Qt::Core
            Qt::Gui
        DEFINES
            PROTOC_EXECUTABLE="$<TARGET_FILE:WrapProtoc::WrapProtoc>"
    )

    qt_add_protobuf(tst_protobuf_qtguitypes
        PROTO_FILES
            ../shared/data/proto/qtguitypes.proto
            qtguitypesnested.proto
        PROTO_INCLUDES
            $<TARGET_PROPERTY:Qt6::ProtobufQtGuiTypes,QT_PROTO_INCLUDES>
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated_gui"
    )

    qt_autogen_tools_initial_setup(tst_protobuf_qtguitypes)
endif()
