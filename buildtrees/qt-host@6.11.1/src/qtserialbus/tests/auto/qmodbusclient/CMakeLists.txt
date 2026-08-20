# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_qmodbusclient Test:
#####################################################################

qt_internal_add_test(tst_qmodbusclient
    SOURCES
        tst_qmodbusclient.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
)

