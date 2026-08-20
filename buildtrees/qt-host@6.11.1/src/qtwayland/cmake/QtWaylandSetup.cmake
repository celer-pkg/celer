# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

function(qt_internal_get_wayland_protocols_dir out_var)
    if(QT_SUPERBUILD AND NOT QT_INTERNAL_BUILD_STANDALONE_PARTS)
        set(qtbase_wayland_protocols_dir
            "${QT_BUILD_DIR}/${INSTALL_SHAREDIR}/qt6/wayland/protocols/")
    else()
        set(qtbase_wayland_protocols_dir
            "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/${QT6_INSTALL_SHAREDIR}/qt6/wayland/protocols/")
    endif()
    set(${out_var} "${qtbase_wayland_protocols_dir}" PARENT_SCOPE)
endfunction()

function(qt_internal_get_wayland_extensions_dir out_var)
    if(QT_SUPERBUILD AND NOT QT_INTERNAL_BUILD_STANDALONE_PARTS)
        set(qtbase_wayland_extensions_dir
            "${QT_BUILD_DIR}/${INSTALL_SHAREDIR}/qt6/wayland/extensions/")
    else()
        set(qtbase_wayland_extensions_dir
            "${QT_BUILD_INTERNALS_RELOCATABLE_INSTALL_PREFIX}/${QT6_INSTALL_SHAREDIR}/qt6/wayland/extensions/")
    endif()
    set(${out_var} "${qtbase_wayland_extensions_dir}" PARENT_SCOPE)
endfunction()
