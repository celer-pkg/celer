# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(qml_files
    "designer/Bars3DSpecifics.qml"
    "designer/Scatter3DSpecifics.qml"
    "designer/Surface3DSpecifics.qml"
    "designer/default/Bars3D.qml"
    "designer/default/Scatter3D.qml"
    "designer/default/Surface3D.qml"
)

set(resources
    "designer/qtdatavisualization.metainfo"
    "designer/images/bars3d-icon.png"
    "designer/images/bars3d-icon16.png"
    "designer/images/scatter3d-icon.png"
    "designer/images/scatter3d-icon16.png"
    "designer/images/surface3d-icon.png"
    "designer/images/surface3d-icon16.png"
)

set_source_files_properties(${qml_files} PROPERTIES
    QT_QML_SKIP_QMLDIR_ENTRY TRUE
)

qt_internal_add_qml_module(DataVisualizationQml
    URI "QtDataVisualization"
    VERSION "${PROJECT_VERSION}"
    PAST_MAJOR_VERSIONS 1
    DEPENDENCIES
        QtQuick
    SOURCES
        abstractdeclarative.cpp abstractdeclarative_p.h
        colorgradient.cpp colorgradient_p.h
        declarativebars.cpp declarativebars_p.h
        declarativecolor.cpp declarativecolor_p.h
        declarativerendernode.cpp declarativerendernode_p.h
        declarativescatter.cpp declarativescatter_p.h
        declarativescene.cpp declarativescene_p.h
        declarativeseries.cpp declarativeseries_p.h
        declarativesurface.cpp declarativesurface_p.h
        declarativetheme.cpp declarativetheme_p.h
        enumtostringmap.cpp enumtostringmap_p.h
        foreigntypes_p.h
        glstatestore.cpp glstatestore_p.h
    NO_PCH_SOURCES
        declarativetheme.cpp # undef QT_NO_FOREACH
    QML_FILES
        ${qml_files}
    RESOURCES
        ${resources}
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::OpenGL
        Qt::Qml
        Qt::Quick
        Qt::DataVisualizationPrivate
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_extend_target(DataVisualizationQml CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::GuiPrivate
)
