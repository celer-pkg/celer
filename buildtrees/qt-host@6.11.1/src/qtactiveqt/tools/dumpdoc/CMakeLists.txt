# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## dumpdoc Tool:
#####################################################################

qt_get_tool_target_name(target_name dumpdoc)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Active Qt DumpDoc"
    TOOLS_TARGET AxContainer
    SOURCES
        main.cpp
    LIBRARIES
        Qt::AxContainer
        Qt::Gui
        Qt::Widgets
)
qt_internal_return_unless_building_tools()
