# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Quick3DParticleEffects Module:
#####################################################################

qt_internal_add_qml_module(Quick3DParticleEffects
    URI "QtQuick3D.ParticleEffects"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    CLASS_NAME QtQuick3DParticleEffectsPlugin
    PLUGIN_TARGET qtquick3dparticleeffectsplugin
    DEPENDENCIES
        QtQuick3D/auto
        QtQuick3D.Particles3D/auto
    NO_SYNC_QT
    NO_PRIVATE_MODULE
    DEFINES
        QT_BUILD_QUICK3DPARTICLEEFFECTS_LIB
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::Quick3DRuntimeRender
        Qt::Quick3DAssetImport
        Qt::Quick3D
        Qt::Quick3DParticles
)

set_target_properties(Quick3DParticleEffects PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)

if(QT_FEATURE_quick_designer AND QT_BUILD_SHARED_LIBS) # special case handle unconverted static
    add_subdirectory(designer)
endif()
