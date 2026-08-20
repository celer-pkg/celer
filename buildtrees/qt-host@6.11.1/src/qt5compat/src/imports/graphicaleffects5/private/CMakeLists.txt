# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qtgraphicaleffectsprivate Plugin:
#####################################################################

set(qml_files
    "DropShadowBase.qml"
    "FastGlow.qml"
    "FastInnerShadow.qml"
    "GaussianDirectionalBlur.qml"
    "GaussianGlow.qml"
    "GaussianInnerShadow.qml"
    "GaussianMaskedBlur.qml"
)

qt_internal_add_qml_module(qtgraphicaleffectsprivate
    URI "Qt5Compat.GraphicalEffects.private"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QtGraphicalEffectsPrivatePlugin
    PLUGIN_TARGET qtgraphicaleffectsprivate
    PAST_MAJOR_VERSIONS 1
    SOURCES
        qgfxsourceproxy.cpp qgfxsourceproxy_p.h
        qgfxshaderbuilder.cpp qgfxshaderbuilder_p.h
    QML_FILES
        ${qml_files}
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::QuickPrivate
        Qt::ShaderTools
        Qt::ShaderToolsPrivate
)
