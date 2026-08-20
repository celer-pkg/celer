# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbusadu Test:
#####################################################################

qt_internal_add_test(tst_qmodbusadu
    SOURCES
        tst_qmodbusadu.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
        Qt::SerialBusPrivate
)
