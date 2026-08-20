# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_test(q3dscatter-modelproxy_datavis
    SOURCES
        tst_proxy.cpp
    LIBRARIES
        Qt::Gui
        Qt::Widgets
        Qt::DataVisualization
)
