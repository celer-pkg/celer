# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(ProtobufQuick
    URI QtProtobuf
    VERSION ${CMAKE_PROJECT_VERSION}
    PLUGIN_TARGET protobufquickplugin
    CLASS_NAME ProtobufQuickPlugin
    SOURCES
        qqmlprotobuftypes_p.h
    LIBRARIES
        Qt::Core
        Qt::Protobuf
        Qt::Qml
    PUBLIC_LIBRARIES
        Qt::Quick
)
