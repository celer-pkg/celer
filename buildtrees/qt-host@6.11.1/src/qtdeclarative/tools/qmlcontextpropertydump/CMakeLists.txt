# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_tool_target_name(target_name qmlcontextpropertydump)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "QML Context Property Dumper"
    TOOLS_TARGET Qml
    SOURCES
        main.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::QmlCompilerPrivate
        Qt::QmlToolingSettingsPrivate
)
qt_internal_return_unless_building_tools()
