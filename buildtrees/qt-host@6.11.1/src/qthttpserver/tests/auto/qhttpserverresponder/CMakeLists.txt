# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qhttpserverresponder.pro.

#####################################################################
## tst_qhttpserverresponder Test:
#####################################################################

# Collect test data
list(APPEND test_data "data/")

qt_internal_add_test(tst_qhttpserverresponder
    SOURCES
        tst_qhttpserverresponder.cpp
    LIBRARIES
        Qt::HttpServer
    TESTDATA ${test_data}
)
