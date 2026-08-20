# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_executable(qgraphs_qmlscenegrabber
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
        Qt::CorePrivate
)

# Resources:
qt_internal_add_resource(qgraphs_qmlscenegrabber "scenegrabber"
    PREFIX
        "/"
    FILES
        ${scenegrabber_resource_files}
)

# Turn it into a QML module which has no plugin (i.e. just a backing target)
qt6_add_qml_module(qgraphs_qmlscenegrabber
    URI "QtGraphs.Lancelot"
    VERSION "${PROJECT_VERSION}"
    NO_GENERATE_PLUGIN_SOURCE
    RESOURCE_PREFIX "/"
)
