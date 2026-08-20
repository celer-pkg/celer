# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qmodbusrtuserialclient
    SOURCES
        tst_qmodbusrtuserialclient.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
