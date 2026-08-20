# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


qt_add_library(tst_qqmlimport_static_module STATIC)
qt_autogen_tools_initial_setup(tst_qqmlimport_static_module)

qt_add_qml_module(tst_qqmlimport_static_module
    URI MyStaticModule
    QML_FILES
        SomeComponent.qml
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    RESOURCE_PREFIX "/"
)

qt_autogen_tools_initial_setup(tst_qqmlimport_static_moduleplugin)
qt_internal_extend_target(tst_qqmlimport_static_moduleplugin
    SOURCES
        mystaticmoduleplugin.h
)
