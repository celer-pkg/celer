# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_syntax_qml LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_internal_add_test(tst_protobuf_syntax_qml
    QMLTEST
    SOURCES
        tst_protobuf_syntax_qml.cpp
    LIBRARIES
        Qt::Qml
)

qt_add_protobuf(tst_protobuf_syntax_qml_gen
    PROTO_FILES
        ../../shared/data/proto/syntax.proto
    QML
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
    ALLOW_MUTABLE_GETTER_CONFLICTS
)

qt_add_qml_module(tst_protobuf_syntax_qml
    URI QmlTestUri
    VERSION 1.0
    QML_FILES
        qml/tst_syntax.qml
)

set_target_properties(tst_protobuf_syntax_qml_gen PROPERTIES
    QT_USE_PROTOBUF_LIST_ALIASES FALSE
)

qt_autogen_tools_initial_setup(tst_protobuf_syntax_qml)
qt_autogen_tools_initial_setup(tst_protobuf_syntax_qml_gen)
qt_autogen_tools_initial_setup(tst_protobuf_syntax_qml_genplugin)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_protobuf_syntax_qml)
endif()
