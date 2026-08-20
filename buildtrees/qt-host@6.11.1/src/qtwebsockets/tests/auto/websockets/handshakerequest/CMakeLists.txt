# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from handshakerequest.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_handshakerequest Test:
#####################################################################

qt_internal_add_test(tst_handshakerequest
    SOURCES
        tst_handshakerequest.cpp
    LIBRARIES
        Qt::NetworkPrivate
        Qt::WebSocketsPrivate
)

#### Keys ignored in scope 1:.:.:handshakerequest.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
