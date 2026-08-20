# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_baseline_qtlottie Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_baseline_qtlottie LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

# Collect test data
list(APPEND test_data "../data")

set(BASELINE_DIR "../../../../qtbase/tests/baseline/shared")
qt_internal_add_test(tst_baseline_qtlottie
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.."
    SOURCES
        ${BASELINE_DIR}/baselineprotocol.cpp
        ${BASELINE_DIR}/baselineprotocol.h
        ${BASELINE_DIR}/lookup3.cpp
        ${BASELINE_DIR}/qbaselinetest.cpp
        ${BASELINE_DIR}/qbaselinetest.h
        tst_baseline_qtlottie.cpp
    INCLUDE_DIRECTORIES
        ${BASELINE_DIR}
    LIBRARIES
        Qt::Gui
        Qt::Network
    TESTDATA ${test_data}
)
