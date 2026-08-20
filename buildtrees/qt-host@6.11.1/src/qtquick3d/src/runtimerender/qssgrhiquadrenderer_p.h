// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRHIQUADRENDERER_P_H
#define QSSGRHIQUADRENDERER_P_H

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
#include <QtQuick3DRuntimeRender/private/qssgrhicontext_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRhiQuadRenderer
{
public:
    // The expected usage is one of the following:
    // prepareQuad() + recordRenderQuadPass() right after each other (to record a full standalone renderpass, in the prepare phase)
    // prepareQuad() in prepare phase, then recordRenderQuad() in render phase (to draw a quad within the main renderpass)

    enum Flag {
        UvCoords = 0x01,
        DepthTest = 0x02,
        DepthWrite = 0x04,
        PremulBlend = 0x08,
        RenderBehind = 0x10
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    void prepareQuad(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *maybeRub);

    void recordRenderQuad(QSSGRhiContext *rhiCtx,
                          QSSGRhiGraphicsPipelineState *ps, QRhiShaderResourceBindings *srb,
                          QRhiRenderPassDescriptor *rpDesc, Flags flags);

    void recordRenderQuadPass(QSSGRhiContext *rhiCtx,
                              QSSGRhiGraphicsPipelineState *ps, QRhiShaderResourceBindings *srb,
                              QRhiTextureRenderTarget *rt, Flags flags);

private:
    void ensureBuffers(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *rub);

    QSSGRhiBufferPtr m_vbuf;
    QSSGRhiBufferPtr m_ibuf;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSSGRhiQuadRenderer::Flags)


class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRhiCubeRenderer
{
public:
    void prepareCube(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *maybeRub);

    void recordRenderCube(QSSGRhiContext *rhiCtx,
                          QSSGRhiGraphicsPipelineState *ps, QRhiShaderResourceBindings *srb,
                          QRhiRenderPassDescriptor *rpDesc, QSSGRhiQuadRenderer::Flags flags);
private:
    void ensureBuffers(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *rub);

    QSSGRhiBufferPtr m_vbuf;
    QSSGRhiBufferPtr m_ibuf;
};


QT_END_NAMESPACE

#endif
