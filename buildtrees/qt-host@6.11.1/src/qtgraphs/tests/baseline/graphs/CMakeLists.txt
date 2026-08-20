# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Collect test data
list(APPEND test_data "../data")
set(BASELINE_DIR "../../../../qtbase/tests/baseline/shared")

qt_internal_add_test(tst_baseline_graphs
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        ${BASELINE_DIR}/baselineprotocol.cpp
        ${BASELINE_DIR}/baselineprotocol.h
        ${BASELINE_DIR}/lookup3.cpp
        ${BASELINE_DIR}/qbaselinetest.cpp
        ${BASELINE_DIR}/qbaselinetest.h
        tst_baseline_graphs.cpp
    DEFINES
        QMAKESPEC=""
    INCLUDE_DIRECTORIES
        ${BASELINE_DIR}
    LIBRARIES
        Qt::Gui
        Qt::Network
    TESTDATA ${test_data}
)

add_dependencies(tst_baseline_graphs Qt6::Quick3D)
add_dependencies(tst_baseline_graphs qgraphs_qmlscenegrabber)
