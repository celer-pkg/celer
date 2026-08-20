# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

list(APPEND test_data "../shared/certs")

qt_internal_add_test(tst_abstractoauth2
    SOURCES
        tst_abstractoauth2.cpp
        ../shared/oauthtestutils.h ../shared/oauthtestutils.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
        Qt::NetworkAuth
        Qt::NetworkAuthPrivate
    TESTDATA ${test_data}
)
