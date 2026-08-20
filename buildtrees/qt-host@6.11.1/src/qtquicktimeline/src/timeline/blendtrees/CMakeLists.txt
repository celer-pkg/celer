# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(QuickTimelineBlendTrees
    URI "QtQuick.Timeline.BlendTrees"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    CLASS_NAME QtQuickTimelineBlendTreesPlugin
    PLUGIN_TARGET qtquicktimelineblendtreesplugin
    DEPENDENCIES
        QtQuick/auto
        QtQuick.Timeline/auto
    SOURCES
        qblendtreenode.cpp qblendtreenode_p.h
        qtimelineanimationnode.cpp qtimelineanimationnode_p.h
        qblendanimationnode.cpp qblendanimationnode_p.h
        qtquicktimelineblendtreesglobal.h qtquicktimelineblendtreesglobal_p.h
    DEFINES
        QT_BUILD_QUICKTIMELINEBLENDTREES_LIB
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::Quick
        Qt::QuickTimeline
    LIBRARIES
        Qt::QuickPrivate
        Qt::QuickTimelinePrivate
)
