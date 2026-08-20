# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbusdeviceidentification Test:
#####################################################################

qt_internal_add_test(tst_qmodbusdeviceidentification
    SOURCES
        tst_qmodbusdeviceidentification.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
