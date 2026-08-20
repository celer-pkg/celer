# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qhttpserverrequestfilter Test:
#####################################################################

qt_internal_add_test(tst_qhttpserverrequestfilter
    SOURCES
        tst_qhttpserverrequestfilter.cpp
    LIBRARIES
        Qt::HttpServerPrivate
)
