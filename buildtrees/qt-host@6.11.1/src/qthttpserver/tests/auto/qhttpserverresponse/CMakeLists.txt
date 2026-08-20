# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qhttpserverresponse.pro.

#####################################################################
## tst_qhttpserverresponse Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserverresponse
    SOURCES
        tst_qhttpserverresponse.cpp
    LIBRARIES
        Qt::HttpServer
    TESTDATA ${test_data}
)
