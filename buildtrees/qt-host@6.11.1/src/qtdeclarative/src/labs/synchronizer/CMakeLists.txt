# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(LabsSynchronizer
    URI "Qt.labs.synchronizer"
    VERSION "${PROJECT_VERSION}"
    DEPENDENCIES
        QtQml/auto
    SOURCES
        qqmlsynchronizer.cpp qqmlsynchronizer_p.h
    PUBLIC_LIBRARIES
        Qt::Core
    LIBRARIES
        Qt::QmlPrivate
    PRIVATE_MODULE_INTERFACE
        Qt::QmlPrivate
)
