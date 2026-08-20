# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## idc Tool:
#####################################################################

qt_get_tool_target_name(target_name idc)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Active Qt Interface Description Compiler"
    TOOLS_TARGET AxServer
    SOURCES
        main.cpp
)
qt_internal_return_unless_building_tools()
