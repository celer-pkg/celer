# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from quick3d.pro.

#####################################################################
## tst_qquick3d Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_baseline_qquick3d LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Collect test data
list(APPEND test_data "../data")
set(BASELINE_DIR "../../../../qtbase/tests/baseline/shared")

qt_internal_add_test(tst_baseline_qquick3d
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        ${BASELINE_DIR}/baselineprotocol.cpp
        ${BASELINE_DIR}/baselineprotocol.h
        ${BASELINE_DIR}/lookup3.cpp
        ${BASELINE_DIR}/qbaselinetest.cpp
        ${BASELINE_DIR}/qbaselinetest.h
        tst_baseline_quick3d.cpp
    DEFINES
        QMAKESPEC=""
    INCLUDE_DIRECTORIES
        ${BASELINE_DIR}
    LIBRARIES
        Qt::Gui
        Qt::Network
    TESTDATA ${test_data}
)

# special case begin
add_dependencies(tst_baseline_qquick3d Qt6::Quick3D)
add_dependencies(tst_baseline_qquick3d qquick3d_qmlscenegrabber)
# special case end

## Scopes:
#####################################################################

#### Keys ignored in scope 4:.:../shared:../shared/qbaselinetest.pri:WIN32:
# MKSPEC = "$$replace(QMAKESPEC, \\\\, /)"

#### Keys ignored in scope 5:.:../shared:../shared/qbaselinetest.pri:else:
# MKSPEC = "$$QMAKESPEC"
