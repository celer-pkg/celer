# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgsurface_proxy
    SOURCES
        tst_proxy.cpp
    LIBRARIES
        Qt::Gui
        Qt::Graphs
)
