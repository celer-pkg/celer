# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_grpc_http2channel_qml LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_grpc_http2channel_qml
    QMLTEST
    SOURCES
        tst_grpchttp2channel_qml.cpp
    LIBRARIES
        Qt::Qml
        Qt::GrpcQuick
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_qml_module(tst_grpc_http2channel_qml
    URI QmlTestUri
    VERSION 1.0
    QML_FILES
        qml/tst_grpchttp2channel.qml
)

qt_autogen_tools_initial_setup(tst_grpc_http2channel_qml)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_grpc_http2channel_qml)
endif()

