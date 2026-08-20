# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qgqml2dtest
    QMLTEST
    SOURCES
        tst_qml2dtest.cpp
    LIBRARIES
        Qt::Gui
    TESTDATA ${test_data}
)
