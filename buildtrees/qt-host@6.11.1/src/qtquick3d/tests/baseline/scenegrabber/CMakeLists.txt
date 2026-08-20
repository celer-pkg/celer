# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from scenegrabber.pro.

#####################################################################
## qmlscenegrabber Binary:
#####################################################################

qt_internal_add_executable(qquick3d_qmlscenegrabber
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        dynamicgeometry.cpp dynamicgeometry.h
        dynamictexturedata.cpp dynamictexturedata.h
        indextrianglestopology.cpp indextrianglestopology.h
        main.cpp
        nonindextrianglestopology.cpp nonindextrianglestopology.h
        pointstopology.cpp pointstopology.h
        skingeometry.cpp skingeometry.h
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3DPrivate
)

# Resources:
set(scenegrabber_resource_files
    "trim.ttf"
)

qt_internal_add_resource(qquick3d_qmlscenegrabber "scenegrabber"
    PREFIX
        "/"
    FILES
        ${scenegrabber_resource_files}
)

qt6_add_lightprobe_images(qquick3d_qmlscenegrabber "ibl_assets"
    PREFIX
        "/"
    BASE
        "../data/shared/maps"
    FILES
        "../data/shared/maps/OpenfootageNET_lowerAustria01-1024.hdr"
        "../data/shared/maps/TestEnvironment-512.exr"
)

# Turn it into a QML module which has no plugin (i.e. just a backing target)
qt6_add_qml_module(qquick3d_qmlscenegrabber
    URI "QtQuick3D.Lancelot"
    VERSION "${PROJECT_VERSION}"
    NO_GENERATE_PLUGIN_SOURCE
    RESOURCE_PREFIX "/"
)
