# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Quick)
    if(QT_FEATURE_graphs_2d_bar AND QT_FEATURE_graphs_2d_line)
        qt_internal_add_example(hellographs)
    endif()
    if(QT_FEATURE_graphs_2d_area AND QT_FEATURE_graphs_2d_bar AND QT_FEATURE_graphs_2d_donut_pie AND QT_FEATURE_graphs_2d_line AND QT_FEATURE_graphs_2d_scatter AND QT_FEATURE_graphs_2d_spline)
        qt_internal_add_example(cockpit)
    endif()
endif()
if(NOT ANDROID AND NOT IOS AND NOT WINRT)
    if(QT_FEATURE_graphs_2d_donut_pie)
        qt_internal_add_example(quickwidgetgraphs)
    endif()
endif()
