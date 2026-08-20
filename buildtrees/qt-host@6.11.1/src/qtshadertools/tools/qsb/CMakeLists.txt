# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qsb.pro.

#####################################################################
## qsb Tool:
#####################################################################

qt_get_tool_target_name(target_name qsb)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET # special case
        ShaderTools # special case
    SOURCES
        qsb.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
        Qt::ShaderToolsPrivate
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}ShaderToolsMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}ShaderToolsMacros.cmake"
)
qt_internal_return_unless_building_tools()

#### Keys ignored in scope 1:.:.:qsb.pro:<TRUE>:
# _REQUIREMENTS = "qtConfig(process)"
