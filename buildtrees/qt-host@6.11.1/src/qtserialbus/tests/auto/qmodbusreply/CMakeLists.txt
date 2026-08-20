# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbusreply Test:
#####################################################################

qt_internal_add_test(tst_qmodbusreply
    SOURCES
        tst_qmodbusreply.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
