// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERERIMPLSHADERS_P_H
#define QSSGRENDERERIMPLSHADERS_P_H

#include <QtCore/qbytearray.h>

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderparticles_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QSSGShaderCache;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGBuiltInRhiShaderCache
{
public:
    explicit QSSGBuiltInRhiShaderCache(QSSGShaderCache &shaderCache)
        : m_shaderCache(shaderCache) {}

    void releaseCachedResources();

    enum class LightmapUVRasterizationShaderMode {
        Default,
        Uv,
        UvTangent
    };

    QSSGRhiShaderPipelinePtr getRhiGridShader(int viewCount);
    QSSGRhiShaderPipelinePtr getRhiSsaoShader(int viewCount);
    QSSGRhiShaderPipelinePtr getRhiSkyBoxCubeShader(QSSGRenderLayer::TonemapMode tonemapMode, bool isLinear, int viewCount);
    QSSGRhiShaderPipelinePtr getRhiSkyBoxShader(QSSGRenderLayer::TonemapMode tonemapMode, bool isRGBE, int viewCount);
    QSSGRhiShaderPipelinePtr getRhiSupersampleResolveShader(int viewCount);
    QSSGRhiShaderPipelinePtr getRhiProgressiveAAShader();
    QSSGRhiShaderPipelinePtr getRhiMotionVectorShader(bool skin, bool instance, bool morph);
    QSSGRhiShaderPipelinePtr getRhiTemporalAAShader();
    QSSGRhiShaderPipelinePtr getRhiSimpleQuadShader(int viewCount, QSSGRenderLayer::TonemapMode tonemapMode = QSSGRenderLayer::TonemapMode::None);
    QSSGRhiShaderPipelinePtr getRhiLightmapUVRasterizationShader(LightmapUVRasterizationShaderMode mode);
    QSSGRhiShaderPipelinePtr getRhiLightmapDilateShader();
    QSSGRhiShaderPipelinePtr getRhiDebugObjectShader(int viewCount);
    QSSGRhiShaderPipelinePtr getRhiReflectionprobePreFilterShader();
    QSSGRhiShaderPipelinePtr getRhienvironmentmapPreFilterShader(bool isRGBE);
    QSSGRhiShaderPipelinePtr getRhiEnvironmentmapShader();
    QSSGRhiShaderPipelinePtr getRhiClearMRTShader();
    QSSGRhiShaderPipelinePtr getRhiOitCompositeShader(QSSGRenderLayer::OITMethod method, bool multisample, bool use_buffers = false);
    QSSGRhiShaderPipelinePtr getRhiCubeMapToAtlasShader();
    QSSGRhiShaderPipelinePtr getRhiClearShadowMapShader();
    QSSGRhiShaderPipelinePtr getRhiClearImageShader();
    QSSGRhiShaderPipelinePtr getRhiClearBufferShader();

private:
    QSSGShaderCache &m_shaderCache; // We're owned by the shadercache

    struct BuiltinShader {
        // The shader refs are non-null if we have attempted to generate the
        // shader. This does not mean we were successul, however.
        QSSGRhiShaderPipelinePtr shaderPipeline;
        int viewCount = 1;
    };

    QSSGRhiShaderPipelinePtr getBuiltinRhiShader(const QByteArray &name,
                                                 BuiltinShader &storage,
                                                 int viewCount = 1,
                                                 QSSGRhiShaderPipeline::StageFlags vertexStageFlags = QSSGRhiShaderPipeline::UsedWithoutIa);
    static constexpr int motionvectorShaderCount = 8; // (int(skin) << 2) | (int(instance) << 1) | int(morph);
    static constexpr int particleShaderCount = 2;
    static constexpr int compositeShaderCount = 6;
    struct {
        BuiltinShader gridShader;
        BuiltinShader ssaoRhiShader;
        BuiltinShader skyBoxRhiShader[QSSGRenderLayer::TonemapModeCount * 2 /* rgbe+hdr */];
        BuiltinShader skyBoxCubeRhiShader;
        BuiltinShader supersampleResolveRhiShader;
        BuiltinShader progressiveAARhiShader;
        BuiltinShader texturedQuadRhiShader;
        BuiltinShader simpleQuadRhiShader;
        BuiltinShader lightmapUVRasterShader;
        BuiltinShader lightmapUVRasterShader_uv;
        BuiltinShader lightmapUVRasterShader_uv_tangent;
        BuiltinShader lightmapDilateShader;
        BuiltinShader debugObjectShader;

        BuiltinShader reflectionprobePreFilterShader;
        BuiltinShader environmentmapPreFilterShader[2];
        BuiltinShader environmentmapShader;

        BuiltinShader clearMRTShader;
        BuiltinShader oitCompositeShader[compositeShaderCount];
        BuiltinShader cubeMapToAtlasShader;
        BuiltinShader clearShadowMapShader;
        BuiltinShader clearImageShader;
        BuiltinShader clearBufferShader;
        BuiltinShader motionVectorRhiShader[motionvectorShaderCount];
        BuiltinShader temporalAARhiShader;
    } m_cache;
};

QT_END_NAMESPACE

#endif // QSSGRENDERERIMPLSHADERS_P_H
