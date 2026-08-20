# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qquick3drendercontrol Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qquick3drendercontrol LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

file(GLOB_RECURSE test_data_glob
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        data/*)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qquick3drendercontrol
    SOURCES
        ../shared/util.cpp ../shared/util.h
        tst_rendercontrol.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::Gui
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRenderPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:rendercontrol.pro:<TRUE>:
# OTHER_FILES = "data/cube.qml"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qquick3drendercontrol CONDITION ANDROID OR IOS
    DEFINES
        QT_QMLTEST_DATADIR=":/data"
)

qt_internal_extend_target(tst_qquick3drendercontrol CONDITION NOT ANDROID AND NOT IOS
    DEFINES
        QT_QMLTEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
