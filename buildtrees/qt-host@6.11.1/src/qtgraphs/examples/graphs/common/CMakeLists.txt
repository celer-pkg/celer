# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

if(TARGET Qt::Quick)
    if(TARGET Qt6::PrintSupport AND QT_FEATURE_graphs_3d_bars3d AND QT_FEATURE_graphs_2d_line)
        qt_internal_add_example(graphprinting)
    endif()
    if(QT_FEATURE_graphs_3d_bars3d AND QT_FEATURE_graphs_3d_scatter3d AND QT_FEATURE_graphs_3d_surface3d AND QT_FEATURE_graphs_2d_area AND QT_FEATURE_graphs_2d_bar AND QT_FEATURE_graphs_2d_donut_pie AND QT_FEATURE_graphs_2d_spline)
        qt_internal_add_example(aerospacehub)
    endif()
endif()

