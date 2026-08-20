# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## wasmdeployqt Tool:
#####################################################################

qt_get_tool_target_name(target_name wasmdeployqt)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Core
    USER_FACING
    INSTALL_VERSIONED_LINK
    TARGET_DESCRIPTION "Qt WebAssembly Deployment Tool"
    SOURCES
        main.cpp wasmbinary.cpp jsontools.cpp
    LIBRARIES
        Qt::CorePrivate
)
qt_internal_return_unless_building_tools()
