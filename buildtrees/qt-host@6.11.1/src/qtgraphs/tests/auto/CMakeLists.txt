# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT ANDROID)
    if(QT_FEATURE_graphs_3d)
        add_subdirectory(cpptest)
    endif()
    if(QT_FEATURE_graphs_2d)
        add_subdirectory(cpp2dtest)
    endif()
endif()

if(TARGET Qt::Quick AND NOT boot2qt)
    if(QT_FEATURE_graphs_3d AND QT_FEATURE_graphs_3d_bars3d AND QT_FEATURE_graphs_3d_scatter3d AND QT_FEATURE_graphs_3d_surface3d)
        add_subdirectory(qmltest)
    endif()
    if(QT_FEATURE_graphs_2d AND QT_FEATURE_graphs_2d_area AND QT_FEATURE_graphs_2d_bar AND QT_FEATURE_graphs_2d_donut_pie AND QT_FEATURE_graphs_2d_line AND QT_FEATURE_graphs_2d_scatter AND QT_FEATURE_graphs_2d_spline)
        add_subdirectory(qml2dtest)
    endif()
endif()
