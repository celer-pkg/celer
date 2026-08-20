# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from balsam.pro.

#####################################################################
## balsam Tool:
#####################################################################

qt_get_tool_target_name(target_name balsam)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Quick3D # special case
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Qml # special case
        Qt::Gui
        Qt::Quick3DAssetImportPrivate
        Qt::Quick3DIblBakerPrivate
)
qt_internal_return_unless_building_tools()
