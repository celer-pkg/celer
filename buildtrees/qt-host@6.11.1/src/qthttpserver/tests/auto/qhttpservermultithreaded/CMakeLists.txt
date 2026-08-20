# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qhttpservermultithreaded Test:
#####################################################################

qt_internal_add_test(tst_qhttpservermultithreaded
    DEFINES
        QTEST_THROW_ON_FAIL
        QTEST_THROW_ON_SKIP
    SOURCES
        tst_qhttpservermultithreaded.cpp
    LIBRARIES
        Qt::HttpServer Qt::Concurrent Qt::TestPrivate
)
