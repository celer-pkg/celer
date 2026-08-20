// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_REFLECTION_MAP_H
#define QSSG_RENDER_REFLECTION_MAP_H

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
#include <QtQuick3DRuntimeRender/private/qssgrenderreflectionprobe_p.h>

QT_BEGIN_NAMESPACE

class QRhi;
class QRhiCommandBuffer;
class QSSGRhiContext;
class QSSGRenderContextInterface;

class QRhiRenderBuffer;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiTexture;
class QRhiGraphicsPipeline;
class QRhiShaderResourceBindings;
class QRhiBuffer;

struct QSSGReflectionMapEntry
{
    QSSGReflectionMapEntry();

    static QSSGReflectionMapEntry withRhiTexturedCubeMap(quint32 probeIdx,
                                                         QRhiTexture *preFiltered);
    static QSSGReflectionMapEntry withRhiCubeMap(quint32 probeIdx,
                                                 QRhiTexture *cube,
                                                 QRhiTexture *prefiltered,
                                                 QRhiRenderBuffer *depthStencil);

    void renderMips(QSSGRhiContext *rhiCtx);
    void destroyRhiResources();

    quint32 m_probeIndex;

    // RHI resources
    QRhiTexture *m_rhiCube = nullptr;
    QRhiTexture *m_rhiPrefilteredCube = nullptr;
    QRhiRenderBuffer *m_rhiDepthStencil = nullptr;
    QVarLengthArray<QRhiTextureRenderTarget *, 6> m_rhiRenderTargets;
    QRhiRenderPassDescriptor *m_rhiRenderPassDesc = nullptr;

    QRhiGraphicsPipeline *m_prefilterPipeline = nullptr;
    QRhiGraphicsPipeline *m_irradiancePipeline = nullptr;
    QRhiShaderResourceBindings *m_prefilterSrb = nullptr;
    QRhiShaderResourceBindings *m_irradianceSrb = nullptr;
    QRhiBuffer *m_prefilterVertBuffer = nullptr;
    QRhiBuffer *m_prefilterFragBuffer = nullptr;
    QRhiBuffer *m_irradianceFragBuffer = nullptr;
    QMap<int, QVarLengthArray<QRhiTextureRenderTarget *, 6>> m_rhiPrefilterRenderTargetsMap;
    QRhiRenderPassDescriptor *m_rhiPrefilterRenderPassDesc = nullptr;
    QMap<int, QSize> m_prefilterMipLevelSizes;

    QVarLengthArray<QRhiShaderResourceBindings *, 6> m_skyBoxSrbs;

    QMatrix4x4 m_viewProjection;

    bool m_needsRender = false;
    bool m_rendered = false;

    QSSGRenderReflectionProbe::ReflectionTimeSlicing m_timeSlicing = QSSGRenderReflectionProbe::ReflectionTimeSlicing::None;
    int m_timeSliceFrame = 1;
    QSSGRenderTextureCubeFace m_timeSliceFace = { QSSGRenderTextureCubeFaces[0] };
    Q_QUICK3D_PROFILE_ID
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderReflectionMap
{
    typedef QVector<QSSGReflectionMapEntry> TReflectionMapEntryList;
    Q_DISABLE_COPY(QSSGRenderReflectionMap)

public:
    const QSSGRenderContextInterface &m_context;

    explicit QSSGRenderReflectionMap(const QSSGRenderContextInterface &inContext);
    ~QSSGRenderReflectionMap();
    void releaseCachedResources();

    void addReflectionMapEntry(qint32 probeIdx, const QSSGRenderReflectionProbe &probe);
    void addTexturedReflectionMapEntry(qint32 probeIdx, const QSSGRenderReflectionProbe &probe);

    QSSGReflectionMapEntry *reflectionMapEntry(int probeIdx);

    qint32 reflectionMapEntryCount() { return m_reflectionMapList.size(); }

private:
    TReflectionMapEntryList m_reflectionMapList;
};

using QSSGRenderReflectionMapPtr = std::shared_ptr<QSSGRenderReflectionMap>;

QT_END_NAMESPACE

#endif
