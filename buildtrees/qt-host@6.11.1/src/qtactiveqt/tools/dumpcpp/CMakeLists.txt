# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## dumpcpp Tool:
#####################################################################

qt_get_tool_target_name(target_name dumpcpp)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Active Qt DumpCpp"
    TOOLS_TARGET AxContainer
    SOURCES
        main.cpp
        moc.cpp moc.h
    DEFINES
        QT_ASCII_CAST_WARNINGS
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::AxContainer
        Qt::CorePrivate
        Qt::Gui
        Qt::Widgets
)
qt_internal_return_unless_building_tools()
