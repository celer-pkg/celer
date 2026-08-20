# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QAppleNetworkInformationPlugin
    OUTPUT_NAME qapplenetworkinformation
    CLASS_NAME QAppleNetworkInformationBackendFactory
    PLUGIN_TYPE networkinformation
    DEFAULT_IF APPLE
    SOURCES
    qapplenetworkinformationbackend.mm
    LIBRARIES
        Qt::NetworkPrivate
        ${FWNetwork}
    DEFINES
        "QT_ENABLE_STRICT_MODE_UP_TO=QT_VERSION_CHECK(6,11,0)"
        QT_NO_CAST_FROM_ASCII
)
