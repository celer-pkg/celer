# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_grpc_client_bidistream_qml LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
    include("../../../grpc/client/shared/test_server/CMakeLists.txt")
endif()

if(NOT TARGET grpc_testserver)
    return()
endif()
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_internal_add_test(tst_grpc_client_bidistream_qml
    QMLTEST
    SOURCES
        tst_grpc_client_bidistream_qml.cpp
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../grpc/shared
    DEFINES
        TEST_GRPC_SERVER_PATH="$<TARGET_FILE:grpc_testserver>"
    LIBRARIES
        Qt::Qml
        Qt::Grpc
)

qt_add_grpc(tst_grpc_client_bidistream_qml_gen CLIENT
    PROTO_FILES
        ../../../grpc/shared/proto/testservice.proto
    QML
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/qt_grpc_generated_qml"
)

qt_add_protobuf(tst_grpc_client_bidistream_qml_gen
    PROTO_FILES
        ../../../grpc/shared/proto/testservice.proto
    QML
    QML_URI qtgrpc.tests
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}/qt_grpc_generated_qml"
)

qt_add_qml_module(tst_grpc_client_bidistream_qml
    URI QmlTestUri
    VERSION 1.0
    QML_FILES
        qml/tst_grpc_client_bidistream.qml
)

target_link_libraries(tst_grpc_client_bidistream_qml
    PRIVATE tst_grpc_client_bidistream_qml_gen)

add_dependencies(tst_grpc_client_bidistream_qml grpc_testserver)

qt_autogen_tools_initial_setup(tst_grpc_client_bidistream_qml)
qt_autogen_tools_initial_setup(tst_grpc_client_bidistream_qml_gen)
qt_autogen_tools_initial_setup(tst_grpc_client_bidistream_qml_genplugin)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_grpc_client_bidistream_qml)
    qt_import_qml_plugins(tst_grpc_client_bidistream_qml_gen)
endif()

