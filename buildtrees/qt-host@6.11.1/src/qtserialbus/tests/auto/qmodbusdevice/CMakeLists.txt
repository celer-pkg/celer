# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbusdevice Test:
#####################################################################

qt_internal_add_test(tst_qmodbusdevice
    SOURCES
        tst_qmodbusdevice.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
