# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(QmlAssetDownloaderPrivate
    URI "Qt.labs.assetdownloader"
    STATIC
    INTERNAL_MODULE
    SOURCES
        assetdownloader.cpp
        assetdownloader.h
        qqmlassetdownloader_p.h
        qqmlassetdownloader.cpp
    LIBRARIES
        Qt6::CorePrivate
    PUBLIC_LIBRARIES
        Qt6::Concurrent
        Qt6::Core
        Qt6::Network
        Qt6::Qml
        Qt6::TaskTree
)
