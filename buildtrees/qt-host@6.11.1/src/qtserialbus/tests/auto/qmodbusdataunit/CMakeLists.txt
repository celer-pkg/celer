# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmodbusdataunit Test:
#####################################################################

qt_internal_add_test(tst_qmodbusdataunit
    SOURCES
        tst_qmodbusdataunit.cpp
    LIBRARIES
        Qt::Network
        Qt::SerialBus
)
