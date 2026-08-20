# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from handshakeresponse.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_handshakeresponse Test:
#####################################################################

qt_internal_add_test(tst_handshakeresponse
    SOURCES
        tst_handshakeresponse.cpp
    LIBRARIES
        Qt::NetworkPrivate
        Qt::WebSocketsPrivate
)

#### Keys ignored in scope 1:.:.:handshakeresponse.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
