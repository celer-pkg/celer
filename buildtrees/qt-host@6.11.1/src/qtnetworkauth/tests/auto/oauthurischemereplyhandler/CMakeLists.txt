# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_oauthurischemereplyhandler
    SOURCES
        tst_oauthurischemereplyhandler.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::NetworkAuth
        Qt::NetworkAuthPrivate
)
