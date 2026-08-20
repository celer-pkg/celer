// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QCPAINTERRHIRENDERER_P_H
#define QCPAINTERRHIRENDERER_P_H

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

#include <rhi/qrhi.h>
#include "qcpainterengineutils_p.h"
#include "qcanvascustombrush_p.h"
#include "qcanvasoffscreencanvas.h"
#include <functional>
#include <optional>

QT_BEGIN_NAMESPACE

class QCPainterEngine;
struct QCRHIContext;
class QCanvasPainter;
class QCanvasCustomBrush;
struct QCRHIPipelineStateKey;
struct QCRHISamplerDesc;
struct QCRHICall;
struct QCRHIPath;
struct QCRHICommonUniforms;
class QCanvasPath;

struct QCRHITexture
{
    QRhiTexture *tex = nullptr;
    int id = 0;
    int width = 0;
    int height = 0;
    QCanvasPainter::ImageFlags flags = {};
};

struct QCRhiCanvas
{
    QRhiTexture *tex = nullptr;
    QRhiRenderBuffer *msaaColorBuffer = nullptr;
    QRhiRenderBuffer *ds = nullptr;
    QRhiTextureRenderTarget *rt = nullptr;
    QRhiRenderPassDescriptor *rp = nullptr;
    QCanvasOffscreenCanvas::Flags flags;
    bool isNull() const { return !tex; }
    void destroy();
};

bool operator==(const QCRhiCanvas &a, const QCRhiCanvas &b) noexcept;
bool operator!=(const QCRhiCanvas &a, const QCRhiCanvas &b) noexcept;

struct QCRhiUncachedPathDrawArgs
{
    const QCPaths &paths;
    int pathsCount;
};

struct QCRhiCachedPathDrawArgs
{
    QCanvasPath *canvasPath;
    int pathGroup;
    const QTransform &pathTransform;
    std::optional<QCRhiUncachedPathDrawArgs> updateData;
};

class QCPainterRhiRenderer
{
public:
    enum RenderFlag {
        Antialiasing = 1 << 0,
        StencilStrokes = 1 << 1,
        TransformedClipping = 1 << 3,
        SimpleClipping = 1 << 4,
        DepthTest = 1 << 5,
        CustomMatrix = 1 << 6,
    };
    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

    QCPainterRhiRenderer();
    ~QCPainterRhiRenderer();
    void create(QRhi *rhi, QCanvasPainter *painter);
    void destroy();
    bool isValid() const { return ctx != nullptr; }

    QCPainterEngine *engine() { return m_e; }

    // beginPrepare() and endPrepare() must be called outside a render pass.
    //
    // If devicePixelRatio is 0, rt->devicePixelRatio() is used. Specifying an
    // override is relevant when the target is a texture that is independent
    // from any on-screen window: rt's dpr is 1 then, but we may still want to
    // take an on-screen widget or window's dpr into account. (because we know
    // that the texture is eventually used in that window for something)
    //
    void beginPrepare(QRhiCommandBuffer *cb,
                      QRhiRenderTarget *rt,
                      float devicePixelRatio = 0.0f);
    void endPrepare();

    // Conveniences doing both beginPrepare and engine->beginPaint
    void beginPrepareAndPaint(QRhiCommandBuffer *cb,
                              QRhiRenderTarget *rt);
    void beginPrepareAndPaint(QRhiCommandBuffer *cb,
                              QRhiRenderTarget *rt,
                              float logicalWidth, float logicalHeight, float dpr);
    void setCustomMatrix(const QMatrix4x4 &matrix);
    void endPrepareAndPaint();

    // then when recording the render pass, call render()
    void render();

    void resetForNewFrame(); // to be called once for the whole frame
    void resetForPass(); // called by beginPrepare()
    void syncDebugCounters();
    void resetDebugCounters();

    bool hasDrawCalls() const;

    bool testFlag(RenderFlag flag) const;
    void setFlag(RenderFlags flag, bool enable);
    RenderFlags flags() const;

    bool renderCreate();

    QCRHITexture *renderCreateNativeTexture(QRhiTexture *texture, QCanvasPainter::ImageFlags flags = {});
    QCRHITexture *renderUpdateNativeTexture(QRhiTexture *oldTexture, QRhiTexture *texture);
    bool isOffscreenCanvasYUp() const;

    int renderCreateTexture(QCTextureFormat type, int w, int h, QCanvasPainter::ImageFlags imageFlags, const uchar* data);
    bool renderDeleteTexture(int image);
    bool renderUpdateTexture(int image, int x, int y, int w, int h, const uchar* data);
    void setViewport(float x, float y, float width, float height);
    void renderFill(const QCPaint &paint, const QCState &state,
                    const QRectF &bounds,
                    std::optional<QCRhiUncachedPathDrawArgs> uncachedPathInfo,
                    std::optional<QCRhiCachedPathDrawArgs> cachedPathInfo);
    void renderStroke(const QCPaint &paint, const QCState &state,
                      float strokeWidth,
                      std::optional<QCRhiUncachedPathDrawArgs> uncachedPathInfo,
                      std::optional<QCRhiCachedPathDrawArgs> cachedPathInfo);

#ifndef QCPAINTER_DISABLE_TEXT_SUPPORT
    void renderTextFill(
        const QCPaint &paint,
        const QCState &state,
        const QCRhiDistanceFieldGlyphCache::VertexList &verts,
        const QCRhiDistanceFieldGlyphCache::IndexList &indices);
    void renderTextFillCustom(
        const QCPaint &paint,
        const QCState &state,
        QCanvasCustomBrush *brush,
        const QCRhiDistanceFieldGlyphCache::VertexList &verts,
        const QCRhiDistanceFieldGlyphCache::IndexList &indices);
    int populateFont(
        const QFont &font,
        const QRectF &rect,
        const QString &text,
        QCRhiDistanceFieldGlyphCache::VertexList &vertices,
        QCRhiDistanceFieldGlyphCache::IndexList &indices,
        int *textureWidth,
        int *textureHeight);
#endif
    void renderDelete();

    bool isPathCachedForFill(QCanvasPath *path, int pathGroup, const QCCachedPathFillProperties &fillProperties);
    bool isPathCachedForStroke(QCanvasPath *path, int pathGroup, const QCCachedPathStrokeProperties &strokeProperties);
    void removePathGroup(int pathGroup);

    static void textureFormatInfo(QRhiTexture::Format format, QSize size,
                                  quint32 *bpl, quint32 *byteSize, quint32 *bytesPerPixel);

    QCanvasOffscreenCanvas createCanvas(QSize pixelSize, int sampleCount, QCanvasOffscreenCanvas::Flags flags);
    void destroyCanvas(QCanvasOffscreenCanvas &canvas);
    QRhiRenderTarget *canvasRenderTarget(const QCanvasOffscreenCanvas &canvas);
    void recordCanvasRenderPass(QRhiCommandBuffer *cb, const QCanvasOffscreenCanvas &canvas);
    void grabCanvas(const QCanvasOffscreenCanvas &canvas, std::function<void(const QImage &)> callback, QRhiCommandBuffer *maybeCb);

    void recordRenderPass(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const QColor &clearColor);

    bool isYUpInNDC() const;

private:
    QRhiGraphicsPipeline *pipeline(const QCRHIPipelineStateKey &key,
                                   QRhiRenderPassDescriptor *rpDesc,
                                   QRhiShaderResourceBindings *srb);
    QRhiSampler *sampler(const QCRHISamplerDesc &samplerDescription);
    QRhiResourceUpdateBatch *resourceUpdateBatch();
    void commitResourceUpdates();
    QCRHITexture *findTexture(QRhiTexture *texture) const;
    QCRHITexture *findTexture(int id) const;
    QRhiShaderResourceBindings *createSrb(int brushImage, int fontImage);
    void updateVertUniforms(QCRHICall *call, const QTransform &transform);

    QCRHICall *allocCall();
    QCRHITexture *allocTexture();
    int allocPaths(int count);
    int allocVerts(int count);
    int allocIndices(int count);
    int allocCommonUniforms(int count);

    QCRHICommonUniforms *uniformPtr(int i) const;
    QCanvasCustomBrushPrivate::CommonUniforms *customUniformPtr(int i) const;
    void preparePaint(QCRHICommonUniforms* frag, const QCPaint &paint,
                      const QCState &state, float width, float aa, float strokeThr,
                      float fontAlphaMin, float fontAlphaMax);
    void prepareCustomPaint(QCanvasCustomBrushPrivate::CommonUniforms* frag, const QCPaint &paint,
                            QCanvasCustomBrush *brush, const QCState &state,
                            float width, float aa, float strokeThr,
                            float fontAlphaMin, float fontAlphaMax);
    void bindPipeline(QCRHICall *call,
                      int pipelineIndex, int srbIndex,
                      const QRhiCommandBuffer::DynamicOffset &vertDynamicOffset,
                      const QRhiCommandBuffer::DynamicOffset &dynamicOffset,
                      bool indexedDraw,
                      bool *needsViewport);

    int transferFillGeom(QCRHICall *call,
                         int vertexCount, const QCVertex *vertices, int indexCount,
                         int pathCount, const QCPath *pathInfos);
    void transferStrokeGeom(QCRHICall *call,
                            int vertexCount, const QCVertex *vertices,
                            int pathCount, const QCPath *pathInfos);
    void transferPathsFromCachedPathGroup(QCRHICall *call, int pathCount, const QCRHIPath *paths);

private:
    friend class QCanvasPainter;
    friend class QCanvasPainterPrivate;

    QCContext* createRhiContext(QRhi *rhi);

    QCContext *ctx = nullptr;
    QCRHIContext *rhiCtx = nullptr;
    QCanvasPainter *m_painter = nullptr;
    QCPainterEngine *m_e = nullptr;
    QVector<QCRhiCanvas> m_canvases;

    QVector<std::pair<QRhiReadbackResult, std::function<void(const QImage &)>>> m_canvasGrabs;

    // Log draw calls and triangle amounts for each painting type.
    int logFillDrawCallCount = 0;
    int logStrokeDrawCallCount = 0;
    int logTextDrawCallCount = 0;
    int logFillTriCount = 0;
    int logStrokeTriCount = 0;
    int logTextTriCount = 0;
};

QT_END_NAMESPACE

#endif // QCPAINTERRHIRENDERER_P_H
