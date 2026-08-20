# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(tst_qgscatter_series
    SOURCES
        tst_series.cpp
    LIBRARIES
        Qt::Gui
        Qt::Graphs
)
