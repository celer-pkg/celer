# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from libhybris-egl-server.pro.

#####################################################################
## LibHybrisEglServerBufferPlugin Plugin:
#####################################################################

qt_internal_add_plugin(LibHybrisEglServerBufferPlugin
    OUTPUT_NAME libhybris-egl-server
    PLUGIN_TYPE wayland-graphics-integration-client
    SOURCES
        libhybriseglserverbufferintegration.cpp libhybriseglserverbufferintegration.h
        main.cpp
    PUBLIC_LIBRARIES
        EGL::EGL
        Qt::Core
        Qt::Gui
        Qt::WaylandClientPrivate
        Wayland::Client
    QT_LICENSE_ID QT_COMMERCIAL_OR_LGPL3
)

qt6_generate_wayland_protocol_client_sources(LibHybrisEglServerBufferPlugin
    PRIVATE_CODE
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../3rdparty/wayland/extensions/libhybris-egl-server-buffer.xml
)

#### Keys ignored in scope 1:.:.:libhybris-egl-server.pro:<TRUE>:
# OTHER_FILES = "libhybris-egl-server.json"
