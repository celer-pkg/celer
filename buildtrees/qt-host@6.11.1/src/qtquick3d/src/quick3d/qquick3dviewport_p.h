// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGVIEW3D_H
#define QSSGVIEW3D_H

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

#include <QtQuick/QQuickItem>
#include <QtCore/qurl.h>

#include <QtQuick3D/qtquick3dglobal.h>
#include <QtQuick3D/private/qquick3dpickresult_p.h>
#if QT_CONFIG(quick_shadereffect)
#include <QtQuick/private/qquickshadereffectsource_p.h>
#endif

#include <QtQuick3DRuntimeRender/private/qssgrenderpickresult_p.h>

#include "qquick3dsceneenvironment_p.h"
#include "qquick3drenderstats_p.h"
#include "qquick3dlightmapbaker_p.h"

QT_BEGIN_NAMESPACE

class QSSGView3DPrivate;
class QQuick3DCamera;
class QQuick3DSceneEnvironment;
class QQuick3DNode;
class QQuick3DSceneRootNode;
class QQuick3DSceneRenderer;
class QQuick3DRenderStats;
class QQuick3DSceneManager;

class SGFramebufferObjectNode;
class QQuick3DSGRenderNode;
class QQuick3DSGDirectRenderer;

class Q_QUICK3D_EXPORT QQuick3DViewport : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> data READ data DESIGNABLE false FINAL)
    Q_PROPERTY(QQuick3DCamera *camera READ camera WRITE setCamera NOTIFY cameraChanged FINAL)
    Q_PROPERTY(QQuick3DSceneEnvironment *environment READ environment WRITE setEnvironment NOTIFY environmentChanged FINAL)
    Q_PROPERTY(QQuick3DNode *scene READ scene NOTIFY sceneChanged)
    Q_PROPERTY(QQuick3DNode *importScene READ importScene WRITE setImportScene NOTIFY importSceneChanged FINAL)
    Q_PROPERTY(RenderMode renderMode READ renderMode WRITE setRenderMode NOTIFY renderModeChanged FINAL)
#if QT_CONFIG(quick_shadereffect)
    Q_PROPERTY(QQuickShaderEffectSource::Format renderFormat READ renderFormat WRITE setRenderFormat NOTIFY renderFormatChanged FINAL REVISION(6, 4))
#endif
    Q_PROPERTY(QQuick3DRenderStats *renderStats READ renderStats CONSTANT)
    Q_PROPERTY(QQmlListProperty<QQuick3DObject> extensions READ extensions FINAL REVISION(6, 6))
    Q_PROPERTY(int explicitTextureWidth READ explicitTextureWidth WRITE setExplicitTextureWidth NOTIFY explicitTextureWidthChanged FINAL REVISION(6, 7))
    Q_PROPERTY(int explicitTextureHeight READ explicitTextureHeight WRITE setExplicitTextureHeight NOTIFY explicitTextureHeightChanged FINAL REVISION(6, 7))
    Q_PROPERTY(QSize effectiveTextureSize READ effectiveTextureSize NOTIFY effectiveTextureSizeChanged FINAL REVISION(6, 7))
    Q_PROPERTY(RenderOverrides renderOverrides READ renderOverrides WRITE setRenderOverrides NOTIFY renderOverridesChanged FINAL REVISION(6, 11))
    Q_CLASSINFO("DefaultProperty", "data")

    QML_NAMED_ELEMENT(View3D)

public:
    enum RenderMode {
        Offscreen,
        Underlay,
        Overlay,
        Inline
    };
    Q_ENUM(RenderMode)

    enum class RenderOverride {
        None = 0x0,
        DisableInternalPasses = 0x1,
    };
    Q_ENUM(RenderOverride)
    Q_DECLARE_FLAGS(RenderOverrides, RenderOverride)
    Q_FLAG(RenderOverrides)

    explicit QQuick3DViewport(QQuickItem *parent = nullptr);
    ~QQuick3DViewport() override;

    QQmlListProperty<QObject> data();

    QQuick3DCamera *camera() const;
    QQuick3DSceneEnvironment *environment() const;
    QQuick3DNode *scene() const;
    QQuick3DNode *importScene() const;
    RenderMode renderMode() const;
#if QT_CONFIG(quick_shadereffect)
    Q_REVISION(6, 4) QQuickShaderEffectSource::Format renderFormat() const;
#endif
    QQuick3DRenderStats *renderStats() const;

    QQuick3DSceneRenderer *createRenderer() const;

    bool isTextureProvider() const override;
    QSGTextureProvider *textureProvider() const override;
    void releaseResources() override;

    Q_INVOKABLE QVector3D mapFrom3DScene(const QVector3D &scenePos) const;
    Q_INVOKABLE QVector3D mapTo3DScene(const QVector3D &viewPos) const;

    Q_INVOKABLE QQuick3DPickResult pick(float x, float y) const;
    Q_REVISION(6, 8) Q_INVOKABLE QQuick3DPickResult pick(float x, float y, QQuick3DModel *model) const;
    Q_REVISION(6, 8) Q_INVOKABLE QList<QQuick3DPickResult> pickSubset(float x, float y, const QJSValue &models) const;
    Q_REVISION(6, 2) Q_INVOKABLE QList<QQuick3DPickResult> pickAll(float x, float y) const;
    Q_REVISION(6, 2) Q_INVOKABLE QQuick3DPickResult rayPick(const QVector3D &origin, const QVector3D &direction) const;
    Q_REVISION(6, 2) Q_INVOKABLE QList<QQuick3DPickResult> rayPickAll(const QVector3D &origin, const QVector3D &direction) const;

    Q_REVISION(6, 11) Q_INVOKABLE QQuick3DPickResult rayPick(const QVector3D &origin, const QVector3D &direction, QQuick3DModel *model) const;
    Q_REVISION(6, 11) Q_INVOKABLE QQuick3DPickResult closestPointPick(const QVector3D &origin, float radius, QQuick3DModel *model = nullptr) const;
    Q_REVISION(6, 11) Q_INVOKABLE QList<QQuick3DObject *> pickInRect(const QPointF &start, const QPointF &end) const;

    void processPointerEventFromRay(const QVector3D &origin, const QVector3D &direction, QPointerEvent *event) const;
    bool singlePointPick(QSinglePointEvent *event, const QVector3D &origin, const QVector3D &direction);

    Q_REVISION(6, 8) Q_INVOKABLE void setTouchpoint(QQuickItem *target, const QPointF &position, int pointId, bool active);

    QQuick3DLightmapBaker *maybeLightmapBaker();
    QQuick3DLightmapBaker *lightmapBaker();

    Q_INVOKABLE void bakeLightmap();
    Q_REVISION(6, 10) Q_INVOKABLE void denoiseLightmap();

    QQmlListProperty<QQuick3DObject> extensions();

    Q_REVISION(6, 7) int explicitTextureWidth() const;
    Q_REVISION(6, 7) int explicitTextureHeight() const;
    Q_REVISION(6, 7) QSize effectiveTextureSize() const;

    // Private helpers
    [[nodiscard]] bool extensionListDirty() const { return m_extensionListDirty; }
    [[nodiscard]] const QList<QQuick3DObject *> &extensionList() const { return m_extensions; }
    void clearExtensionListDirty() { m_extensionListDirty = false; }

    Q_REVISION(6, 7) Q_INVOKABLE void rebuildExtensionList();

    enum class PrivateInstanceType : quint8 { XrViewInstance = 1 };
    explicit QQuick3DViewport(PrivateInstanceType type, QQuickItem *parent = nullptr);
    [[nodiscard]] bool isXrViewInstance() const { return m_isXrViewInstance; }

    static void updateCameraForLayer(const QQuick3DViewport &view3D, QSSGRenderLayer &layerNode);

    RenderOverrides renderOverrides() const;
    void setRenderOverrides(RenderOverrides newRenderOverrides);

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value) override;

    bool event(QEvent *) override;
    void componentComplete() override;

public Q_SLOTS:
    void setCamera(QQuick3DCamera *camera);
    void setEnvironment(QQuick3DSceneEnvironment * environment);
    void setImportScene(QQuick3DNode *inScene);
    void setRenderMode(QQuick3DViewport::RenderMode renderMode);
#if QT_CONFIG(quick_shadereffect)
    Q_REVISION(6, 4) void setRenderFormat(QQuickShaderEffectSource::Format format);
#endif
    Q_REVISION(6, 7) void setExplicitTextureWidth(int width);
    Q_REVISION(6, 7) void setExplicitTextureHeight(int height);
    void cleanupDirectRenderer();

    // Setting this true enables picking for all the models, regardless of
    // the models pickable property.
    void setGlobalPickingEnabled(bool isEnabled);

private Q_SLOTS:
    void invalidateSceneGraph();
    void updateInputProcessing();
    void onReleaseCachedResources();

Q_SIGNALS:
    void cameraChanged();
    void environmentChanged();
    void sceneChanged();
    void importSceneChanged();
    void renderModeChanged();
    Q_REVISION(6, 4) void renderFormatChanged();
    Q_REVISION(6, 7) void explicitTextureWidthChanged();
    Q_REVISION(6, 7) void explicitTextureHeightChanged();
    Q_REVISION(6, 7) void effectiveTextureSizeChanged();

    void renderOverridesChanged();

private:
    void setMultiViewCameras(QQuick3DCamera **firstCamera, int count);
    template <size_t N>
    void setMultiViewCameras(QQuick3DCamera *(&cameras)[N])
    {
        static_assert(N > 1, "Use setCamera for single view");
        setMultiViewCameras(cameras, N);
    }

    friend class QQuick3DExtensionListHelper;
    friend class QQuick3DXrManager;
    friend class QQuick3DXrManagerPrivate;
    friend class QQuick3DRenderLayerHelpers;

    Q_DISABLE_COPY(QQuick3DViewport)
    struct SubsceneInfo {
        QQuick3DObject* obj = nullptr;
        QVarLengthArray<QPointF, 16> eventPointScenePositions;
    };
    QQuick3DSceneRenderer *getRenderer() const;
    void updateDynamicTextures();
    QSGNode *setupOffscreenRenderer(QSGNode *node);
    QSGNode *setupInlineRenderer(QSGNode *node);
    void setupDirectRenderer(RenderMode mode);
    bool checkIsVisible() const;
    bool internalPick(QPointerEvent *event, const QVector3D &origin = QVector3D(), const QVector3D &direction = QVector3D()) const;
    QPair<QQuickItem *, QPointF> getItemAndPosition(const QSSGRenderPickResult &pickResult) const;
    QVarLengthArray<QSSGRenderPickResult, 20> getPickResults(QQuick3DSceneRenderer *renderer, const QVector3D &origin, const QVector3D &direction) const;
    QVarLengthArray<QSSGRenderPickResult, 20> getPickResults(QQuick3DSceneRenderer *renderer, const QEventPoint &eventPoint) const;
    bool forwardEventToSubscenes(QPointerEvent *event,
                                 bool useRayPicking,
                                 QQuick3DSceneRenderer *renderer,
                                 const QFlatMap<QQuickItem *, SubsceneInfo> &visitedSubscenes) const;

    void processPickedObject(const QSSGRenderPickResult &pickResult,
                             int pointIndex,
                             QPointerEvent *event,
                             QFlatMap<QQuickItem *, SubsceneInfo> &vistedSubscenes) const;
    QQuickItem *getSubSceneRootItem(QQuick3DMaterial *material) const;
    QQuick3DPickResult getNearestPickResult(const QVarLengthArray<QSSGRenderPickResult, 20> &pickResults) const;
    QQuick3DPickResult processPickResult(const QSSGRenderPickResult &pickResult) const;
    QQuick3DObject *findFrontendNode(const QSSGRenderGraphObject *backendObject) const;
    QQuick3DSceneManager *findChildSceneManager(QQuick3DObject *inObject, QQuick3DSceneManager *manager = nullptr);
    void updateSceneManagerForImportScene();
    QQuick3DCamera *m_camera = nullptr;
    QVarLengthArray<QQuick3DCamera *, 2> m_multiViewCameras;
    QQuick3DSceneEnvironment *m_environment = nullptr;
    mutable QPointer<QQuick3DSceneEnvironment> m_builtInEnvironment;
    QQuick3DSceneRootNode *m_sceneRoot = nullptr;
    QQuick3DNode *m_importScene = nullptr;
    mutable SGFramebufferObjectNode *m_node = nullptr;
    mutable QQuick3DSGRenderNode *m_renderNode = nullptr;
    mutable QQuick3DSGDirectRenderer *m_directRenderer = nullptr;
    bool m_renderModeDirty = false;
    RenderMode m_renderMode = Offscreen;
#if QT_CONFIG(quick_shadereffect)
    QQuickShaderEffectSource::Format m_renderFormat = QQuickShaderEffectSource::RGBA8;
#endif
    int m_explicitTextureWidth = 0;
    int m_explicitTextureHeight = 0;
    QSize m_effectiveTextureSize;
    float m_widthMultiplier = 1.0f;
    float m_heightMultiplier = 1.0f;
    QQuick3DRenderStats *m_renderStats = nullptr;
    bool m_enableInputProcessing = false;
    QQuick3DLightmapBaker *m_lightmapBaker = nullptr;
    QList<QQuick3DObject *> m_extensions;
    bool m_extensionListDirty = false;
    bool m_isXrViewInstance = false;

    struct TouchState {
        QQuickItem *target = nullptr;
        QPointF position;
        bool isPressed = false;
    };
    QPointingDevice *m_syntheticTouchDevice = nullptr;
    QVarLengthArray<TouchState, 2> m_touchState{2};

    QPointer<QQuickItem> m_prevMouseItem = nullptr;
    QPointF m_prevMousePos;
    RenderOverrides m_renderOverrides;

    Q_QUICK3D_PROFILE_ID
};

QT_END_NAMESPACE

#endif // QSSGVIEW3D_H
