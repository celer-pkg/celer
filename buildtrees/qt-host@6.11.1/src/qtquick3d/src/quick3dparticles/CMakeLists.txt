# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Quick3DParticles Module:
#####################################################################

include(../runtimerender/ParticleShaderGenMacros.cmake)

qt_internal_add_qml_module(Quick3DParticles
    URI "QtQuick3D.Particles3D"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    CLASS_NAME QtQuick3DParticles3DPlugin
    PLUGIN_TARGET qtquick3dparticles3dplugin
    DEPENDENCIES
        QtQuick3D/auto
    SOURCES
        qtquick3dparticlesglobal.h qtquick3dparticlesglobal_p.h
        qquick3dparticle.cpp qquick3dparticle_p.h
        qquick3dparticleaffector.cpp qquick3dparticleaffector_p.h
        qquick3dparticleattractor.cpp qquick3dparticleattractor_p.h
        qquick3dparticledata_p.h
        qquick3dparticledirection.cpp qquick3dparticledirection_p.h
        qquick3dparticleemitburst.cpp qquick3dparticleemitburst_p.h
        qquick3dparticleemitter.cpp qquick3dparticleemitter_p.h
        qquick3dparticlegravity.cpp qquick3dparticlegravity_p.h
        qquick3dparticlemodelparticle.cpp qquick3dparticlemodelparticle_p.h
        qquick3dparticlepointrotator.cpp qquick3dparticlepointrotator_p.h
        qquick3dparticlerandomizer_p.h
        qquick3dparticleabstractshape.cpp qquick3dparticleabstractshape_p.h
        qquick3dparticlecustomshape.cpp qquick3dparticlecustomshape_p.h
        qquick3dparticleshape.cpp qquick3dparticleshape_p.h
        qquick3dparticlemodelshape.cpp qquick3dparticlemodelshape_p.h
        qquick3dparticlerepeller.cpp qquick3dparticlerepeller_p.h
        qquick3dparticleshapedatautils.cpp qquick3dparticleshapedatautils_p.h
        qquick3dparticlespriteparticle.cpp qquick3dparticlespriteparticle_p.h
        qquick3dparticlesystem.cpp qquick3dparticlesystem_p.h
        qquick3dparticlesystemlogging.cpp qquick3dparticlesystemlogging_p.h
        qquick3dparticletargetdirection.cpp qquick3dparticletargetdirection_p.h
        qquick3dparticletrailemitter.cpp qquick3dparticletrailemitter_p.h
        qquick3dparticlevectordirection.cpp qquick3dparticlevectordirection_p.h
        qquick3dparticlewander.cpp qquick3dparticlewander_p.h
        qquick3dparticleutils.cpp qquick3dparticleutils_p.h
        qquick3dparticlespritesequence.cpp qquick3dparticlespritesequence_p.h
        qquick3dparticlescaleaffector.cpp qquick3dparticlescaleaffector_p.h
        qquick3dparticlemodelblendparticle_p.h qquick3dparticlemodelblendparticle.cpp
        qquick3dparticledynamicburst.cpp qquick3dparticledynamicburst_p.h
        qquick3dparticlelineparticle.cpp qquick3dparticlelineparticle_p.h
        qquick3dparticlesceneshape.cpp qquick3dparticlesceneshape_p.h
    DEFINES
        QT_BUILD_QUICK3DPARTICLES_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3DRuntimeRenderPrivate
        Qt::Quick3DAssetImportPrivate
        Qt::QuickPrivate
        Qt::Quick3DPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::Quick3DRuntimeRender
        Qt::Quick3DAssetImport
        Qt::Quick3D
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::QmlPrivate
        Qt::Quick3DRuntimeRenderPrivate
        Qt::Quick3DAssetImportPrivate
        Qt::QuickPrivate
        Qt::Quick3DPrivate
)

qt_internal_generate_tracepoints(Quick3DParticles quick3d
    SOURCES
        qquick3dparticlesystem.cpp
)

_qt_internal_gen_particle_shaders(Quick3DParticles "res_particle_shaders")

if(QT_FEATURE_quick_designer AND QT_BUILD_SHARED_LIBS) # special case handle unconverted static
    add_subdirectory(designer)
endif()
