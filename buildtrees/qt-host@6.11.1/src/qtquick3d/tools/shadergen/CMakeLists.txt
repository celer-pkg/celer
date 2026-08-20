# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from shadergen.pro.

#####################################################################
## shadergen Tool:
#####################################################################

qt_get_tool_target_name(target_name shadergen)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Quick3D # special case
    SOURCES
        genshaders.cpp genshaders.h
        main.cpp
        parser.cpp parser.h
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::Qml
        Qt::QmlPrivate
        Qt::Quick3DPrivate
        Qt::Quick3DRuntimeRender
        Qt::Quick3DRuntimeRenderPrivate
        Qt::ShaderToolsPrivate
)
qt_internal_return_unless_building_tools()
