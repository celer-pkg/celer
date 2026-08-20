# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from simplescene.pro.

#####################################################################
## qquick3dsimplescene Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qquick3dsimplescene LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Collect test data
file(GLOB_RECURSE test_data
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    data/*
)

qt_internal_add_test(tst_qquick3dsimplescene
    SOURCES
        ../shared/util.cpp ../shared/util.h
        tst_simplescene.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Gui
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRenderPrivate
   TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:simplescene.pro:<TRUE>:
# OTHER_FILES = "data/cube.qml"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qquick3dsimplescene CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=":/data"
)

qt_internal_extend_target(tst_qquick3dsimplescene CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(tst_qquick3dsimplescene)
endif()
