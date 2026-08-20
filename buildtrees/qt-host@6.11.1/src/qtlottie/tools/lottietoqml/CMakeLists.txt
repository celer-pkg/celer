# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## lottietoqml Tool:
#####################################################################

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS QuickVectorImageGeneratorPrivate)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS QuickVectorImageGeneratorPrivate)

qt_get_tool_target_name(target_name lottietoqml)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Lottie to QML Converter"
    TOOLS_TARGET Lottie
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}LottieToQmlMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}LottieToQmlMacros.cmake"
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickPrivate
        Qt::QuickVectorImageGeneratorPrivate
        Qt::LottieVectorImageGeneratorPrivate
        Qt::LottiePrivate
)

qt_internal_return_unless_building_tools()

set(resource_files
    "main.qml"
)

qt_internal_add_resource(${target_name} "qml"
    PREFIX
        "/"
    FILES
        ${resource_files}
)
