# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qhttpserver.pro.

#####################################################################
## tst_qhttpserver Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserver
    SOURCES
        tst_qhttpserver.cpp
    LIBRARIES
        Qt::HttpServer
        Qt::TestPrivate
    TESTDATA ${test_data}
)

qt_internal_extend_target(tst_qhttpserver CONDITION QT_FEATURE_concurrent
    LIBRARIES
        Qt::Concurrent
)
