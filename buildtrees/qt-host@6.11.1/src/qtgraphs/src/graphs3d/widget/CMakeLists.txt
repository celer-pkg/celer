# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(GraphsWidgets
    SOURCES
        qgraphswidgetsglobal.h
        q3dgraphswidgetitem.cpp q3dgraphswidgetitem.h q3dgraphswidgetitem_p.h
    DEFINES
        QT_NO_FOREACH
        QT_NO_INTEGER_EVENT_COORDINATES
        QT_NO_URL_CAST_FROM_STRING
    LIBRARIES
        Qt::GraphsPrivate
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Graphs
        Qt::QuickWidgets
    PRIVATE_MODULE_INTERFACE
        Qt::GraphsPrivate
        Qt::CorePrivate
)

qt_internal_extend_target(GraphsWidgets CONDITION QT_FEATURE_graphs_3d_bars3d
    SOURCES
        q3dbarswidgetitem.cpp q3dbarswidgetitem.h q3dbarswidgetitem_p.h
)

qt_internal_extend_target(GraphsWidgets CONDITION QT_FEATURE_graphs_3d_scatter3d
    SOURCES
        q3dscatterwidgetitem.cpp q3dscatterwidgetitem.h q3dscatterwidgetitem_p.h
)

qt_internal_extend_target(GraphsWidgets CONDITION QT_FEATURE_graphs_3d_surface3d
    SOURCES
        q3dsurfacewidgetitem.cpp q3dsurfacewidgetitem.h q3dsurfacewidgetitem_p.h
)

# Include required QML plugins for static build
if (NOT QT_BUILD_SHARED_LIBS)
    qt_internal_add_resource(GraphsWidgets "qml_for_widget"
      PREFIX
      "/qml"
      BASE
      "qml"
      FILES
        "./qml/dummy.qml"
    )

    qt_import_qml_plugins(GraphsWidgets)
endif()
