# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QNetworkManagerNetworkInformationPlugin
    OUTPUT_NAME qnetworkmanager
    CLASS_NAME QNetworkManagerNetworkInformationBackendFactory
    PLUGIN_TYPE networkinformation
    DEFAULT_IF LINUX
    SOURCES
        qnetworkmanagernetworkinformationbackend.h
        qnetworkmanagernetworkinformationbackend.cpp
        qnetworkmanagerservice.h
        qnetworkmanagerservice.cpp
    LIBRARIES
        Qt::DBus
        Qt::NetworkPrivate
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
        QT_NO_CAST_FROM_ASCII
)
