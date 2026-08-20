# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from compositor-extensions.pro.

if(QT_FEATURE_wayland_compositor_quick)
    add_subdirectory(xdgshell)
    add_subdirectory(iviapplication)
    add_subdirectory(wlshell)
    add_subdirectory(qtshell)
    add_subdirectory(presentationtime)
endif()

