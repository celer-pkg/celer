# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbusserver Test:
#####################################################################

qt_internal_add_test(tst_qmodbusserver
    SOURCES
        tst_qmodbusserver.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
