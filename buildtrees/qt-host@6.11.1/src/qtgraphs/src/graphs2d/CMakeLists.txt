# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_configure_add_summary_section(NAME "Qt Graphs 2D")
qt_configure_add_summary_entry(ARGS "graphs-2d-area")
qt_configure_add_summary_entry(ARGS "graphs-2d-bar")
qt_configure_add_summary_entry(ARGS "graphs-2d-donut-pie") # These are tightly wound together
qt_configure_add_summary_entry(ARGS "graphs-2d-line")
qt_configure_add_summary_entry(ARGS "graphs-2d-scatter")
qt_configure_add_summary_entry(ARGS "graphs-2d-spline")
qt_configure_add_summary_entry(ARGS "graphs-2d-custom")
qt_configure_end_summary_section() # end of "Qt Graphs 2D" section

if(NOT QT_FEATURE_graphs_2d_area AND NOT QT_FEATURE_graphs_2d_bar AND NOT QT_FEATURE_graphs_2d_donut_pie AND NOT QT_FEATURE_graphs_2d_line AND NOT QT_FEATURE_graphs_2d_scatter AND NOT QT_FEATURE_graphs_2d_spline AND NOT QT_FEATURE_graphs_2d_custom)
    message("You must have one of the 2D features ON, or nothing will be built.")
else()
    qt_internal_extend_target(Graphs
        SOURCES
            qgraphsview.cpp qgraphsview_p.h
            qabstractseries.cpp qabstractseries.h qabstractseries_p.h
            charthelpers_p.h

            axis/qabstractaxis.cpp axis/qabstractaxis.h axis/qabstractaxis_p.h
            axis/axisgrid.cpp axis/axisgrid_p.h
            axis/axisticker.cpp axis/axisticker_p.h
            axis/axisline.cpp axis/axisline_p.h
            axis/valueaxis/qvalueaxis.cpp axis/valueaxis/qvalueaxis.h axis/valueaxis/qvalueaxis_p.h
            axis/datetimeaxis/qdatetimeaxis.h axis/datetimeaxis/qdatetimeaxis.cpp axis/datetimeaxis/qdatetimeaxis_p.h

            qsgrenderer/axisrenderer.cpp qsgrenderer/axisrenderer_p.h

            animation/qgraphanimation.cpp animation/qgraphanimation_p.h
        INCLUDE_DIRECTORIES
            axis
            qsgrenderer
            animation
    )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_area OR QT_FEATURE_graphs_2d_line OR QT_FEATURE_graphs_2d_scatter OR QT_FEATURE_graphs_2d_spline
        SOURCES
            xychart/qxyseries.cpp xychart/qxyseries.h xychart/qxyseries_p.h
            xychart/qxypoint.cpp xychart/qxypoint_p.h
            xychart/qxymodelmapper.cpp xychart/qxymodelmapper.h xychart/qxymodelmapper_p.h

            qsgrenderer/pointrenderer.cpp qsgrenderer/pointrenderer_p.h

            animation/qgraphpointanimation.cpp animation/qgraphpointanimation_p.h
            animation/qgraphtransition_p.h animation/qgraphtransition.cpp
            animation/qxyseriesanimation_p.h animation/qxyseriesanimation.cpp
        DEFINES
            USE_POINTS
        )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_area
        SOURCES
            areachart/qareaseries.cpp areachart/qareaseries.h areachart/qareaseries_p.h

            qsgrenderer/arearenderer.cpp qsgrenderer/arearenderer_p.h
        DEFINES
            USE_AREAGRAPH
    )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_bar
        SOURCES
            barchart/qbarseries.cpp barchart/qbarseries.h barchart/qbarseries_p.h
            barchart/qbarset.cpp barchart/qbarset.h barchart/qbarset_p.h
            barchart/qbarmodelmapper.cpp barchart/qbarmodelmapper.h barchart/qbarmodelmapper_p.h

            axis/barcategoryaxis/qbarcategoryaxis.cpp axis/barcategoryaxis/qbarcategoryaxis.h axis/barcategoryaxis/qbarcategoryaxis_p.h

            qsgrenderer/barsrenderer.cpp qsgrenderer/barsrenderer_p.h
        DEFINES
            USE_BARGRAPH
    )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_donut_pie
        SOURCES
            piechart/qpieseries.cpp piechart/qpieseries.h piechart/qpieseries_p.h
            piechart/qpieslice.cpp piechart/qpieslice.h piechart/qpieslice_p.h
            piechart/qpiemodelmapper.cpp piechart/qpiemodelmapper.h piechart/qpiemodelmapper_p.h

            qsgrenderer/pierenderer.cpp qsgrenderer/pierenderer_p.h
        DEFINES
            USE_PIEGRAPH
    )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_line OR QT_FEATURE_graphs_2d_area
        # area graph requires at least line series support
        SOURCES
            linechart/qlineseries.cpp linechart/qlineseries.h linechart/qlineseries_p.h
        DEFINES
            USE_LINEGRAPH
    )
    qt_internal_extend_target(Graphs CONDITION (QT_FEATURE_graphs_2d_line OR QT_FEATURE_graphs_2d_area) AND MSVC
        NO_UNITY_BUILD_SOURCES
            # QDebug lookup issue with QList<qreal>
            linechart/qlineseries.cpp
    )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_scatter
        SOURCES
            scatterchart/qscatterseries.cpp scatterchart/qscatterseries.h scatterchart/qscatterseries_p.h
        DEFINES
            USE_SCATTERGRAPH
    )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_spline
        SOURCES
            splinechart/qsplineseries.cpp splinechart/qsplineseries.h splinechart/qsplineseries_p.h

            animation/qsplinecontrolanimation.cpp animation/qsplinecontrolanimation_p.h
        DEFINES
            USE_SPLINEGRAPH
    )
    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_2d_custom
        SOURCES
            customchart/qcustomseries.cpp customchart/qcustomseries.h customchart/qcustomseries_p.h
            customchart/qcustomseriesdata.cpp customchart/qcustomseriesdata_p.h

            qsgrenderer/customrenderer.cpp qsgrenderer/customrenderer_p.h
        DEFINES
            USE_CUSTOMGRAPH
    )

    set(LOCAL_TRACE_SOURCE_2D
            graphs2d/qsgrenderer/axisrenderer.cpp
            graphs2d/qgraphsview.cpp
    )


    if(QT_FEATURE_graphs_2d_bar)
        set(LOCAL_TRACE_SOURCE_2D
                ${LOCAL_TRACE_SOURCE_2D}
                graphs2d/qsgrenderer/barsrenderer.cpp
                graphs2d/barchart/qbarmodelmapper.cpp
        )
    endif()

    if(QT_FEATURE_graphs_2d_donut_pie)
        set(LOCAL_TRACE_SOURCE_2D
                ${LOCAL_TRACE_SOURCE_2D}
                graphs2d/qsgrenderer/pierenderer.cpp
                graphs2d/piechart/qpiemodelmapper.cpp
                graphs2d/piechart/qpieseries.cpp
        )
    endif()

    if(QT_FEATURE_graphs_2d_spline)
        set(LOCAL_TRACE_SOURCE_2D
                ${LOCAL_TRACE_SOURCE_2D}
                graphs2d/splinechart/qsplineseries.cpp
        )
    endif()

    if(QT_FEATURE_graphs_2d_area)
        set(LOCAL_TRACE_SOURCE_2D
                ${LOCAL_TRACE_SOURCE_2D}
                graphs2d/qsgrenderer/arearenderer.cpp
        )
    endif()
    if(QT_FEATURE_graphs_2d_area OR QT_FEATURE_graphs_2d_line OR QT_FEATURE_graphs_2d_scatter
        OR QT_FEATURE_graphs_2d_spline)
        set(LOCAL_TRACE_SOURCE_2D
                ${LOCAL_TRACE_SOURCE_2D}
                graphs2d/qsgrenderer/pointrenderer.cpp
                graphs2d/xychart/qxymodelmapper.cpp
        )
    endif()

    if(QT_FEATURE_graphs_2d_custom)
        set(LOCAL_TRACE_SOURCE_2D
                ${LOCAL_TRACE_SOURCE_2D}
                graphs2d/qsgrenderer/customrenderer.cpp
        )
    endif()

    set(TRACE_SOURCES_2D
            ${LOCAL_TRACE_SOURCE_2D}
        PARENT_SCOPE
    )
endif()

add_subdirectory(qml/designer)
