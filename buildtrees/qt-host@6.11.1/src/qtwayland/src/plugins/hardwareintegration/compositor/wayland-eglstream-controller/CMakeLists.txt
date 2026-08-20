# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(QtWaylandSetup)
qt_internal_get_wayland_protocols_dir(protocols_dir)

# Generated from wayland-eglstream-controller.pro.

#####################################################################
## QWaylandEglStreamClientBufferIntegrationPlugin Plugin:
#####################################################################
qt_find_package(EGL) # special case

qt_internal_add_plugin(QWaylandEglStreamClientBufferIntegrationPlugin
    OUTPUT_NAME qt-wayland-compositor-wayland-eglstream-controller
    PLUGIN_TYPE wayland-graphics-integration-server
    SOURCES
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamcontroller.h
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.cpp ../../../../hardwareintegration/compositor/wayland-eglstream-controller/waylandeglstreamintegration.h
        main.cpp
    INCLUDE_DIRECTORIES
        ../../../../hardwareintegration/compositor/wayland-eglstream-controller
    LIBRARIES
        EGL::EGL
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WaylandCompositor
        Qt::WaylandCompositorPrivate
        Wayland::Egl
        Wayland::Server
    ATTRIBUTION_FILE_DIR_PATHS
        ${protocols_dir}/wl-eglstream
)

qt6_generate_wayland_protocol_server_sources(QWaylandEglStreamClientBufferIntegrationPlugin
    FILES
        ${protocols_dir}/wl-eglstream/wl-eglstream-controller.xml
)

#### Keys ignored in scope 1:.:.:wayland-eglstream-controller.pro:<TRUE>:
# OTHER_FILES = "wayland-eglstream-controller.json"
