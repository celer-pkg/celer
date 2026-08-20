# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from websocketframe.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_websocketframe Test:
#####################################################################

qt_internal_add_test(tst_websocketframe
    SOURCES
        tst_websocketframe.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
)

#### Keys ignored in scope 1:.:.:websocketframe.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
