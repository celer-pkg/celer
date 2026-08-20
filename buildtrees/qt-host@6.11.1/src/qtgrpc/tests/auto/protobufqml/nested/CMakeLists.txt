# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_protobuf_nested_qml LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_internal_add_test(tst_nestedtypes_qtprotobuf_qmltest
    QMLTEST
    SOURCES
        tst_protobuf_nested_qml.cpp
    LIBRARIES
        Qt::Qml
)

qt_add_protobuf(tst_nestedtypes_qtprotobuf_qml
    PROTO_FILES
        ../../protobuf/nested/proto/nestedmessages.proto
        ../../protobuf/nested/proto/externalpackage.proto
    QML
    QML_URI
        qtprotobufnamespace.tests.nested
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_generated"
)

qt_add_qml_module(tst_nestedtypes_qtprotobuf_qmltest
    URI QmlTestUri
    VERSION 1.0
    QML_FILES
        qml/tst_nested.qml
)

qt_autogen_tools_initial_setup(tst_nestedtypes_qtprotobuf_qmltest)
qt_autogen_tools_initial_setup(tst_nestedtypes_qtprotobuf_qml)
qt_autogen_tools_initial_setup(tst_nestedtypes_qtprotobuf_qmlplugin)

qt_add_protobuf(tst_externalpackage_qtprotobuf_qml
    PROTO_FILES
        ../../protobuf/nested/proto/externalpackage.proto
    QML
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qt_protobuf_externalpackage_generated"
)

qt_autogen_tools_initial_setup(tst_externalpackage_qtprotobuf_qml)
qt_autogen_tools_initial_setup(tst_externalpackage_qtprotobuf_qmlplugin)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_nestedtypes_qtprotobuf_qmltest)
    qt_import_qml_plugins(tst_nestedtypes_qtprotobuf_qml)
    qt_import_qml_plugins(tst_externalpackage_qtprotobuf_qml)
endif()

