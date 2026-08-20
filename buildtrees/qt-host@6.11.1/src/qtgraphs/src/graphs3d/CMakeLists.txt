# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_configure_add_summary_section(NAME "Qt Graphs 3D")
qt_configure_add_summary_entry(ARGS "graphs-3d-bars3d")
qt_configure_add_summary_entry(ARGS "graphs-3d-scatter3d")
qt_configure_add_summary_entry(ARGS "graphs-3d-surface3d")
qt_configure_end_summary_section() # end of "Qt Graphs 3D" section

if(NOT QT_FEATURE_graphs_3d_bars3d AND NOT QT_FEATURE_graphs_3d_scatter3d AND NOT QT_FEATURE_graphs_3d_surface3d)
    message("You must have either FEATURE_graphs_3d_bars3d, FEATURE_graphs_3d_scatter3d, or FEATURE_graphs_3d_surface3d ON, or nothing will be built.")
else()
    qt_internal_extend_target(Graphs
        SOURCES
            axis/qabstract3daxis.cpp axis/qabstract3daxis.h axis/qabstract3daxis_p.h
            axis/qcategory3daxis.cpp axis/qcategory3daxis.h axis/qcategory3daxis_p.h
            axis/qlogvalue3daxisformatter.cpp axis/qlogvalue3daxisformatter.h axis/qlogvalue3daxisformatter_p.h
            axis/qvalue3daxis.cpp axis/qvalue3daxis.h axis/qvalue3daxis_p.h
            axis/qvalue3daxisformatter.cpp axis/qvalue3daxisformatter.h axis/qvalue3daxisformatter_p.h

            data/abstractitemmodelhandler.cpp data/abstractitemmodelhandler_p.h
            data/qabstract3dseries.cpp data/qabstract3dseries.h data/qabstract3dseries_p.h
            data/qabstractdataproxy.cpp data/qabstractdataproxy.h data/qabstractdataproxy_p.h
            data/qcustom3ditem.cpp data/qcustom3ditem.h data/qcustom3ditem_p.h
            data/qcustom3dlabel.cpp data/qcustom3dlabel.h data/qcustom3dlabel_p.h
            data/qcustom3dvolume.cpp data/qcustom3dvolume.h data/qcustom3dvolume_p.h

            engine/q3dscene.cpp engine/q3dscene.h engine/q3dscene_p.h

            input/qgraphsinputhandler.cpp  input/qgraphsinputhandler_p.h

            utils/qutils.h
            utils/utils.cpp utils/utils_p.h
            utils/qgraphs3dnamespace.cpp utils/qgraphs3dnamespace.h
            utils/qgraphs3dlogging.cpp utils/qgraphs3dlogging_p.h

            qml/qquickgraphsitem.cpp qml/qquickgraphsitem_p.h
            qml/qquickgraphsnode.cpp qml/qquickgraphsnode_p.h

            qml/qquickgraphstexturedata.cpp qml/qquickgraphstexturedata_p.h
            qml/gradientholder_p.h
        INCLUDE_DIRECTORIES
            axis
            data
            engine
            input
            utils
            qml
            ../common/utils/
    )

    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_3d_bars3d
        SOURCES
            data/baritemmodelhandler.cpp data/baritemmodelhandler_p.h
            data/qbar3dseries.cpp data/qbar3dseries.h data/qbar3dseries_p.h
            data/qbardataitem.cpp data/qbardataitem.h
            data/qbardataproxy.cpp data/qbardataproxy.h data/qbardataproxy_p.h
            data/qitemmodelbardataproxy.cpp data/qitemmodelbardataproxy.h data/qitemmodelbardataproxy_p.h

            engine/barinstancing.cpp engine/barinstancing_p.h

            qml/qquickgraphsbars.cpp qml/qquickgraphsbars_p.h
            qml/qquickgraphsbarsnode.cpp qml/qquickgraphsbarsnode_p.h
            qml/qquickgraphsbarsseries.cpp qml/qquickgraphsbarsseries_p.h
        INCLUDE_DIRECTORIES
            data
            engine
            qml
    )

    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_3d_scatter3d
        SOURCES
            data/qitemmodelscatterdataproxy.cpp data/qitemmodelscatterdataproxy.h data/qitemmodelscatterdataproxy_p.h
            data/qscatter3dseries.cpp data/qscatter3dseries.h data/qscatter3dseries_p.h
            data/qspline3dseries.cpp data/qspline3dseries.h data/qspline3dseries_p.h
            data/qscatterdataitem.cpp data/qscatterdataitem.h
            data/qscatterdataproxy.cpp data/qscatterdataproxy.h data/qscatterdataproxy_p.h
            data/scatteritemmodelhandler.cpp data/scatteritemmodelhandler_p.h

            engine/scatterinstancing.cpp engine/scatterinstancing_p.h

            qml/qquickgraphsscatter.cpp qml/qquickgraphsscatter_p.h
            qml/qquickgraphsscatternode.cpp qml/qquickgraphsscatternode_p.h
            qml/qquickgraphsscatterseries.cpp qml/qquickgraphsscatterseries_p.h
            qml/qquickgraphssplineseries.cpp qml/qquickgraphssplineseries_p.h
        INCLUDE_DIRECTORIES
            data
            engine
            qml
    )

    qt_internal_extend_target(Graphs CONDITION QT_FEATURE_graphs_3d_surface3d
        SOURCES
            data/qheightmapsurfacedataproxy.cpp data/qheightmapsurfacedataproxy.h data/qheightmapsurfacedataproxy_p.h
            data/qitemmodelsurfacedataproxy.cpp data/qitemmodelsurfacedataproxy.h data/qitemmodelsurfacedataproxy_p.h
            data/qsurface3dseries.cpp data/qsurface3dseries.h data/qsurface3dseries_p.h
            data/qsurfacedataitem.cpp data/qsurfacedataitem.h
            data/qsurfacedataproxy.cpp data/qsurfacedataproxy.h data/qsurfacedataproxy_p.h
            data/surfaceitemmodelhandler.cpp data/surfaceitemmodelhandler_p.h

            qml/qquickgraphssurface.cpp qml/qquickgraphssurface_p.h
            qml/qquickgraphssurfacenode.cpp qml/qquickgraphssurfacenode_p.h
            qml/qquickgraphssurfaceseries.cpp qml/qquickgraphssurfaceseries_p.h
        INCLUDE_DIRECTORIES
            data
            engine
            qml
    )

    qt_internal_extend_target(Graphs CONDITION MACOS
        LIBRARIES
            Qt::GuiPrivate
        PUBLIC_LIBRARIES
            Qt::Gui
        PRIVATE_MODULE_INTERFACE
            Qt::GuiPrivate
    )

    add_subdirectory(qml/designer)

    set(
        LOCAL_TRACE_SOURCES
            graphs3d/qml/qquickgraphsitem.cpp
            graphs3d/qml/qquickgraphstexturedata.cpp
            graphs3d/data/qcustom3dvolume.cpp
    )

    if(QT_FEATURE_graphs_3d_surface3d)
        set(
            LOCAL_TRACE_SOURCES
                ${LOCAL_TRACE_SOURCES}
                graphs3d/qml/qquickgraphssurface.cpp
                graphs3d/data/qheightmapsurfacedataproxy.cpp
                graphs3d/data/surfaceitemmodelhandler.cpp
        )
    endif()


    if(QT_FEATURE_graphs_3d_scatter3d)
        set(
            LOCAL_TRACE_SOURCES
                ${LOCAL_TRACE_SOURCES}
                graphs3d/qml/qquickgraphsscatter.cpp
                graphs3d/data/scatteritemmodelhandler.cpp
        )
    endif()

    if(QT_FEATURE_graphs_3d_bars3d)
        set(
            LOCAL_TRACE_SOURCES
                ${LOCAL_TRACE_SOURCES}
                graphs3d/data/baritemmodelhandler.cpp
                graphs3d/qml/qquickgraphsbars.cpp
        )
    endif()

    set(
        TRACE_SOURCES
            ${LOCAL_TRACE_SOURCES}
        PARENT_SCOPE
    )
endif()
if(TARGET Qt::Widgets)
    add_subdirectory(widget)
endif()
