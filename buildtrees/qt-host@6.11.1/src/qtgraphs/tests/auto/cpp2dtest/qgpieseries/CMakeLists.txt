# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgpieseries2d
    SOURCES
        tst_qgpieseries.cpp
    INCLUDE_DIRECTORIES
        ../common
    LIBRARIES
        Qt::Graphs
        Qt::Gui
        Qt::GuiPrivate
)
