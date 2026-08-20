# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

list(APPEND test_data "../shared/certs")

qt_internal_add_test(tst_oauth2codeflow
    SOURCES
        ../shared/webserver.h ../shared/webserver.cpp
        ../shared/tlswebserver.h ../shared/tlswebserver.cpp
        ../shared/oauthtestutils.h ../shared/oauthtestutils.cpp
        tst_oauth2codeflow.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
        Qt::NetworkAuth
        Qt::NetworkAuthPrivate
    TESTDATA ${test_data}
)
