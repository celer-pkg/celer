# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from quick3danimation.pro.

#####################################################################
## Qt::3DQuickAnimation Module:
#####################################################################

qt_internal_add_qml_module(3DQuickAnimation
    URI "Qt3D.Animation"
    VERSION "${PROJECT_VERSION}"
    PAST_MAJOR_VERSIONS 2
    CLASS_NAME Qt3DQuick3DAnimationPlugin
    DEPENDENCIES
        QtQml/auto
        Qt3D.Render/auto
        Qt3D.Core/auto
    NO_GENERATE_PLUGIN_SOURCE
    PLUGIN_TARGET quick3danimationplugin

    SOURCES
        items/quick3danimationcontroller.cpp items/quick3danimationcontroller_p.h
        items/quick3danimationgroup.cpp items/quick3danimationgroup_p.h
        items/quick3dchannelmapper.cpp items/quick3dchannelmapper_p.h
        items/quick3dkeyframeanimation.cpp items/quick3dkeyframeanimation_p.h
        items/quick3dmorphinganimation.cpp items/quick3dmorphinganimation_p.h
        items/quick3dmorphtarget.cpp items/quick3dmorphtarget_p.h
        items/quick3dvertexblendanimation.cpp items/quick3dvertexblendanimation_p.h
        qt3dquickanimation_global.cpp qt3dquickanimation_global.h qt3dquickanimation_global_p.h
        qt3dquickanimationnodefactory.cpp qt3dquickanimationnodefactory_p.h
        qt3dquickanimationforeign_p.h
    INCLUDE_DIRECTORIES
        items
    LIBRARIES
        Qt::3DCorePrivate
        Qt::3DQuickPrivate
        Qt::3DRenderPrivate
        Qt::CorePrivate
        Qt::QmlPrivate
    PUBLIC_LIBRARIES
        Qt::3DAnimation
        Qt::3DCore
        Qt::3DQuick
        Qt::3DRender
        Qt::Core
        Qt::Gui
        Qt::Qml
    PRIVATE_MODULE_INTERFACE
        Qt::3DCorePrivate
        Qt::3DQuickPrivate
        Qt::3DRenderPrivate
        Qt::CorePrivate
        Qt::QmlPrivate
)

#### Keys ignored in scope 1:.:.:quick3danimation.pro:<TRUE>:
# MODULE = "3dquickanimation"

## Scopes:
#####################################################################

qt_internal_extend_target(3DQuickAnimation CONDITION gcov
    COMPILE_OPTIONS
        -fprofile-arcs
        -ftest-coverage
    LINK_OPTIONS
        "-fprofile-arcs"
        "-ftest-coverage"
)

qt_internal_extend_target(quick3danimationplugin
    SOURCES
        qt3dquickanimationplugin.cpp qt3dquickanimationplugin_p.h
    LIBRARIES
        Qt::3DQuickAnimationPrivate
)
