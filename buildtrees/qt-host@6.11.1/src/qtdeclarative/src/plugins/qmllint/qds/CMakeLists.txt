# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QmlLintQdsPlugin
    OUTPUT_NAME qdslintplugin
    PLUGIN_TYPE qmllint
    SOURCES
        qdslintplugin.h
        qdslintplugin.cpp
    LIBRARIES
        Qt::QmlCompilerPrivate
)
