# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QConnManNetworkInformationPlugin
    OUTPUT_NAME qconnman
    CLASS_NAME QConnManNetworkInformationBackendFactory
    PLUGIN_TYPE networkinformation
    DEFAULT_IF FALSE
    SOURCES
        qconnmannetworkinformationbackend.h
        qconnmannetworkinformationbackend.cpp
        qconnmanservice.h
        qconnmanservice.cpp
    LIBRARIES
        Qt::DBus
        Qt::NetworkPrivate
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
        QT_NO_CAST_FROM_ASCII
)
