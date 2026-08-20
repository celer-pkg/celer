# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QGlibNetworkInformationPlugin
    OUTPUT_NAME qglib
    CLASS_NAME QGlibNetworkInformationBackendFactory
    PLUGIN_TYPE networkinformation
    DEFAULT_IF LINUX
    SOURCES
        qglibnetworkinformationbackend.cpp
    LIBRARIES
        Qt::NetworkPrivate
        GLIB2::GOBJECT
        GLIB2::GIO
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
        QT_NO_CAST_FROM_ASCII
        QT_NO_SIGNALS_SLOTS_KEYWORDS
)
