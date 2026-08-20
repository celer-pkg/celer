# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from shellintegration.pro.

if(QT_FEATURE_wayland_client_ivi_shell)
    add_subdirectory(ivi-shell)
endif()
if(QT_FEATURE_wayland_client_qt_shell)
    add_subdirectory(qt-shell)
endif()
