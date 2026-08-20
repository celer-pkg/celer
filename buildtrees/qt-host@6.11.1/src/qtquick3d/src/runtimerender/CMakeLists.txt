# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from runtimerender.pro.

#####################################################################
## Quick3DRuntimeRender Module:
#####################################################################

# Handle the conditional finding of either system zlib or qt zlib.
if(NOT QT_FEATURE_system_zlib)
    find_package(Qt6 COMPONENTS ZlibPrivate)
elseif(NOT TARGET WrapZLIB::WrapZLIB)
    qt_find_package(WrapZLIB PROVIDED_TARGETS WrapZLIB::WrapZLIB)
endif()

qt_internal_add_module(Quick3DRuntimeRender
    SOURCES
        graphobjects/qssgrendercamera.cpp graphobjects/qssgrendercamera_p.h
        graphobjects/qssgrendercustommaterial_p.h
        graphobjects/qssgrenderdefaultmaterial.cpp graphobjects/qssgrenderdefaultmaterial_p.h
        graphobjects/qssgrendereffect.cpp graphobjects/qssgrendereffect_p.h
        graphobjects/qssgrendergeometry.cpp graphobjects/qssgrendergeometry_p.h
        graphobjects/qssgrendergraphobject.cpp graphobjects/qssgrendergraphobject_p.h graphobjects/qssgrendergraphobject.h
        graphobjects/qssgrenderimage.cpp graphobjects/qssgrenderimage_p.h
        graphobjects/qssgrenderinstancetable.cpp graphobjects/qssgrenderinstancetable_p.h
        graphobjects/qssgrenderitem2d.cpp graphobjects/qssgrenderitem2d_p.h
        graphobjects/qssgrenderjoint.cpp graphobjects/qssgrenderjoint_p.h
        graphobjects/qssgrenderlayer.cpp graphobjects/qssgrenderlayer_p.h
        graphobjects/qssgrenderlight.cpp graphobjects/qssgrenderlight_p.h
        graphobjects/qssgrendermodel.cpp graphobjects/qssgrendermodel_p.h
        graphobjects/qssgrendernode.cpp graphobjects/qssgrendernode_p.h
        graphobjects/qssgrenderskeleton.cpp graphobjects/qssgrenderskeleton_p.h
        graphobjects/qssgrenderskin.cpp graphobjects/qssgrenderskin_p.h
        graphobjects/qssgrendertexturedata.cpp graphobjects/qssgrendertexturedata_p.h
        graphobjects/qssgrendermorphtarget.cpp graphobjects/qssgrendermorphtarget_p.h
        graphobjects/qssgrenderresourceloader.cpp graphobjects/qssgrenderresourceloader_p.h
        graphobjects/qssgrenderreflectionprobe.cpp graphobjects/qssgrenderreflectionprobe_p.h
        graphobjects/qssgrenderroot_p.h graphobjects/qssgrenderroot.cpp
        graphobjects/qssgrenderuserpass_p.h graphobjects/qssgrenderuserpass.cpp
        qssgperframeallocator_p.h
        qssgrenderableimage_p.h
        qssgrenderclippingfrustum.cpp qssgrenderclippingfrustum_p.h
        graphobjects/qssgrenderparticles.cpp graphobjects/qssgrenderparticles_p.h
        qssgrendercommands.cpp qssgrendercommands_p.h
        qssgrendercontextcore.cpp qssgrendercontextcore.h
        qssgrenderdefaultmaterialshadergenerator.cpp qssgrenderdefaultmaterialshadergenerator_p.h
        qssgrendererutil_p.h
        qssgrenderimagetexture_p.h
        qssgrendermesh_p.h
        qssgrenderray.cpp qssgrenderray_p.h
        qssgrendershadercache.cpp qssgrendershadercache_p.h
        qssgrendershadercodegenerator.cpp qssgrendershadercodegenerator_p.h
        qssgrendershaderkeys_p.h
        qssgrendershadermetadata.cpp qssgrendershadermetadata_p.h
        qssgrendershadowmap.cpp qssgrendershadowmap_p.h
        qssgrenderreflectionmap.cpp qssgrenderreflectionmap_p.h
        qssgrendermotionvectormap.cpp qssgrendermotionvectormap_p.h
        qssgrenderpickresult_p.h qssgrenderpickresult.h
        qssgrhiparticles.cpp qssgrhiparticles_p.h
        qssgrhicontext.cpp qssgrhicontext_p.h qssgrhicontext.h
        qssgrhicustommaterialsystem.cpp qssgrhicustommaterialsystem_p.h
        qssgrhieffectsystem.cpp qssgrhieffectsystem_p.h
        qssgrhiquadrenderer.cpp qssgrhiquadrenderer_p.h
        qssgruntimerenderlogging.cpp qssgruntimerenderlogging_p.h
        qssgshadermapkey_p.h
        qssgshadermaterialadapter.cpp qssgshadermaterialadapter_p.h
        qssgshaderresourcemergecontext_p.h
        qssglightmapio.cpp qssglightmapio_p.h
        qtquick3druntimerenderglobal_p.h
        qtquick3druntimerenderglobal.h
        rendererimpl/qssgrenderableobjects.cpp rendererimpl/qssgrenderableobjects_p.h
        rendererimpl/qssgrenderer.cpp rendererimpl/qssgrenderer_p.h
        rendererimpl/qssglayerrenderdata_p.h
        rendererimpl/qssglayerrenderdata.cpp
        rendererimpl/qssglightmapbaker_p.h rendererimpl/qssglightmapbaker.cpp
        rendererimpl/qssglightmapper.cpp rendererimpl/qssglightmapper_p.h rendererimpl/qssglightmapper.h
        rendererimpl/qssgrendererimplshaders_p.h rendererimpl/qssgrendererimplshaders_rhi.cpp
        rendererimpl/qssgvertexpipelineimpl.cpp rendererimpl/qssgvertexpipelineimpl_p.h
        rendererimpl/qssgrenderpass_p.h rendererimpl/qssgrenderpass.cpp
        rendererimpl/qssgrenderhelpers_p.h rendererimpl/qssgrenderhelpers.cpp
        rendererimpl/qssgshadowmaphelpers_p.h rendererimpl/qssgshadowmaphelpers.cpp
        rendererimpl/qssgrenderdata_p.h rendererimpl/qssgrenderdata.cpp
        resourcemanager/qssgrenderbuffermanager.cpp resourcemanager/qssgrenderbuffermanager_p.h
        resourcemanager/qssgrenderloadedtexture.cpp resourcemanager/qssgrenderloadedtexture_p.h
        resourcemanager/qssgrendershaderlibrarymanager.cpp resourcemanager/qssgrendershaderlibrarymanager_p.h
        rendererimpl/qssgcputonemapper_p.h
        extensionapi/qssgrenderextensions.h extensionapi/qssgrenderextensions.cpp
        extensionapi/qssgrenderhelpers.h extensionapi/qssgrenderhelpers.cpp
        qssgdebugdrawsystem_p.h qssgdebugdrawsystem.cpp
        qssguserrenderpassmanager.cpp qssguserrenderpassmanager_p.h
        qssgrenderparticleshaderkeys_p.h
    NO_UNITY_BUILD_SOURCES
        rendererimpl/qssglightmapper.cpp              # avoiding possible clash with tinyexr' macros
        resourcemanager/qssgrenderbuffermanager.cpp # redefinition of 'cube' (from qssgrenderreflectionmap.cpp)
        resourcemanager/qssgrenderloadedtexture.cpp   # avoiding possible clash with tinyexr' macros
    DEFINES
        QT_BUILD_QUICK3DRUNTIMERENDER_LIB
    LIBRARIES
        Qt::Quick3DUtilsPrivate
        Qt::QuickPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::Quick3DUtils
    PRIVATE_MODULE_INTERFACE
        Qt::Quick3DUtilsPrivate
        Qt::QuickPrivate
    SSG_HEADER_FILTERS
        "(^|/)qssg.*[^_][^p]\\.h$"
)

if (MACOS)
    target_compile_definitions(Quick3DRuntimeRender PRIVATE QSSG_OIT_USE_BUFFERS)
endif()

qt_internal_extend_target(Quick3DRuntimeRender CONDITION TARGET Qt::ShaderTools
    PUBLIC_LIBRARIES
        Qt::ShaderTools
    LIBRARIES
        Qt::ShaderToolsPrivate
    DEFINES
        QT_QUICK3D_HAS_RUNTIME_SHADERS
)

if(QT_QUICK3D_HAS_EMBREE)
    find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Concurrent)
endif()

qt_internal_extend_target(Quick3DRuntimeRender CONDITION QT_QUICK3D_HAS_EMBREE
    PUBLIC_LIBRARIES
        Qt::Concurrent
    LIBRARIES
        Qt::BundledEmbree
    DEFINES
        QT_QUICK3D_HAS_LIGHTMAPPER
)

# Resources:
set(res_resource_files
    "res/effectlib/bsdf.glsllib"
    "res/effectlib/defaultMaterialBumpNoLod.glsllib"
    "res/effectlib/depthpass.glsllib"
    "res/effectlib/funccalculatePointLightAttenuation.glsllib"
    "res/effectlib/funcdebugShadowOcclusion.glsllib"
    "res/effectlib/funcdiffuseBurleyBSDF.glsllib"
    "res/effectlib/funcdiffuseLambertBRDF.glsllib"
    "res/effectlib/funcdiffuseLambertWrapBRDF.glsllib"
    "res/effectlib/funcgetTransformedUVCoords.glsllib"
    "res/effectlib/funclightmap.glsllib"
    "res/effectlib/funcsampleNormalTexture.glsllib"
    "res/effectlib/funcspecularBlinnPhong.glsllib"
    "res/effectlib/funcspecularGGXBSDF.glsllib"
    "res/effectlib/principledMaterialFresnel.glsllib"
    "res/effectlib/sampleProbe.glsllib"
    "res/effectlib/sampleReflectionProbe.glsllib"
    "res/effectlib/shadowMapping.glsllib"
    "res/effectlib/skinanim.glsllib"
    "res/effectlib/ssao.glsllib"
    "res/effectlib/tonemapping.glsllib"
    "res/effectlib/transmission.glsllib"
    "res/effectlib/morphanim.glsllib"
    "res/effectlib/morphanim_custom.glsllib"
    "res/effectlib/parallaxMapping.glsllib"
    "res/effectlib/particles.glsllib"
    "res/effectlib/fog.glsllib"
    "res/effectlib/texturesample.glsllib"
    "res/effectlib/oitcommon.glsllib"
    "res/effectlib/oitweightedblended.glsllib"
    "res/effectlib/oitlinkedlist.glsllib"
    "res/effectlib/oitlinkedlist_buf.glsllib"
    "res/effectlib/lightsData.glsllib"
    "res/effectlib/funcprocessPunctualLighting.glsllib"
    "res/effectlib/custommaterialforwarddeclarations.glsllib"
    "res/effectlib/particleLighting.glsllib"
    "res/effectlib/particleLoading.glsllib"
    "res/effectlib/particleSprite.glsllib"
    "res/effectlib/sorting.glsllib"
    "res/primitives/Cone.mesh"
    "res/primitives/Cube.mesh"
    "res/primitives/Cylinder.mesh"
    "res/primitives/Rectangle.mesh"
    "res/primitives/Sphere.mesh"
    "res/textures/blue_noise.png"
)

qt_internal_add_resource(Quick3DRuntimeRender "res"
    PREFIX
        "/"
    FILES
        ${res_resource_files}
)

# special case begin

qt_internal_extend_target(Quick3DRuntimeRender
    INCLUDE_DIRECTORIES
        ../3rdparty/tinyexr
    NO_UNITY_BUILD_SOURCES
        ../3rdparty/tinyexr/tinyexr.h
    ATTRIBUTION_FILE_DIR_PATHS
        ../3rdparty/tinyexr
)

qt_internal_extend_target(Quick3DRuntimeRender CONDITION QT_FEATURE_system_zlib
    LIBRARIES
        WrapZLIB::WrapZLIB
)

qt_internal_extend_target(Quick3DRuntimeRender CONDITION NOT QT_FEATURE_system_zlib
    LIBRARIES
        Qt6::ZlibPrivate
)

qt_internal_generate_tracepoints(Quick3DRuntimeRender quick3d
    SOURCES
        qssgrhicontext.cpp
        resourcemanager/qssgrenderbuffermanager.cpp
        qssgrendershadercache.cpp
        qssgrhicustommaterialsystem.cpp
)

# default settings are GLSL "100es,120,150" HLSL "50" MSL "12"
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PREFIX
        "/"
    FILES
        res/rhishaders/environmentmap.vert
        res/rhishaders/environmentmap.frag
        res/rhishaders/debugobject.vert
        res/rhishaders/debugobject.frag
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_pertarget"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    PREFIX
        "/"
    FILES
        res/rhishaders/ssaaresolve.vert
        res/rhishaders/ssaaresolve.frag
        res/rhishaders/progressiveaa.vert
        res/rhishaders/progressiveaa.frag
        res/rhishaders/temporalaa.vert
        res/rhishaders/temporalaa.frag
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_simplequad_none"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    PREFIX
        "/"
    FILES
        res/rhishaders/simplequad.vert
        res/rhishaders/simplequad.frag
    OUTPUTS
        res/rhishaders/simplequad.vert.qsb
        res/rhishaders/simplequad.frag.qsb
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_simplequad_linear"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    PREFIX
        "/"
    FILES
        res/rhishaders/simplequad.vert
        res/rhishaders/simplequad.frag
    OUTPUTS
        res/rhishaders/simplequad_linear.vert.qsb
        res/rhishaders/simplequad_linear.frag.qsb
    DEFINES
        QSSG_ENABLE_LINEAR_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_simplequad_aces"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    PREFIX
        "/"
    FILES
        res/rhishaders/simplequad.vert
        res/rhishaders/simplequad.frag
    OUTPUTS
        res/rhishaders/simplequad_aces.vert.qsb
        res/rhishaders/simplequad_aces.frag.qsb
    DEFINES
        QSSG_ENABLE_ACES_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_simplequad_hejldawson"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    PREFIX
        "/"
    FILES
        res/rhishaders/simplequad.vert
        res/rhishaders/simplequad.frag
    OUTPUTS
        res/rhishaders/simplequad_hejldawson.vert.qsb
        res/rhishaders/simplequad_hejldawson.frag.qsb
    DEFINES
        QSSG_ENABLE_HEJLDAWSON_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_simplequad_filmic"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    PREFIX
        "/"
    FILES
        res/rhishaders/simplequad.vert
        res/rhishaders/simplequad.frag
    OUTPUTS
        res/rhishaders/simplequad_filmic.vert.qsb
        res/rhishaders/simplequad_filmic.frag.qsb
    DEFINES
        QSSG_ENABLE_FILMIC_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_es3_pertarget"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    GLSL "300es,120,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/lightmapdilate.vert
        res/rhishaders/lightmapdilate.frag
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_clear_mrt"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/clearmrt.vert
        res/rhishaders/clearmrt.frag
    OUTPUTS
        res/rhishaders/clear_mrt.vert.qsb
        res/rhishaders/clear_mrt.frag.qsb
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_es3_gl3"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/ssao.vert
        res/rhishaders/ssao.frag
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
        res/rhishaders/environmentmapprefilter.vert
        res/rhishaders/environmentmapprefilter.frag
        res/rhishaders/reflectionprobeprefilter.vert
        res/rhishaders/reflectionprobeprefilter.frag
        res/rhishaders/skyboxcube.vert
        res/rhishaders/skyboxcube.frag
        res/rhishaders/grid.frag
        res/rhishaders/grid.vert
        res/rhishaders/cubeMapToHemisphere.vert
        res/rhishaders/cubeMapToHemisphere.frag
        res/rhishaders/clearshadowmap.vert
        res/rhishaders/clearshadowmap.frag
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_compute"
    SILENT
    PRECOMPILE
    OPTIMIZED
    GLSL "310es,430"
    PREFIX
        "/"
    FILES
        res/rhishaders/nlm_denoise.comp
    OUTPUTS
        res/rhishaders/nlm_denoise.comp.qsb

)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_lightprobe_rgbe"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/environmentmapprefilter.vert
        res/rhishaders/environmentmapprefilter.frag
    OUTPUTS
        res/rhishaders/environmentmapprefilter_rgbe.vert.qsb
        res/rhishaders/environmentmapprefilter_rgbe.frag.qsb
    DEFINES
        QSSG_ENABLE_RGBE_LIGHT_PROBE=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_hdr_none"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_hdr_none.vert.qsb
        res/rhishaders/skybox_hdr_none.frag.qsb
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_rgbe_none"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_rgbe_none.vert.qsb
        res/rhishaders/skybox_rgbe_none.frag.qsb
    DEFINES
        QSSG_ENABLE_RGBE_LIGHT_PROBE=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_hdr_custom"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_hdr_custom.vert.qsb
        res/rhishaders/skybox_hdr_custom.frag.qsb
    DEFINES
        QSSG_FORCE_IBL_EXPOSURE=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_rgbe_custom"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_rgbe_custom.vert.qsb
        res/rhishaders/skybox_rgbe_custom.frag.qsb
    DEFINES
        QSSG_ENABLE_RGBE_LIGHT_PROBE=1
        QSSG_FORCE_IBL_EXPOSURE=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_hdr_linear"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_hdr_linear.vert.qsb
        res/rhishaders/skybox_hdr_linear.frag.qsb
    DEFINES
        QSSG_ENABLE_LINEAR_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_rgbe_linear"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_rgbe_linear.vert.qsb
        res/rhishaders/skybox_rgbe_linear.frag.qsb
    DEFINES
        QSSG_ENABLE_LINEAR_TONEMAPPING=1
        QSSG_ENABLE_RGBE_LIGHT_PROBE=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_hdr_aces"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_hdr_aces.vert.qsb
        res/rhishaders/skybox_hdr_aces.frag.qsb
    DEFINES
        QSSG_ENABLE_ACES_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_rgbe_aces"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_rgbe_aces.vert.qsb
        res/rhishaders/skybox_rgbe_aces.frag.qsb
    DEFINES
        QSSG_ENABLE_ACES_TONEMAPPING=1
        QSSG_ENABLE_RGBE_LIGHT_PROBE=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_hdr_hejldawson"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_hdr_hejldawson.vert.qsb
        res/rhishaders/skybox_hdr_hejldawson.frag.qsb
    DEFINES
        QSSG_ENABLE_HEJLDAWSON_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_rgbe_hejldawson"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_rgbe_hejldawson.vert.qsb
        res/rhishaders/skybox_rgbe_hejldawson.frag.qsb
    DEFINES
        QSSG_ENABLE_HEJLDAWSON_TONEMAPPING=1
        QSSG_ENABLE_RGBE_LIGHT_PROBE=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_hdr_filmic"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_hdr_filmic.vert.qsb
        res/rhishaders/skybox_hdr_filmic.frag.qsb
    DEFINES
        QSSG_ENABLE_FILMIC_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skybox_rgbe_filmic"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skybox.vert
        res/rhishaders/skybox.frag
    OUTPUTS
        res/rhishaders/skybox_rgbe_filmic.vert.qsb
        res/rhishaders/skybox_rgbe_filmic.frag.qsb
    DEFINES
        QSSG_ENABLE_FILMIC_TONEMAPPING=1
        QSSG_ENABLE_RGBE_LIGHT_PROBE=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector.vert.qsb
        res/rhishaders/motionvector.frag.qsb
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector_morph"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector_morph.vert.qsb
        res/rhishaders/motionvector_morph.frag.qsb
    DEFINES
        QSSG_MORPH=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector_instance"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector_instance.vert.qsb
        res/rhishaders/motionvector_instance.frag.qsb
    DEFINES
        QSSG_INSTANCE=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector_instance_morph"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector_instance_morph.vert.qsb
        res/rhishaders/motionvector_instance_morph.frag.qsb
    DEFINES
        QSSG_INSTANCE=1
        QSSG_MORPH=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector_skin"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector_skin.vert.qsb
        res/rhishaders/motionvector_skin.frag.qsb
    DEFINES
        QSSG_SKIN=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector_skin_morph"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector_skin_morph.vert.qsb
        res/rhishaders/motionvector_skin_morph.frag.qsb
    DEFINES
        QSSG_SKIN=1
        QSSG_MORPH=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector_skin_instance"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector_skin_instance.vert.qsb
        res/rhishaders/motionvector_skin_instance.frag.qsb
    DEFINES
        QSSG_SKIN=1
        QSSG_INSTANCE=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_motionvector_skin_instance_morph"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/motionvector.vert
        res/rhishaders/motionvector.frag
    OUTPUTS
        res/rhishaders/motionvector_skin_instance_morph.vert.qsb
        res/rhishaders/motionvector_skin_instance_morph.frag.qsb
    DEFINES
        QSSG_SKIN=1
        QSSG_INSTANCE=1
        QSSG_MORPH=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_skyboxcube_runtime_tonemapping"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/skyboxcube.vert
        res/rhishaders/skyboxcube.frag
    OUTPUTS
        res/rhishaders/skyboxcube_runtime_tonemapping.vert.qsb
        res/rhishaders/skyboxcube_runtime_tonemapping.frag.qsb
    DEFINES
        QSSG_RUNTIME_TONEMAPPING=1
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_lightmapuvraster_default"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,120,330"
    PREFIX
        "/"
    FILES
        res/rhishaders/lightmapuvraster.vert
        res/rhishaders/lightmapuvraster.frag
    OUTPUTS
        res/rhishaders/lightmapuvraster.vert.qsb
        res/rhishaders/lightmapuvraster.frag.qsb
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_lightmapuvraster_uv"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,120,330"
    PREFIX
        "/"
    FILES
        res/rhishaders/lightmapuvraster.vert
        res/rhishaders/lightmapuvraster.frag
    OUTPUTS
        res/rhishaders/lightmapuvraster_uv.vert.qsb
        res/rhishaders/lightmapuvraster_uv.frag.qsb
    DEFINES
        QSSG_LIGHTMAPUVRASTER_UV
)
qt_internal_add_shaders(Quick3DRuntimeRender "res_lightmapuvraster_uv_tangent"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "300es,120,330"
    PREFIX
        "/"
    FILES
        res/rhishaders/lightmapuvraster.vert
        res/rhishaders/lightmapuvraster.frag
    OUTPUTS
        res/rhishaders/lightmapuvraster_uv_tangent.vert.qsb
        res/rhishaders/lightmapuvraster_uv_tangent.frag.qsb
    DEFINES
        QSSG_LIGHTMAPUVRASTER_UV_TANGENT
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_clearimage"
    SILENT
    PRECOMPILE
    OPTIMIZED
    GLSL "320es,430"
    PREFIX
        "/"
    FILES
        res/rhishaders/clearimage.vert
        res/rhishaders/clearimage.frag
    OUTPUTS
        res/rhishaders/clearimage.vert.qsb
        res/rhishaders/clearimage.frag.qsb
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_clearbuffer"
    SILENT
    PRECOMPILE
    OPTIMIZED
    GLSL "320es,430"
    PREFIX
        "/"
    FILES
        res/rhishaders/clearimage.vert
        res/rhishaders/clearimage.frag
    OUTPUTS
        res/rhishaders/clearbuffer.vert.qsb
        res/rhishaders/clearbuffer.frag.qsb
    DEFINES
        QSSG_CLEAR_BUFFER=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_composite_weightedblended"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/oitcomposite.vert
        res/rhishaders/oitcomposite.frag
    OUTPUTS
        res/rhishaders/oitcomposite_weightedblended.vert.qsb
        res/rhishaders/oitcomposite_weightedblended.frag.qsb
    DEFINES
        QSSG_OIT_METHOD=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_composite_weightedblended_ms"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    PERTARGETCOMPILE
    GLSL "300es,150"
    PREFIX
        "/"
    FILES
        res/rhishaders/oitcomposite.vert
        res/rhishaders/oitcomposite.frag
    OUTPUTS
        res/rhishaders/oitcomposite_weightedblended_ms.vert.qsb
        res/rhishaders/oitcomposite_weightedblended_ms.frag.qsb
    DEFINES
        QSSG_OIT_METHOD=1
        QSSG_MULTISAMPLE
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_composite_linkedlist"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "320es,430"
    PREFIX
        "/"
    FILES
        res/rhishaders/oitcomposite.vert
        res/rhishaders/oitcomposite.frag
    OUTPUTS
        res/rhishaders/oitcomposite_linkedlist.vert.qsb
        res/rhishaders/oitcomposite_linkedlist.frag.qsb
    DEFINES
        QSSG_OIT_METHOD=2
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_composite_linkedlist_ms"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "320es,430"
    PREFIX
        "/"
    FILES
        res/rhishaders/oitcomposite.vert
        res/rhishaders/oitcomposite.frag
    OUTPUTS
        res/rhishaders/oitcomposite_linkedlist_ms.vert.qsb
        res/rhishaders/oitcomposite_linkedlist_ms.frag.qsb
    DEFINES
        QSSG_OIT_METHOD=2
        QSSG_MULTISAMPLE
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_composite_linkedlist_buf"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "320es,430"
    PREFIX
        "/"
    FILES
        res/rhishaders/oitcomposite.vert
        res/rhishaders/oitcomposite.frag
    OUTPUTS
        res/rhishaders/oitcomposite_linkedlist_buf.vert.qsb
        res/rhishaders/oitcomposite_linkedlist_buf.frag.qsb
    DEFINES
        QSSG_OIT_METHOD=2
        QSSG_USE_BUFFER=1
)

qt_internal_add_shaders(Quick3DRuntimeRender "res_shaders_composite_linkedlist_ms_buf"
    SILENT
    PRECOMPILE
    OPTIMIZED
    MULTIVIEW
    GLSL "320es,430"
    PREFIX
        "/"
    FILES
        res/rhishaders/oitcomposite.vert
        res/rhishaders/oitcomposite.frag
    OUTPUTS
        res/rhishaders/oitcomposite_linkedlist_ms_buf.vert.qsb
        res/rhishaders/oitcomposite_linkedlist_ms_buf.frag.qsb
    DEFINES
        QSSG_OIT_METHOD=2
        QSSG_MULTISAMPLE
        QSSG_USE_BUFFER=1
)

# special case end

#### Keys ignored in scope 1:.:.:runtimerender.pro:<TRUE>:
# MODULE = "quick3druntimerender"
