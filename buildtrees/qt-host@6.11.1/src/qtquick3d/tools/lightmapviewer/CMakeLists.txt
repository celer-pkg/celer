# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_tool_target_name(target_name lightmapviewer)

set(lightmapviewer_uri "QtQuick3D.lightmapviewer")
set(lightmapviewer_asset_prefix "/qt-project.org/imports/QtQuick3D/lightmapviewer")

qt_internal_add_tool(${target_name}
    TOOLS_TARGET Quick3D
    SOURCES
        lightmapfile.cpp
        lightmapfile.h
        lightmapimageprovider.cpp
        lightmapimageprovider.h
        lightmapviewer.cpp
        lightmapviewerhelpers.cpp
        lightmapviewerhelpers.h
        lightmapmesh.h
        lightmapmesh.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRender
        Qt::Quick3DAssetImportPrivate
        Qt::Quick3DRuntimeRenderPrivate
)

qt_internal_return_unless_building_tools()

qt_internal_add_resource(${target_name} "assets"
    PREFIX
        ${lightmapviewer_asset_prefix}
    FILES
        grid.png
        mesh.vert
        mesh.frag
)

qt_internal_add_qml_module(${target_name}
    VERSION 1.0
    URI ${lightmapviewer_uri}
    QML_FILES
        LightmapViewer.qml
        SceneMetadataView.qml
        MeshViewer.qml
        ImageViewer.qml
        ArcballController.qml
        OriginGizmo.qml
    NO_PLUGIN
    IMPORTS
        QtQuick3D
)
