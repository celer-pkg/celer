# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QQuick3DProfilerAdapterFactory Plugin:
#####################################################################

qt_internal_add_plugin(QQuick3DProfilerAdapterFactoryPlugin
    OUTPUT_NAME qmldbg_quick3dprofiler
    CLASS_NAME QQuick3DProfilerAdapterFactory
    PLUGIN_TYPE qmltooling
    SOURCES
        qquick3dprofileradapter.cpp qquick3dprofileradapter.h
        qquick3dprofileradapterfactory.cpp qquick3dprofileradapterfactory.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::PacketProtocolPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::Quick3DUtils
        Qt::Quick3DUtilsPrivate
)

# OTHER_FILES = "qquick3dprofileradapter.json"
