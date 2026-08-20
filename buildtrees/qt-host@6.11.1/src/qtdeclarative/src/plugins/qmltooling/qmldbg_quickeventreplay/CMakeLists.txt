# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QQuickEventReplayFactory Plugin:
#####################################################################

qt_internal_add_plugin(QQuickEventReplayServiceFactoryPlugin
    OUTPUT_NAME qmldbg_quickeventreplay
    CLASS_NAME QQuickEventReplayServiceFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qquickeventreplayservice.cpp qquickeventreplayservice.h
        qquickeventreplayservicefactory.cpp qquickeventreplayservicefactory.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::PacketProtocolPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
)
