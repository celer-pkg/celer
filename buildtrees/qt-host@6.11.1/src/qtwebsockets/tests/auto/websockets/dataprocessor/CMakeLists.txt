# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from dataprocessor.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_dataprocessor Test:
#####################################################################

qt_internal_add_test(tst_dataprocessor
    SOURCES
        tst_dataprocessor.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
)

#### Keys ignored in scope 1:.:.:dataprocessor.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
