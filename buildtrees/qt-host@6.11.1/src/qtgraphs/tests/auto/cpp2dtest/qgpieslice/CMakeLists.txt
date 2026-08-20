# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgpieslice2d
    SOURCES
        tst_qgpieslice.cpp
    INCLUDE_DIRECTORIES
        ../common
    LIBRARIES
        Qt::Graphs
        Qt::Gui
        Qt::GuiPrivate
)
