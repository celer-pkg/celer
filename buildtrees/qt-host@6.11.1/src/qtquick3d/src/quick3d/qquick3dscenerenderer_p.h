// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSCENERENDERER_H
#define QSSGSCENERENDERER_H

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

#include <ssg/qssgrendercontextcore.h>

#include <qsgtextureprovider.h>
#include <qsgrendernode.h>
#include <QSGSimpleTextureNode>

#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrhieffectsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>

#include <QtCore/qpointer.h>

#include <optional>

#include "qquick3dsceneenvironment_p.h"

QT_BEGIN_NAMESPACE


class QQuick3DSceneManager;
class QQuick3DViewport;

class QQuick3DSceneRenderer
{
    using PickResultList = QVarLengthArray<QSSGRenderPickResult, 20>;
public:
    explicit QQuick3DSceneRenderer(const std::shared_ptr<QSSGRenderContextInterface> &rci);
    ~QQuick3DSceneRenderer();

    static QSSGRenderLayer::TonemapMode getTonemapMode(const QQuick3DSceneEnvironment &environment)
    {
        if (environment.useBuiltinTonemapper())
            return QSSGRenderLayer::TonemapMode(environment.tonemapMode());

        // Special case for the extend scene environment...
        return (environment.tonemapMode() != QQuick3DSceneEnvironment::QQuick3DEnvironmentTonemapModes::TonemapModeNone) ? QSSGRenderLayer::TonemapMode::Custom
                                                                                                                         : QSSGRenderLayer::TonemapMode::None;
    }

protected:
    QRhiTexture *renderToRhiTexture(QQuickWindow *qw);
    void beginFrame();
    void endFrame();
    void rhiPrepare(const QRect &viewport, qreal displayPixelRatio);
    void rhiRender();
    void synchronize(QQuick3DViewport *view3D, const QSize &size, float dpr);
    void invalidateFramebufferObject();
    QSize surfaceSize() const { return m_surfaceSize; }
    void releaseCachedResources();

    std::optional<QSSGRenderRay> getRayFromViewportPos(const QPointF &pos);
    PickResultList syncPick(const QSSGRenderRay &ray);
    PickResultList syncPickOne(const QSSGRenderRay &ray, QSSGRenderNode *node);
    PickResultList syncPickSubset(const QSSGRenderRay &ray, QVarLengthArray<QSSGRenderNode *> subset);
    PickResultList syncPickAll(const QSSGRenderRay &ray);
    std::optional<QSSGRenderPickResult> syncPickClosestPoint(const QVector3D &center, float radiusSquared, QSSGRenderNode *node);
    QList<const QSSGRenderNode *> syncPickInFrustum(const QSSGFrustum &frustum);

    void setGlobalPickingEnabled(bool isEnabled);

    QQuick3DRenderStats *renderStats();

private:
    void releaseAaDependentRhiResources();
    void updateLayerNode(QSSGRenderLayer &layerNode,
                         const QQuick3DViewport &view3D,
                         const QList<QSSGRenderGraphObject *> &resourceLoaders);
    void addNodeToLayer(QSSGRenderNode *node);
    void removeNodeFromLayer(QSSGRenderNode *node);
    void maybeSetupLightmapBaking(QQuick3DViewport *view3D);
    std::shared_ptr<QSSGRenderContextInterface> m_sgContext;
    QSSGRenderLayer *m_layer = nullptr;
    QPointer<QQuick3DWindowAttachment> winAttacment;
    QSize m_surfaceSize;
    SGFramebufferObjectNode *fboNode = nullptr;
    bool m_aaIsDirty = true;
    bool m_temporalIsDirty = false;
    bool m_timeBasedAA = false;

    // RHI
    QRhiTexture *m_texture = nullptr;
    // the rt is set up to output into m_texture or m_ssaaTexture or m_msaaRenderBuffer(+resolve into m_texture)
    QRhiTextureRenderTarget *m_textureRenderTarget = nullptr;
    QRhiRenderPassDescriptor *m_textureRenderPassDescriptor = nullptr;
    // used by the draw quad that does m_ssaaTexture -> m_texture
    QRhiTextureRenderTarget *m_ssaaTextureToTextureRenderTarget = nullptr;
    QRhiRenderPassDescriptor *m_ssaaTextureToTextureRenderPassDescriptor = nullptr;
    QRhiRenderBuffer *m_msaaRenderBufferLegacy = nullptr;
    QRhiTexture *m_msaaRenderTexture = nullptr;
    QRhiTexture *m_msaaMultiViewRenderBuffer = nullptr;
    QRhiTexture *m_ssaaTexture = nullptr;
    QRhiTexture *m_temporalAATexture = nullptr;
    QRhiTexture *m_prevTempAATexture = nullptr;
    QRhiTextureRenderTarget *m_temporalAARenderTarget = nullptr;
    QRhiRenderPassDescriptor *m_temporalAARenderPassDescriptor = nullptr;
    QRhiRenderBuffer *m_depthStencilBuffer = nullptr;
    QRhiTexture *m_multiViewDepthStencilBuffer = nullptr;
    bool m_textureNeedsFlip = true;
    QSSGRenderLayer::Background m_backgroundMode = QSSGRenderLayer::Background::Unspecified;
    QColor m_userBackgroundColor = Qt::black;
    QColor m_linearBackgroundColor = Qt::black;
    QColor m_tonemappedBackgroundColor = Qt::black;
    int m_samples = 1;
    QSSGRhiEffectSystem *m_effectSystem = nullptr;

    QPointer<QQuick3DRenderStats> m_renderStats;

    QSSGRenderNode *m_sceneRootNode = nullptr;
    QSSGRenderNode *m_importSceneRootNode = nullptr;

    bool m_prepared = false;

    QSSGLightmapperOptions lmOptions;
    bool m_lightmapBakingFromCmdRequested = false;
    bool m_lightmapDenoisingFromCmdRequested = false;

    int m_requestedFramesCount = 0;
    bool m_postProcessingStack = false;
    bool m_useFBO = false;
    Q_QUICK3D_PROFILE_ID

    friend class SGFramebufferObjectNode;
    friend class QQuick3DSGRenderNode;
    friend class QQuick3DSGDirectRenderer;
    friend class QQuick3DViewport;
    friend struct ViewportTransformHelper;
    friend class QQuick3DRenderLayerHelpers;
};

class Q_QUICK3D_EXPORT QQuick3DRenderLayerHelpers
{
public:
    static void updateLayerNodeHelper(const QQuick3DViewport &view3D,
                                      const std::shared_ptr<QSSGRenderContextInterface> &rci,
                                      QSSGRenderLayer &layerNode,
                                      bool &aaIsDirty,
                                      bool &temporalIsDirty);
};

class SGFramebufferObjectNode final : public QSGTextureProvider, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    SGFramebufferObjectNode();
    ~SGFramebufferObjectNode() override;

    void scheduleRender();

    QSGTexture *texture() const override;

    void preprocess() override;

public Q_SLOTS:
    void render();

    void handleScreenChange();

public:
    QQuickWindow *window;
    QQuick3DSceneRenderer *renderer;
    QQuick3DViewport *quickFbo;

    bool renderPending;
    bool invalidatePending;

    qreal devicePixelRatio;
};

class QQuick3DSGRenderNode final : public QSGRenderNode
{
public:
    ~QQuick3DSGRenderNode();
    void prepare() override;
    StateFlags changedStates() const override;
    void render(const RenderState *state) override;
    void releaseResources() override;
    RenderingFlags flags() const override;
public:
    QQuickWindow *window = nullptr;
    QQuick3DSceneRenderer *renderer = nullptr;
};

class QQuick3DSGDirectRenderer : public QObject
{
    Q_OBJECT
public:
    enum QQuick3DSGDirectRendererMode {
        Underlay,
        Overlay
    };
    QQuick3DSGDirectRenderer(QQuick3DSceneRenderer *renderer, QQuickWindow *window, QQuick3DSGDirectRendererMode mode = Underlay);
    ~QQuick3DSGDirectRenderer();

    QQuick3DSceneRenderer *renderer() { return m_renderer; }
    void setViewport(const QRectF &viewport);

    void requestRender();
    void setVisibility(bool visible);

    void preSynchronize();

private Q_SLOTS:
    void prepare();
    void render();

private:
    QQuick3DSceneRenderer *m_renderer = nullptr;
    QQuickWindow *m_window = nullptr;
    QRectF m_viewport;
    bool m_isVisible = true;
    QRhiTexture *m_rhiTexture = nullptr;
    bool renderPending = false;
};

QT_END_NAMESPACE

#endif // QSSGSCENERENDERER_H
