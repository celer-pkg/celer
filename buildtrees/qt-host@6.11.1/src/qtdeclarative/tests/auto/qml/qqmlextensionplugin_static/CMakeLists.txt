# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(MockExtensionPlugin
    URI "MockExtension"
    CLASS_NAME MockExtensionPlugin
    PLUGIN_TARGET MockExtensionPlugin
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_PLUGIN_SOURCE
    STATIC
    SOURCES
        plugin/plugin.cpp
    LIBRARIES
        Qt::Qml
    TEST_PLUGIN
)


qt_internal_add_test(tst_qqmlextensionplugin_static
    SOURCES
        tst_qqmlextensionplugin_static.cpp
    LIBRARIES
        Qt::Qml
        Qt::MockExtensionPlugin
)
