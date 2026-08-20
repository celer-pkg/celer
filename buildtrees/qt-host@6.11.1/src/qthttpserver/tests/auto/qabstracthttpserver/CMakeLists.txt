# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qabstracthttpserver.pro.

#####################################################################
## tst_qabstracthttpserver Test:
#####################################################################

qt_internal_add_test(tst_qabstracthttpserver
    SOURCES
        tst_qabstracthttpserver.cpp
    LIBRARIES
        Qt::HttpServer
        Qt::NetworkPrivate
        Qt::TestPrivate
)
