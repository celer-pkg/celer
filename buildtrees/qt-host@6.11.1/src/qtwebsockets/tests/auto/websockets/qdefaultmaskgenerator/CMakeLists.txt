# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qdefaultmaskgenerator.pro.

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_defaultmaskgenerator Test:
#####################################################################

qt_internal_add_test(tst_defaultmaskgenerator
    SOURCES
        tst_defaultmaskgenerator.cpp
    LIBRARIES
        Qt::WebSocketsPrivate
)

#### Keys ignored in scope 1:.:.:qdefaultmaskgenerator.pro:<TRUE>:
# TEMPLATE = "app"
# _REQUIREMENTS = "qtConfig(private_tests)"
