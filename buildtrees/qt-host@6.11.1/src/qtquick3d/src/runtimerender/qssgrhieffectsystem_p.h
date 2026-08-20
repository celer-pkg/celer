// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRHIEFFECTSYSTEM_P_H
#define QSSGRHIEFFECTSYSTEM_P_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendereffect_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercommands_p.h>

QT_BEGIN_NAMESPACE

struct QSSGRhiEffectTexture;
class QSSGRenderer;

class QSSGProgramGenerator;
class QSSGShaderLibraryManager;
class QSSGShaderCache;

struct QSSGEffectSceneCacheKey
{
    QByteArray m_shaderPathKey;
    quintptr m_cmd;
    int m_ubufIndex;

    size_t m_hashCode = 0;

    static size_t generateHashCode(const QByteArray &shaderPathKey, quintptr cmd, int ubufIndex)
    {
        return qHash(shaderPathKey) ^ qHash(cmd) ^ qHash(ubufIndex);
    }

    void updateHashCode()
    {
        m_hashCode = generateHashCode(m_shaderPathKey, m_cmd, m_ubufIndex);
    }

    bool operator==(const QSSGEffectSceneCacheKey &other) const
    {
        return m_shaderPathKey == other.m_shaderPathKey
                && m_cmd == other.m_cmd
                && m_ubufIndex == other.m_ubufIndex;
    }
};

inline size_t qHash(const QSSGEffectSceneCacheKey &key)
{
    return key.m_hashCode;
}

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRhiEffectSystem
{
public:
    explicit QSSGRhiEffectSystem(const std::shared_ptr<QSSGRenderContextInterface> &sgContext);
    ~QSSGRhiEffectSystem();

    void setup(QSize outputSize);
    QRhiTexture *process(const QSSGRenderLayer &layer,
                         QRhiTexture *inTexture,
                         QRhiTexture *inDepthTexture,
                         QRhiTexture *inNormalTexture,
                         QRhiTexture *inMotionVectorTexture);

    static QSSGRenderTextureFormat::Format overriddenOutputFormat(const QSSGRenderEffect *inEffect);

    static QSSGRhiShaderPipelinePtr buildShaderForEffect(const QSSGBindShader &inCmd,
                                                         QSSGProgramGenerator &generator,
                                                         QSSGShaderLibraryManager &shaderLib,
                                                         QSSGShaderCache &shaderCache,
                                                         bool isYUpInFramebuffer,
                                                         int viewCount);

private:
    void releaseResources();
    QSSGRhiEffectTexture *doRenderEffect(const QSSGRenderEffect *inEffect,
                                         QSSGRhiEffectTexture *inTexture,
                                         quint8 viewCount);

    void allocateBufferCmd(const QSSGAllocateBuffer *inCmd, QSSGRhiEffectTexture *inTexture, const QSSGRenderEffect *inEffect, quint8 viewCount);
    void applyInstanceValueCmd(const QSSGApplyInstanceValue *inCmd, const QSSGRenderEffect *inEffect);
    void applyValueCmd(const QSSGApplyValue *inCmd, const QSSGRenderEffect *inEffect);
    void bindShaderCmd(const QSSGBindShader *inCmd, const QSSGRenderEffect *inEffect, quint8 viewCount);
    void renderCmd(const QSSGRenderEffect *inEffect, QSSGRhiEffectTexture *inTexture, QSSGRhiEffectTexture *target, quint8 viewCount);

    void addCommonEffectUniforms(const QSSGRenderEffect *inEffect, const QSize &inputSize, const QSize &outputSize, quint8 viewCount);
    void addTextureToShaderPipeline(const QByteArray &name, QRhiTexture *texture, const QSSGRhiSamplerDescription &samplerDesc);

    QSSGRhiEffectTexture *findTexture(const QByteArray &bufferName);
    QSSGRhiEffectTexture *getTexture(const QByteArray &bufferName, const QSize &size,
                                     QRhiTexture::Format format, bool isFinalOutput,
                                     const QSSGRenderEffect *inEffect, quint8 viewCount);
    void releaseTexture(QSSGRhiEffectTexture *texture);
    void releaseTextures();

    QSize m_outSize;
    std::shared_ptr<QSSGRenderContextInterface> m_sgContext;
    QVector<QSSGRhiEffectTexture *> m_textures;
    QRhiTexture *m_depthTexture = nullptr;
    QRhiTexture *m_normalTexture = nullptr;
    QRhiTexture *m_motionVectorTexture = nullptr;
    QVector2D m_cameraClipRange;
    QVarLengthArray<QMatrix4x4, 2> m_projectionMatrices;
    QVarLengthArray<QMatrix4x4, 2> m_viewMatrices;
    int m_currentUbufIndex = 0;
    QHash<QSSGEffectSceneCacheKey, QSSGRhiShaderPipelinePtr> m_shaderPipelines;
    QSSGRhiShaderPipeline *m_currentShaderPipeline = nullptr;
    char *m_currentUBufData = nullptr;
    QHash<QByteArray, QSSGRhiTexture> m_currentTextures;
    QSet<QRhiTextureRenderTarget *> m_pendingClears;
};

QT_END_NAMESPACE

#endif
