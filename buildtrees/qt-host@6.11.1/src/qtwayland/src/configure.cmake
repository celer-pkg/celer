# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# configure.cmake for the QtWaylandGlobalPrivate module

#### Inputs

#### Libraries

if(LINUX OR QT_FIND_ALL_PACKAGES_ALWAYS)
    # waylandclient libraries
    if(TARGET Wayland::Client)
        qt_internal_disable_find_package_global_promotion(Wayland::Client)
    endif()
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Client
        MODULE_NAME waylandclient
        QMAKE_LIB wayland-client)

    if(TARGET Wayland::Cursor)
        qt_internal_disable_find_package_global_promotion(Wayland::Cursor)
    endif()
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Cursor
        MODULE_NAME waylandclient
        QMAKE_LIB wayland-cursor)
    qt_add_qmake_lib_dependency(wayland-cursor wayland-client)

    if(TARGET Wayland::Egl)
        qt_internal_disable_find_package_global_promotion(Wayland::Egl)
    endif()
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Egl
        MODULE_NAME waylandclient
        QMAKE_LIB wayland-egl)

    # waylandcompositor libraries
    if(TARGET Wayland::Server)
        qt_internal_disable_find_package_global_promotion(Wayland::Server)
    endif()
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Server
        MODULE_NAME waylandcompositor
        QMAKE_LIB wayland-server)
    qt_find_package(Wayland
        PROVIDED_TARGETS Wayland::Egl
        MODULE_NAME waylandcompositor
        QMAKE_LIB wayland-egl)

    # X11 is not a public dependency of QtGui anymore, so we need to find it manually in a shared build.
    # In a static build the dependency is still propagated, so check for the target existence to prevent
    # the 'Attempt to promote imported target "X11::X11" to global scope' issue.
    if(NOT TARGET X11::X11)
        qt_find_package(X11 PROVIDED_TARGETS X11::X11 MODULE_NAME gui QMAKE_LIB xlib)
    endif()
    # Same for XKB.
    if(NOT TARGET XKB::XKB)
        qt_find_package(XKB 0.5.0 PROVIDED_TARGETS XKB::XKB MODULE_NAME gui QMAKE_LIB xkbcommon MARK_OPTIONAL)
    endif()
    # EGL
    if(NOT TARGET EGL::EGL)
        qt_find_package(EGL PROVIDED_TARGETS EGL::EGL MODULE_NAME gui QMAKE_LIB egl MARK_OPTIONAL)
    endif()
    # and Libdrm
    if(NOT TARGET Libdrm::Libdrm)
        qt_find_package(Libdrm
            PROVIDED_TARGETS Libdrm::Libdrm
            MODULE_NAME gui
            QMAKE_LIB drm
            MARK_OPTIONAL)
    endif()
endif()

