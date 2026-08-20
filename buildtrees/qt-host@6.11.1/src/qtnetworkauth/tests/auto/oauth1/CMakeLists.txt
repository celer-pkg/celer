# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from oauth1.pro.

#####################################################################
## tst_oauth1 Test:
#####################################################################

qt_internal_add_test(tst_oauth1
    SOURCES
        ../shared/webserver.h ../shared/webserver.cpp
        tst_oauth1.cpp
    INCLUDE_DIRECTORIES
        ../shared
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
        Qt::NetworkAuth
        Qt::NetworkAuthPrivate
)

#### Keys ignored in scope 1:.:.:oauth1.pro:<TRUE>:
# TEMPLATE = "app"
