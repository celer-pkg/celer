# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbuscommevent Test:
#####################################################################

qt_internal_add_test(tst_qmodbuscommevent
    SOURCES
        tst_qmodbuscommevent.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
)
