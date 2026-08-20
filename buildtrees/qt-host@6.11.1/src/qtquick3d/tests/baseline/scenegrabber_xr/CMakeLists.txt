# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(QT_ANDROID_DEPLOYMENT_TYPE Debug) # no release package signing, no matter what the build type is

qt_internal_add_executable(qquick3d_qmlscenegrabber_xr
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/.." # special case
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Core
        Qt::GuiPrivate
        Qt::QuickPrivate
        Qt::Quick3DPrivate
        Qt::Quick3DXrPrivate
)

if(ANDROID)
    set_property(TARGET qquick3d_qmlscenegrabber_xr APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()

# Resources:
set(scenegrabber_xr_assets
    lazer.mesh
    Lazer_baseColor.png
    Lazer_metalness.png
)

# Turn it into a QML module which has no plugin (i.e. just a backing target)
qt6_add_qml_module(qquick3d_qmlscenegrabber_xr
    QML_FILES
        main.qml
        Lazer.qml
        Beam.qml
        ActionMapper.qml
    URI "QtQuick3D.Lancelot.Xr"
    VERSION "${PROJECT_VERSION}"
    NO_GENERATE_PLUGIN_SOURCE
    RESOURCES
       ${scenegrabber_xr_assets}
)
