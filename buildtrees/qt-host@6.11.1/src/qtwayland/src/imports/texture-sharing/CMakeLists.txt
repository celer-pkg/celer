# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(QtWaylandSetup)
qt_internal_get_wayland_extensions_dir(extensions_dir)

# Generated from texture-sharing.pro.

#####################################################################
## qwaylandtexturesharing Plugin:
#####################################################################

qt_internal_add_qml_module(WaylandTextureSharing
    URI "QtWayland.Client.TextureSharing"
    PAST_MAJOR_VERSIONS 1
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    PLUGIN_TARGET WaylandTextureSharing
    CLASS_NAME QWaylandTextureSharingPlugin
    SOURCES
        plugin.cpp
        sharedtextureprovider.cpp sharedtextureprovider_p.h
        texturesharingextension.cpp texturesharingextension_p.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::OpenGL
        Qt::Qml
        Qt::QuickPrivate
        Qt::WaylandClient
        Qt::WaylandClientPrivate
    PRIVATE_HEADER_FILTERS
        "^qwayland-.*\.h|^wayland-.*-protocol\.h"
    NO_GENERATE_CPP_EXPORTS
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

qt6_generate_wayland_protocol_client_sources(WaylandTextureSharing
    PRIVATE_CODE
    FILES
        ${extensions_dir}/qt-texture-sharing-unstable-v1.xml
        ${extensions_dir}/server-buffer-extension.xml
)

qt_internal_add_autogen_sync_header_dependencies(WaylandTextureSharing WaylandCompositor)

#### Keys ignored in scope 1:.:.:texture-sharing.pro:<TRUE>:
# CXX_MODULE = "qml"
# IMPORT_VERSION = "1.$$QT_MINOR_VERSION"
# TARGETPATH = "QtWayland/Client/TextureSharing"
