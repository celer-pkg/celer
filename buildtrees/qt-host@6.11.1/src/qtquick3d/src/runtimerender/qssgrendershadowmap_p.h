// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_SHADOW_MAP_H
#define QSSG_RENDER_SHADOW_MAP_H

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

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>
#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableobjects_p.h>

QT_BEGIN_NAMESPACE

class QSSGRhiContext;
class QSSGRenderContextInterface;

class QRhiRenderBuffer;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiTexture;

struct QSSGShadowMapEntry
{
    QSSGShadowMapEntry();

    static QSSGShadowMapEntry withAtlas(quint32 lightIdx);

    void destroyRhiResources();

    quint32 m_lightIndex; ///< the light index it belongs to
    quint32 m_depthArrayIndex = 0; ///< shadow map texture array index

    // Owned RHI Resoruces (just for point lights)
    QRhiTexture *m_rhiDepthCube = nullptr; // shadow cube map
    QRhiRenderBuffer *m_rhiDepthStencilCube = nullptr;
    QRhiRenderPassDescriptor *m_rhiRenderPassDescCube = nullptr;
    std::array<QRhiTextureRenderTarget *, 6> m_rhiRenderTargetCube = {};
    QRhiShaderResourceBindings *m_cubeToAtlasFrontSrb = nullptr;
    QRhiShaderResourceBindings *m_cubeToAtlasBackSrb = nullptr;

    // Shared RHI Resources
    std::array<QRhiTextureRenderTarget *, 4> m_rhiRenderTargets = {};
    std::array<QRhiRenderPassDescriptor *, 4> m_rhiRenderPassDesc = {};

    QMatrix4x4 m_lightViewProjection[4]; ///< light view projection matrix
    QMatrix4x4 m_lightCubeView[6]; ///< light cubemap view matrices
    QMatrix4x4 m_lightView; ///< light view transform
    quint32 m_csmNumSplits = 0;
    float m_csmSplits[4] = {};
    float m_csmActive[4] = {};
    float m_shadowMapFar = 0.f;
    QMatrix4x4 m_fixedScaleBiasMatrix[4];
    QVector4D m_dimensionsInverted[4]; ///< (x, y, z, 0) 4 vec4

    struct AtlasEntry {
        int layerIndex = 0;
        float uOffset = 0.0f;
        float vOffset = 0.0f;
        float uvScale = 1.0f;
    } m_atlasInfo[4];
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderShadowMap
{
    Q_DISABLE_COPY(QSSGRenderShadowMap)

public:
    const QSSGRenderContextInterface &m_context;

    explicit QSSGRenderShadowMap(const QSSGRenderContextInterface &inContext);
    ~QSSGRenderShadowMap();
    void releaseCachedResources();
    void addShadowMaps(const QSSGShaderLightList &renderableLights);

    QSSGShadowMapEntry *shadowMapEntry(int lightIdx);
    QRhiTextureRenderTarget *layerRenderTarget(int layerIndex);
    QRhiRenderPassDescriptor *layerRenderPassDescriptor(int layerIndex);
    QRhiTexture *shadowMapAtlasTexture() const;
    QRhiTexture *shadowMapBlueNoiseTexture() const;

    qsizetype shadowMapEntryCount() { return m_shadowMapList.size(); }

    QRhiShaderResourceBindings *shadowClearSrb() const { return m_shadowClearSrb.get(); }

private:
    QSSGShadowMapEntry *addShadowMap(quint32 lightIdx,
                                     QSize size,
                                     QVector<QSSGShadowMapEntry::AtlasEntry> atlasEntries,
                                     quint32 csmNumSplits,
                                     QRhiTexture::Format format,
                                     bool isPointLight,
                                     const QString &renderNodeObjName);

    QVector<QSSGShadowMapEntry> m_shadowMapList;
    std::unique_ptr<QRhiTexture> m_shadowMapAtlasTexture;
    std::unique_ptr<QRhiTexture> m_shadowMapBlueNoiseTexture;
    std::unique_ptr<QRhiSampler> m_sharedCubeToAtlasSampler;
    std::unique_ptr<QRhiBuffer> m_sharedFrontCubeToAtlasUniformBuffer;
    std::unique_ptr<QRhiBuffer> m_sharedBackCubeToAtlasUniformBuffer;
    std::unique_ptr<QRhiShaderResourceBindings> m_shadowClearSrb;
    QVector<QRhiRenderBuffer *> m_layerDepthStencilBuffers;
    QVector<QRhiTextureRenderTarget *> m_layerRenderTargets;
    QVector<QRhiRenderPassDescriptor *> m_layerRenderPassDescriptors;
};

using QSSGRenderShadowMapPtr = std::shared_ptr<QSSGRenderShadowMap>;

QT_END_NAMESPACE

#endif
