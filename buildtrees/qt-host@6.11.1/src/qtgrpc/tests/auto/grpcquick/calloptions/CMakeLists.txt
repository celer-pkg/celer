# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_calloptions_qml LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_calloptions_qml
    QMLTEST
    SOURCES
        tst_calloptions_qml.cpp
    LIBRARIES
        Qt::Qml
        Qt::GrpcQuick
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_qml_module(tst_calloptions_qml
    URI QmlTestUri
    VERSION 1.0
    QML_FILES
        qml/tst_calloptions.qml
)

qt_autogen_tools_initial_setup(tst_calloptions_qml)
