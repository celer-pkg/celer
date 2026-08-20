# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(LottieVectorImageHelpers
    URI "Qt.labs.lottieqt.VectorImageHelpers"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QLottieVectorImageHelpersPlugin
    SOURCES
        qquicklayeritem_p.h qquicklayeritem.cpp
    LIBRARIES
        Qt::QuickPrivate
)
