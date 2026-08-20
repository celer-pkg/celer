# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qmlwebsockets.pro.

#####################################################################
## tst_qmlwebsockets Test:
#####################################################################
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.qml)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qmlwebsockets
    QMLTEST
    SOURCES
        tst_qmlwebsockets.cpp
    LIBRARIES
        Qt::Gui
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qmlwebsockets.pro:<TRUE>:
# DEPLOYMENT = "importFiles"
# TEMPLATE = "app"
# importFiles.path = "."
