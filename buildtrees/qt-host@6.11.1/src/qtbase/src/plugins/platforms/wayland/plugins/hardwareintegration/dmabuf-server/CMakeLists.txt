# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from dmabuf-server.pro.

#####################################################################
## DmaBufServerBufferPlugin Plugin:
#####################################################################

qt_internal_add_plugin(DmaBufServerBufferPlugin
    OUTPUT_NAME dmabuf-server
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        dmabufserverbufferintegration.cpp dmabufserverbufferintegration.h
        main.cpp
    LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::OpenGL
        Qt::WaylandClientPrivate
        Wayland::Client
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

qt6_generate_wayland_protocol_client_sources(DmaBufServerBufferPlugin
    PRIVATE_CODE
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../3rdparty/wayland/extensions/qt-dmabuf-server-buffer.xml
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../3rdparty/wayland/extensions/server-buffer-extension.xml
)

#### Keys ignored in scope 1:.:.:dmabuf-server.pro:<TRUE>:
# OTHER_FILES = "dmabuf-server.json"
# QMAKE_CXXFLAGS_WARN_ON = "--Wcast-qual"
