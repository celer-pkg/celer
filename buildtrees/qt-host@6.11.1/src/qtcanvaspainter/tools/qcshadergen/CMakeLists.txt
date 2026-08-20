# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_tool_target_name(target_name qcshadergen)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET
        CanvasPainter
    SOURCES
        qcshadergen.cpp
    LIBRARIES
        Qt::Core
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}CanvasPainterMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}CanvasPainterMacros.cmake"
)
qt_internal_return_unless_building_tools()

qt_add_resources(${target_name}
    PREFIX
        /
    BASE
        shader_includes
    FILES
        shader_includes/common.glsl
        shader_includes/customvert.glsl
        shader_includes/customfrag.glsl
)
