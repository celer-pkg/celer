# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbuspdu Test:
#####################################################################

qt_internal_add_test(tst_qmodbuspdu
    SOURCES
        tst_qmodbuspdu.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
