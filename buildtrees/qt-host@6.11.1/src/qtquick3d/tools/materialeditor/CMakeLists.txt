# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_tool_target_name(target_name materialeditor)

set(materialeditor_uri "QtQuick3D.MaterialEditor")
set(materialeditor_asset_prefix "/qt-project.org/imports/QtQuick3D/MaterialEditor")

qt_internal_add_tool(${target_name}
    TOOLS_TARGET Quick3D
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::Quick3DUtils
        Qt::Quick3DGlslParserPrivate
        Qt::Quick3DAssetUtilsPrivate
        Qt::Quick3DRuntimeRenderPrivate
)

qt_internal_return_unless_building_tools()

qt_internal_add_resource(${target_name} "assets"
    PREFIX
        ${materialeditor_asset_prefix}
    FILES
        assets/meshes/suzanne.mesh
        assets/skybox/OpenfootageNET_lowerAustria01-1024.hdr
        assets/icons/lightdirectional.png
        assets/icons/lightdirectional@2x.png
        assets/icons/lightpoint.png
        assets/icons/lightpoint@2x.png
        assets/icons/lightspot.png
        assets/icons/lightspot@2x.png
        assets/icons/texture.png
        assets/icons/texture@2x.png
        assets/images/noise.png
)

qt_internal_add_qml_module(${target_name}
    VERSION 1.0
    URI ${materialeditor_uri}
    SOURCES
        materialadapter.h materialadapter.cpp
        syntaxhighlighter.h syntaxhighlighter.cpp
        uniformmodel.h uniformmodel.cpp
        buildmessage.h buildmessage.cpp
        custommaterial.h custommaterial.cpp
    QML_FILES
        main.qml
        ShaderEditor.qml
        EditorView.qml
        Preview.qml
        PreviewControls.qml
        FrostedGlass.qml
        AboutDialog.qml
        MaterialPropertiesPane.qml
        InstancingPropertiesPane.qml
        SaveChangesDialog.qml
        UniformManagerPane.qml
    NO_PLUGIN
    IMPORTS
        QtQuick3D
)

qt_add_resources(${target_name}
    PREFIX "/"
    FILES
        qtquickcontrols2.conf
)

qt_add_shaders(${target_name} "shaders"
    PREFIX
        ${materialeditor_asset_prefix}
    FILES
        "assets/shaders/frostedGlass.frag"
    SILENT
)
