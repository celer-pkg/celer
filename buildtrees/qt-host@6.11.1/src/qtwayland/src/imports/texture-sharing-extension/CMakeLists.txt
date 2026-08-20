# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from texture-sharing-extension.pro.

#####################################################################
## qwaylandtexturesharingextension Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandTextureSharingExtension
    URI "QtWayland.Compositor.TextureSharingExtension"
    PAST_MAJOR_VERSIONS 1
    CLASS_NAME QWaylandTextureSharingExtensionPlugin
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    PLUGIN_TARGET WaylandTextureSharingExtension
    SOURCES
        plugin.cpp plugin_p.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Qml
        Qt::QuickPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
    NO_GENERATE_CPP_EXPORTS
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

qt_internal_add_autogen_sync_header_dependencies(WaylandTextureSharingExtension WaylandCompositor)
#### Keys ignored in scope 1:.:.:texture-sharing-extension.pro:<TRUE>:
# CXX_MODULE = "qml"
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# TARGETPATH = "QtWayland/Compositor/TextureSharingExtension"
