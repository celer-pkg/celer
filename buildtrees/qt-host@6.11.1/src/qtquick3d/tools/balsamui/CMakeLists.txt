# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from balsamui.pro.

#####################################################################
## balsamui Tool:
#####################################################################

qt_get_tool_target_name(target_name balsamui)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Quick3D
    SOURCES
        converterthread.cpp converterthread.h
        inputlistview.cpp inputlistview.h
        inputtab.cpp inputtab.h
        main.cpp
        mainwindow.cpp mainwindow.h
        settingstab.cpp settingstab.h
    LIBRARIES
        Qt::Concurrent
        Qt::Gui
        Qt::Quick3DAssetImportPrivate
        Qt::Widgets
)
qt_internal_return_unless_building_tools()
