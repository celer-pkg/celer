// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendererimplshaders_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include "../qssgrendercontextcore.h"
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershaderlibrarymanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterialshadergenerator_p.h>
#include <QtQuick3DRuntimeRender/private/qssgvertexpipelineimpl_p.h>

// this file contains the getXxxxShader implementations suitable for the QRhi-based rendering path

QT_BEGIN_NAMESPACE

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getBuiltinRhiShader(const QByteArray &name,
                                                                        BuiltinShader &storage,
                                                                        int viewCount,
                                                                        QSSGRhiShaderPipeline::StageFlags vertexStageFlags)
{
    if (storage.shaderPipeline && storage.viewCount != viewCount)
        storage = {};

    if (!storage.shaderPipeline) {
        // loadBuiltin must always return a valid QSSGRhiShaderPipeline.
        // There will just be no stages if loading fails.
        storage.shaderPipeline = m_shaderCache.loadBuiltinUncached(name, viewCount, vertexStageFlags);
        storage.viewCount = viewCount;
    }

    return storage.shaderPipeline;
}

void QSSGBuiltInRhiShaderCache::releaseCachedResources()
{
    m_cache = {};
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiGridShader(int viewCount)
{
    return getBuiltinRhiShader(QByteArrayLiteral("grid"), m_cache.gridShader, viewCount);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiSsaoShader(int viewCount)
{
    return getBuiltinRhiShader(QByteArrayLiteral("ssao"), m_cache.ssaoRhiShader, viewCount);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiSkyBoxCubeShader(QSSGRenderLayer::TonemapMode tonemapMode, bool isLinear, int viewCount)
{
    const bool rawPixels = (!isLinear && tonemapMode == QSSGRenderLayer::TonemapMode::Linear) || (isLinear && tonemapMode == QSSGRenderLayer::TonemapMode::None);
    auto shaderName = rawPixels ? QByteArrayLiteral("skyboxcube") : QByteArrayLiteral("skyboxcube_runtime_tonemapping");
    return getBuiltinRhiShader(shaderName, m_cache.skyBoxCubeRhiShader, viewCount);
}

static inline constexpr size_t getSkyboxIndex(QSSGRenderLayer::TonemapMode tonemapMode, bool isRGBE)
{
    switch (tonemapMode) {
    case QSSGRenderLayer::TonemapMode::None:
        return 0 + (size_t(isRGBE) * QSSGRenderLayer::TonemapModeCount);
    case QSSGRenderLayer::TonemapMode::Linear:
        return 1 + (size_t(isRGBE) * QSSGRenderLayer::TonemapModeCount);
    case QSSGRenderLayer::TonemapMode::Aces:
        return 2 + (size_t(isRGBE) * QSSGRenderLayer::TonemapModeCount);
    case QSSGRenderLayer::TonemapMode::HejlDawson:
        return 3 + (size_t(isRGBE) * QSSGRenderLayer::TonemapModeCount);
    case QSSGRenderLayer::TonemapMode::Filmic:
        return 4 + (size_t(isRGBE) * QSSGRenderLayer::TonemapModeCount);
    case QSSGRenderLayer::TonemapMode::Custom:
        return 5 + (size_t(isRGBE) * QSSGRenderLayer::TonemapModeCount);
    }

    // GCC 8.x does not treat __builtin_unreachable() as constexpr
#  if !defined(Q_CC_GNU_ONLY) || (Q_CC_GNU >= 900)
    // NOLINTNEXTLINE(qt-use-unreachable-return): Triggers on Clang, breaking GCC 8
    Q_UNREACHABLE();
#  endif
    return 0;
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiSkyBoxShader(QSSGRenderLayer::TonemapMode tonemapMode, bool isRGBE, int viewCount)
{
    // Skybox shader is special and has multiple possible shaders so we have to do
    // a bit of manual work here (mapping resolved in getSkyboxIndex()).
    static constexpr char variant[][23] { "skybox_hdr_none",
                                          "skybox_hdr_linear",
                                          "skybox_hdr_aces",
                                          "skybox_hdr_hejldawson",
                                          "skybox_hdr_filmic",
                                          "skybox_hdr_custom",
                                          "skybox_rgbe_none",
                                          "skybox_rgbe_linear",
                                          "skybox_rgbe_aces",
                                          "skybox_rgbe_hejldawson",
                                          "skybox_rgbe_filmic",
                                          "skybox_rgbe_custom",
    };

    const size_t skyboxIndex = getSkyboxIndex(tonemapMode, isRGBE);
    return getBuiltinRhiShader(QByteArray::fromRawData(variant[skyboxIndex], std::char_traits<char>::length(variant[skyboxIndex])), m_cache.skyBoxRhiShader[skyboxIndex], viewCount);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiSupersampleResolveShader(int viewCount)
{
    return getBuiltinRhiShader(QByteArrayLiteral("ssaaresolve"), m_cache.supersampleResolveRhiShader, viewCount);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiProgressiveAAShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("progressiveaa"), m_cache.progressiveAARhiShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiMotionVectorShader(bool skin, bool instance, bool morph)
{
    QByteArray shaderName = "motionvector";
    if (skin)
        shaderName += "_skin";
    if (instance)
        shaderName += "_instance";
    if (morph)
        shaderName += "_morph";

    int index = (int(skin) << 2) | (int(instance) << 1) | int(morph);
    return getBuiltinRhiShader(shaderName, m_cache.motionVectorRhiShader[index], 1, {});
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiTemporalAAShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("temporalaa"), m_cache.temporalAARhiShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiSimpleQuadShader(int viewCount, QSSGRenderLayer::TonemapMode tonemapMode)
{
    static constexpr char variant[][22] = {
        "simplequad",
        "simplequad_linear",
        "simplequad_aces",
        "simplequad_hejldawson",
        "simplequad_filmic",
    };

    if (tonemapMode == QSSGRenderLayer::TonemapMode::Custom)
        tonemapMode = QSSGRenderLayer::TonemapMode::None;

    const size_t index = qMin(size_t(tonemapMode), std::size(variant) - 1);
    return getBuiltinRhiShader(QByteArray(variant[index]), m_cache.simpleQuadRhiShader, viewCount);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiLightmapUVRasterizationShader(LightmapUVRasterizationShaderMode mode)
{
    switch (mode) {
    case LightmapUVRasterizationShaderMode::Uv:
        return getBuiltinRhiShader(QByteArrayLiteral("lightmapuvraster_uv"), m_cache.lightmapUVRasterShader_uv);
    case LightmapUVRasterizationShaderMode::UvTangent:
        return getBuiltinRhiShader(QByteArrayLiteral("lightmapuvraster_uv_tangent"), m_cache.lightmapUVRasterShader_uv_tangent);
    case LightmapUVRasterizationShaderMode::Default:
        return getBuiltinRhiShader(QByteArrayLiteral("lightmapuvraster"), m_cache.lightmapUVRasterShader);
    }

    Q_UNREACHABLE_RETURN(getBuiltinRhiShader(QByteArrayLiteral("lightmapuvraster"), m_cache.lightmapUVRasterShader));
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiLightmapDilateShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("lightmapdilate"), m_cache.lightmapDilateShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiDebugObjectShader(int viewCount)
{
    return getBuiltinRhiShader(QByteArrayLiteral("debugobject"), m_cache.debugObjectShader, viewCount);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiReflectionprobePreFilterShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("reflectionprobeprefilter"), m_cache.reflectionprobePreFilterShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhienvironmentmapPreFilterShader(bool isRGBE)
{
    static constexpr char variant[][29] { "environmentmapprefilter", "environmentmapprefilter_rgbe" };
    const quint8 idx = quint8(isRGBE);
    return getBuiltinRhiShader(QByteArray::fromRawData(variant[idx], std::char_traits<char>::length(variant[idx])), m_cache.environmentmapPreFilterShader[idx]);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiEnvironmentmapShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("environmentmap"), m_cache.environmentmapShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiClearMRTShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("clear_mrt"), m_cache.clearMRTShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiOitCompositeShader(QSSGRenderLayer::OITMethod method, bool multisample, bool use_buffers)
{
    if (method == QSSGRenderLayer::OITMethod::WeightedBlended){
        if (multisample)
            return getBuiltinRhiShader(QByteArrayLiteral("oitcomposite_weightedblended_ms"), m_cache.oitCompositeShader[1]);
        else
            return getBuiltinRhiShader(QByteArrayLiteral("oitcomposite_weightedblended"), m_cache.oitCompositeShader[0]);
    } else if (method == QSSGRenderLayer::OITMethod::LinkedList) {
        if (use_buffers) {
            if (multisample)
                return getBuiltinRhiShader(QByteArrayLiteral("oitcomposite_linkedlist_ms_buf"), m_cache.oitCompositeShader[5]);
            else
                return getBuiltinRhiShader(QByteArrayLiteral("oitcomposite_linkedlist_buf"), m_cache.oitCompositeShader[4]);
        } else {
        if (multisample)
            return getBuiltinRhiShader(QByteArrayLiteral("oitcomposite_linkedlist_ms"), m_cache.oitCompositeShader[3]);
        else
            return getBuiltinRhiShader(QByteArrayLiteral("oitcomposite_linkedlist"), m_cache.oitCompositeShader[2]);
        }
    }
    Q_UNREACHABLE_RETURN(getBuiltinRhiShader(QByteArrayLiteral("oitcomposite_weightedblended"), m_cache.oitCompositeShader[0]));
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiCubeMapToAtlasShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("cubeMapToHemisphere"), m_cache.cubeMapToAtlasShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiClearShadowMapShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("clearshadowmap"), m_cache.clearShadowMapShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiClearImageShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("clearimage"), m_cache.clearImageShader);
}

QSSGRhiShaderPipelinePtr QSSGBuiltInRhiShaderCache::getRhiClearBufferShader()
{
    return getBuiltinRhiShader(QByteArrayLiteral("clearbuffer"), m_cache.clearBufferShader);
}

QT_END_NAMESPACE
