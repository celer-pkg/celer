// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dviewport_p.h"
#include "qquick3dsceneenvironment_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3dtexture_p.h"
#include "qquick3dscenerenderer_p.h"
#include "qquick3dscenerootnode_p.h"
#include "qquick3dcamera_p.h"
#include "qquick3dmodel_p.h"
#include "qquick3drenderstats_p.h"
#include "qquick3ditem2d_p.h"
#include "qquick3ddefaultmaterial_p.h"
#include "qquick3dprincipledmaterial_p.h"
#include "qquick3dcustommaterial_p.h"
#include "qquick3dspecularglossymaterial_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderlayer_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>

#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <QtQuick3DUtils/private/qssgfrustum_p.h>

#include <qsgtextureprovider.h>
#include <QSGSimpleTextureNode>
#include <QSGRendererInterface>
#include <QQuickWindow>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickpointerhandler_p.h>

#include <QtQml>

#include <QtGui/private/qeventpoint_p.h>

#include <QtCore/private/qnumeric_p.h>
#include <QtCore/qpointer.h>

#include <optional>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcEv, "qt.quick3d.event")
Q_STATIC_LOGGING_CATEGORY(lcPick, "qt.quick3d.pick")

static bool isforceInputHandlingSet()
{
    static const bool v = (qEnvironmentVariableIntValue("QT_QUICK3D_FORCE_INPUT_HANDLING") > 0);
    return v;
}

struct ViewportTransformHelper : public QQuickDeliveryAgent::Transform
{
    static void removeAll() {
        for (auto o : owners) {
            if (!o.isNull())
                o->setSceneTransform(nullptr);
        }
        owners.clear();
    }

    void setOnDeliveryAgent(QQuickDeliveryAgent *da) {
        da->setSceneTransform(this);
        owners.append(da);
    }

    /*
        Transforms viewport coordinates to 2D scene coordinates.
        Returns the point in targetItem corresponding to \a viewportPoint,
        assuming that targetItem is mapped onto sceneParentNode.
        If it's no longer a "hit" on sceneParentNode, returns the last-good point.
    */
    QPointF map(const QPointF &viewportPoint) override {
        QPointF point = viewportPoint;
        // Despite the name, the input coordinates are the window viewport coordinates
        // so unless the View3D is the same size of the Window, we need to translate
        // to the View3D coordinates before doing any picking.
        if (viewport)
            point = viewport->mapFromScene(viewportPoint);
        point.rx() *= scaleX;
        point.ry() *= scaleY;
        std::optional<QSSGRenderRay> rayResult = renderer->getRayFromViewportPos(point);
        if (rayResult.has_value()) {
            const auto pickResults = renderer->syncPickOne(rayResult.value(), sceneParentNode);
            if (!pickResults.isEmpty()) {
                const auto pickResult = pickResults.first();
                auto ret = pickResult.m_localUVCoords.toPointF();
                if (!uvCoordsArePixels) {
                    ret = QPointF(targetItem->x() + ret.x() * targetItem->width(),
                                  targetItem->y() - ret.y() * targetItem->height() + targetItem->height());
                }
                const bool outOfModel = pickResult.m_localUVCoords.isNull();
                qCDebug(lcEv) << viewportPoint << "->" << (outOfModel ? "OOM" : "") << ret << "@" << pickResult.m_scenePosition
                              << "UV" << pickResult.m_localUVCoords << "dist" << qSqrt(pickResult.m_distanceSq);
                if (outOfModel) {
                    return lastGoodMapping;
                } else {
                    lastGoodMapping = ret;
                    return ret;
                }
            }
        }
        return QPointF();
    }

    QPointer<QQuick3DViewport> viewport;
    QQuick3DSceneRenderer *renderer = nullptr;
    QSSGRenderNode *sceneParentNode = nullptr;
    QPointer<QQuickItem> targetItem;
    qreal scaleX = 1;
    qreal scaleY = 1;
    bool uvCoordsArePixels = false; // if false, they are in the range 0..1
    QPointF lastGoodMapping;

    static QList<QPointer<QQuickDeliveryAgent>> owners;
};

QList<QPointer<QQuickDeliveryAgent>> ViewportTransformHelper::owners;

class QQuick3DExtensionListHelper
{
    Q_DISABLE_COPY_MOVE(QQuick3DExtensionListHelper);
public:
    static void extensionAppend(QQmlListProperty<QQuick3DObject> *list, QQuick3DObject *extension)
    {
        QSSG_ASSERT(list && extension, return);

        if (QQuick3DViewport *that = qobject_cast<QQuick3DViewport *>(list->object)) {
            if (const auto idx = that->m_extensions.indexOf(extension); idx == -1) {
                if (!extension->parentItem())
                    extension->setParentItem(that->m_sceneRoot);
                that->m_extensions.push_back(extension);
                that->m_extensionListDirty = true;
            }
        }
    }
    static QQuick3DObject *extensionAt(QQmlListProperty<QQuick3DObject> *list, qsizetype index)
    {
        QQuick3DObject *ret = nullptr;
        QSSG_ASSERT(list, return ret);

        if (QQuick3DViewport *that = qobject_cast<QQuick3DViewport *>(list->object)) {
            if (that->m_extensions.size() > index)
                ret = that->m_extensions.at(index);
        }

        return ret;
    }
    static qsizetype extensionCount(QQmlListProperty<QQuick3DObject> *list)
    {
        qsizetype ret = -1;
        QSSG_ASSERT(list, return ret);

        if (QQuick3DViewport *that = qobject_cast<QQuick3DViewport *>(list->object))
            ret = that->m_extensions.size();

        return ret;
    }
    static void extensionClear(QQmlListProperty<QQuick3DObject> *list)
    {
        QSSG_ASSERT(list, return);

        if (QQuick3DViewport *that = qobject_cast<QQuick3DViewport *>(list->object)) {
            that->m_extensions.clear();
            that->m_extensionListDirty = true;
        }
    }
    static void extensionReplace(QQmlListProperty<QQuick3DObject> *list, qsizetype idx, QQuick3DObject *o)
    {
        QSSG_ASSERT(list, return);

        if (QQuick3DViewport *that = qobject_cast<QQuick3DViewport *>(list->object)) {
            if (that->m_extensions.size() > idx && idx > -1) {
                that->m_extensions.replace(idx, o);
                that->m_extensionListDirty = true;
            }
        }
    }
        static void extensionRemoveLast(QQmlListProperty<QQuick3DObject> *list)
    {
        QSSG_ASSERT(list, return);

        if (QQuick3DViewport *that = qobject_cast<QQuick3DViewport *>(list->object)) {
            that->m_extensions.removeLast();
            that->m_extensionListDirty = true;
        }
    }
};

/*!
    \qmltype View3D
    \inherits Item
    \inqmlmodule QtQuick3D
    \brief Provides a viewport on which to render a 3D scene.

    View3D provides a 2D surface on which a 3D scene can be rendered. This
    surface is a Qt Quick \l Item and can be placed in a Qt Quick scene.

    There are two ways to define the 3D scene that is visualized on the View3D:
    If you define a hierarchy of \l{Node}{Node-based} items as children of
    the View3D directly, then this will become the implicit scene of the View3D.

    It is also possible to reference an existing scene by using the \l importScene
    property and setting it to the root \l Node of the scene you want to visualize.
    This \l Node does not have to be an ancestor of the View3D, and you can have
    multiple View3Ds that import the same scene.

    This is demonstrated in \l {Qt Quick 3D - View3D example}{View3D example}.

    If the View3D both has child \l{Node}{Nodes} and the \l importScene property is
    set simultaneously, then both scenes will be rendered as if they were sibling
    subtrees in the same scene.

    To control how a scene is rendered, you can set the \l environment
    property. The type \l SceneEnvironment has a number of visual properties
    that can be adjusted, such as background color, tone mapping, anti-aliasing
    and more. \l ExtendedSceneEnvironment in the \c{QtQuick3D.Helpers} module
    extends \l SceneEnvironment with even more features, adding common
    post-processing effects.

    In addition, in order for anything to be rendered in the View3D, the scene
    needs a \l Camera. If there is only a single \l Camera in the scene, then
    this will automatically be picked. Otherwise, the \l camera property can
    be used to select the camera. The \l Camera decides which parts of the scene
    are visible, and how they are projected onto the 2D surface.

    By default, the 3D scene will first be rendered into an off-screen buffer and
    then composited with the rest of the Qt Quick scene when it is done. This provides
    the maximum level of compatibility, but may have performance implications on some
    graphics hardware. If this is the case, the \l renderMode property can be used to
    switch how the View3D is rendered into the window.

    A View3D with the default Offscreen \l renderMode is implicitly a
    \l{QSGTextureProvider}{texture provider} as well. This means that \l
    ShaderEffect or \l{QtQuick3D::Texture::sourceItem}{Texture.sourceItem} can reference
    the View3D directly as long as all items live within the same
    \l{QQuickWindow}{window}. Like with any other \l Item, it is also possible
    to switch the View3D, or one of its ancestors, into a texture-based
    \l{QtQuick::Item::layer.enabled}{item layer}.

    \sa {Qt Quick 3D - View3D example}
*/

QQuick3DViewport::QQuick3DViewport(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    m_camera = nullptr;
    m_sceneRoot = new QQuick3DSceneRootNode(this);
    m_renderStats = new QQuick3DRenderStats();
    QQuick3DSceneManager *sceneManager = new QQuick3DSceneManager();
    QQuick3DObjectPrivate::get(m_sceneRoot)->refSceneManager(*sceneManager);
    Q_ASSERT(sceneManager == QQuick3DObjectPrivate::get(m_sceneRoot)->sceneManager);
    connect(sceneManager, &QQuick3DSceneManager::needsUpdate,
            this, &QQuickItem::update);

    // Overrides the internal input handling to always be true
    // instead of potentially updated after a sync (see updatePaintNode)
    if (isforceInputHandlingSet()) {
        m_enableInputProcessing = true;
        updateInputProcessing();
        forceActiveFocus();
    }
}

QQuick3DViewport::~QQuick3DViewport()
{
    // With the threaded render loop m_directRenderer must be destroyed on the
    // render thread at the proper time, not here. That's handled in
    // releaseResources() + upon sceneGraphInvalidated. However with a
    // QQuickRenderControl-based window on the main thread there is a good
    // chance that this viewport (and so our sceneGraphInvalidated signal
    // connection) is destroyed before the window and the rendercontrol. So act
    // here then.
    if (m_directRenderer && m_directRenderer->thread() == thread()) {
        delete m_directRenderer;
        m_directRenderer = nullptr;
    }

    // If the quick window still exists, make sure to disconnect any of the direct
    // connections to this View3D
    if (auto qw = window())
        disconnect(qw, nullptr, this, nullptr);

    auto sceneManager = QQuick3DObjectPrivate::get(m_sceneRoot)->sceneManager;
    if (sceneManager) {
        sceneManager->setParent(nullptr);
        if (auto wa = sceneManager->wattached)
            wa->queueForCleanup(sceneManager);
    }

    delete m_sceneRoot;
    m_sceneRoot = nullptr;

    delete m_builtInEnvironment;

    // m_renderStats is tightly coupled with the render thread, so can't delete while we
    // might still be rendering.
    m_renderStats->deleteLater();

    if (!window() && sceneManager && sceneManager->wattached)
        QMetaObject::invokeMethod(sceneManager->wattached, &QQuick3DWindowAttachment::evaluateEol, Qt::QueuedConnection);
}

static void ssgn_append(QQmlListProperty<QObject> *property, QObject *obj)
{
    if (!obj)
        return;
    QQuick3DViewport *view3d = static_cast<QQuick3DViewport *>(property->object);

    if (QQuick3DObject *sceneObject = qmlobject_cast<QQuick3DObject *>(obj)) {
        QQmlListProperty<QObject> itemProperty = QQuick3DObjectPrivate::get(view3d->scene())->data();
        itemProperty.append(&itemProperty, sceneObject);
    } else {
        QQuickItemPrivate::data_append(property, obj);
    }
}

static qsizetype ssgn_count(QQmlListProperty<QObject> *property)
{
    QQuick3DViewport *view3d = static_cast<QQuick3DViewport *>(property->object);
    if (!view3d || !view3d->scene() || !QQuick3DObjectPrivate::get(view3d->scene())->data().count)
        return 0;
    QQmlListProperty<QObject> itemProperty = QQuick3DObjectPrivate::get(view3d->scene())->data();
    return itemProperty.count(&itemProperty);
}

static QObject *ssgn_at(QQmlListProperty<QObject> *property, qsizetype i)
{
    QQuick3DViewport *view3d = static_cast<QQuick3DViewport *>(property->object);
    QQmlListProperty<QObject> itemProperty = QQuick3DObjectPrivate::get(view3d->scene())->data();
    return itemProperty.at(&itemProperty, i);
}

static void ssgn_clear(QQmlListProperty<QObject> *property)
{
    QQuick3DViewport *view3d = static_cast<QQuick3DViewport *>(property->object);
    QQmlListProperty<QObject> itemProperty = QQuick3DObjectPrivate::get(view3d->scene())->data();
    return itemProperty.clear(&itemProperty);
}


QQmlListProperty<QObject> QQuick3DViewport::data()
{
    return QQmlListProperty<QObject>(this,
                                     nullptr,
                                     ssgn_append,
                                     ssgn_count,
                                     ssgn_at,
                                     ssgn_clear);
}

/*!
    \qmlproperty QtQuick3D::Camera QtQuick3D::View3D::camera

    This property specifies which \l Camera is used to render the scene. If this
    property is not set, then the first enabled camera in the scene will be used.

    \note It is strongly recommended to set this property explicitly rather than relying on
    automatic camera selection. When multiple cameras are present in the scene, automatic
    selection does not guarantee which camera will be chosen. When \l{Node::layer}{layers}
    are used, explicitly specifying the camera also avoids unnecessary re-evaluation of
    scene nodes.

    \note If this property contains a camera that's not \l {Node::visible}{visible} then
    no further attempts to find a camera will be done.

    \sa PerspectiveCamera, OrthographicCamera, FrustumCamera, CustomCamera
*/
QQuick3DCamera *QQuick3DViewport::camera() const
{
    return m_camera;
}

/*!
    \qmlproperty QtQuick3D::SceneEnvironment QtQuick3D::View3D::environment

    This property specifies the SceneEnvironment used to render the scene.

    \note Setting this property to \c null will reset the SceneEnvironment to the default.

    \sa SceneEnvironment
*/
QQuick3DSceneEnvironment *QQuick3DViewport::environment() const
{
    if (!m_environment) {
        if (!m_builtInEnvironment) {
            m_builtInEnvironment = new QQuick3DSceneEnvironment;
            // Check that we are on the "correct" thread, and move the environment to the
            // correct thread if not. This can happen when environment() is called from the
            // sync and no scene environment has been set.
            if (QThread::currentThread() != m_sceneRoot->thread())
                m_builtInEnvironment->moveToThread(m_sceneRoot->thread());
            m_builtInEnvironment->setParentItem(m_sceneRoot);
        }

        return m_builtInEnvironment;
    }

    return m_environment;
}

/*!
    \qmlproperty QtQuick3D::Node QtQuick3D::View3D::scene
    \readonly

    Returns the root \l Node of the View3D's scene.

    To define the 3D scene that is visualized in the View3D:

    \list
    \li  Define a hierarchy of \l{Node}{Node-based} items as children of
        the View3D directly, then this will become the implicit \l scene of the
        View3D.
    \li Reference an existing scene by using the \l importScene property and
        set it to the root \l Node of the scene you want to visualize. This
        \l Node does not have to be an ancestor of the View3D, and you can have
        multiple View3Ds that import the same scene.
    \endlist

    \sa importScene
*/
QQuick3DNode *QQuick3DViewport::scene() const
{
    return m_sceneRoot;
}

/*!
    \qmlproperty QtQuick3D::Node QtQuick3D::View3D::importScene

    This property defines the reference node of the scene to render to the
    viewport. The node does not have to be a child of the View3D. This
    referenced node becomes a sibling with child nodes of View3D, if there are
    any.

    \note Scenes can only be shared between View3D items that are in the same
    \l{QQuickWindow}{window}.

    \note When sharing scenes between multiple View3D items the imported scene should
    be imported in whole, that is, importing a subtree of a scene is not supported.
    If multiple View3Ds need to show different parts of the same shared scene,
    consider using \l {Node::layer}{layers} instead.

    \note This property can only be set once, and subsequent changes will have
    no effect.

    You can also define a hierarchy of \l{Node}{Node-based} items as children of
    the View3D directly, then this will become the implicit scene of the
    View3D.

    To return the current scene of the View3D, use the \l scene property.

    \sa Node
*/
QQuick3DNode *QQuick3DViewport::importScene() const
{
    return m_importScene;
}

/*!
    \qmlproperty enumeration QtQuick3D::View3D::renderMode

    This property determines how the View3D is combined with the other parts of the
    Qt Quick scene.

    By default, the scene will be rendered into an off-screen buffer as an intermediate
    step. This off-screen buffer is then rendered into the window (or render target) like any
    other Qt Quick \l Item.

    For most users, there will be no need to change the render mode, and this property can
    safely be ignored. But on some graphics hardware, the use of an off-screen buffer can be
    a performance bottleneck. If this is the case, it might be worth experimenting with other
    modes.

    \value View3D.Offscreen The scene is rendered into an off-screen buffer as an intermediate
    step. This off-screen buffer is then composited with the rest of the Qt Quick scene.

    \value View3D.Underlay The scene is rendered directly to the window before the rest of
    the Qt Quick scene is rendered. With this mode, the View3D cannot be placed on top of
    other Qt Quick items.

    \value View3D.Overlay The scene is rendered directly to the window after Qt Quick is
    rendered.  With this mode, the View3D will always be on top of other Qt Quick items.

    \value View3D.Inline The View3D's scene graph is embedded into the main scene graph,
    and the same ordering semantics are applied as to any other Qt Quick \l Item. As this
    mode can lead to subtle issues, depending on the contents of the scene, due to
    injecting depth-based 3D content into a 2D scene graph, it is not recommended to be
    used, unless a specific need arises.

    The default is \c{View3D.Offscreen}.

    \note When changing the render mode, it is important to note that \c{View3D.Offscreen} (the
    default) is the only mode which guarantees perfect graphics fidelity. The other modes
    all have limitations that can cause visual glitches, so it is important to check that
    the visual output still looks correct when changing this property.

    \note When using the Underlay, Overlay, or Inline modes, it can be useful, and in some
    cases, required, to disable the Qt Quick scene graph's depth buffer writes via
    QQuickGraphicsConfiguration::setDepthBufferFor2D() before showing the QQuickWindow or
    QQuickView hosting the View3D item.
*/
QQuick3DViewport::RenderMode QQuick3DViewport::renderMode() const
{
    return m_renderMode;
}

/*!
    \qmlproperty enumeration QtQuick3D::View3D::renderFormat
    \since 6.4

    This property determines the backing texture's format. Applicable only when
    the View3D is rendering to a texture, for example because the \l renderMode
    is \c{View3D.Offscreen}.

    The default is \c{ShaderEffectSource.RGBA8}.

    If the format is not supported by the underlying graphics driver at run
    time, RGBA8 is used.

    \list
    \li ShaderEffectSource.RGBA8
    \li ShaderEffectSource.RGBA16F
    \li ShaderEffectSource.RGBA32F
    \endlist

    \sa QtQuick::ShaderEffectSource::format, QtQuick::Item::layer.format
 */
#if QT_CONFIG(quick_shadereffect)
QQuickShaderEffectSource::Format QQuick3DViewport::renderFormat() const
{
    return m_renderFormat;
}
#endif

/*!
    \qmlproperty QtQuick3D::RenderStats QtQuick3D::View3D::renderStats
    \readonly

    This property provides statistics about the rendering of a frame, such as
    \l {RenderStats::fps}{fps}, \l {RenderStats::frameTime}{frameTime},
    \l {RenderStats::renderTime}{renderTime}, \l {RenderStats::syncTime}{syncTime},
    and \l {RenderStats::maxFrameTime}{maxFrameTime}.
*/
QQuick3DRenderStats *QQuick3DViewport::renderStats() const
{
    return m_renderStats;
}

QQuick3DSceneRenderer *QQuick3DViewport::createRenderer() const
{
    QQuick3DSceneRenderer *renderer = nullptr;

    if (QQuickWindow *qw = window()) {
        auto wa = QQuick3DSceneManager::getOrSetWindowAttachment(*qw);
        auto rci = wa->rci();
        if (!rci) {
            QSGRendererInterface *rif = qw->rendererInterface();
            if (QSSG_GUARD(QSGRendererInterface::isApiRhiBased(rif->graphicsApi()))) {
                QRhi *rhi = static_cast<QRhi *>(rif->getResource(qw, QSGRendererInterface::RhiResource));
                QSSG_CHECK_X(rhi != nullptr, "No QRhi from QQuickWindow, this cannot happen");
                // The RenderContextInterface, and the objects owned by it (such
                // as, the BufferManager) are always per-QQuickWindow, and so per
                // scenegraph render thread. Hence the association with window.
                // Multiple View3Ds in the same window can use the same rendering
                // infrastructure (so e.g. the same QSSGBufferManager), but two
                // View3D objects in different windows must not, except for certain
                // components that do not work with and own native graphics
                // resources (most notably, QSSGShaderLibraryManager - but this
                // distinction is handled internally by QSSGRenderContextInterface).
                rci = std::make_shared<QSSGRenderContextInterface>(rhi);
                wa->setRci(rci);

                // Use DirectConnection to stay on the render thread, if there is one.
                connect(wa, &QQuick3DWindowAttachment::releaseCachedResources, this,
                        &QQuick3DViewport::onReleaseCachedResources, Qt::DirectConnection);

            } else {
                qWarning("The Qt Quick scene is using a rendering method that is not based on QRhi and a 3D graphics API. "
                         "Qt Quick 3D is not functional in such an environment. The View3D item is not going to display anything.");
            }
        }

        if (rci) {
            renderer = new QQuick3DSceneRenderer(rci);
            Q_QUICK3D_PROFILE_ASSIGN_ID(this, renderer);
        }
    }

    return renderer;
}

bool QQuick3DViewport::isTextureProvider() const
{
    // We can only be a texture provider if we are rendering to a texture first
    if (m_renderMode == QQuick3DViewport::Offscreen)
        return true;

    return false;
}

QSGTextureProvider *QQuick3DViewport::textureProvider() const
{
    // When Item::layer::enabled == true, QQuickItem will be a texture
    // provider. In this case we should prefer to return the layer rather
    // than the fbo texture.
    if (QQuickItem::isTextureProvider())
        return QQuickItem::textureProvider();

    // We can only be a texture provider if we are rendering to a texture first
    if (m_renderMode != QQuick3DViewport::Offscreen)
        return nullptr;

    QQuickWindow *w = window();
    if (!w) {
        qWarning("QSSGView3D::textureProvider: can only be queried on the rendering thread of an exposed window");
        return nullptr;
    }

    if (!m_node)
        m_node = new SGFramebufferObjectNode;
    return m_node;
}

class CleanupJob : public QRunnable
{
public:
    CleanupJob(QQuick3DSGDirectRenderer *renderer) : m_renderer(renderer) { }
    void run() override { delete m_renderer; }
private:
    QQuick3DSGDirectRenderer *m_renderer;
};

void QQuick3DViewport::releaseResources()
{
    if (m_directRenderer) {
        window()->scheduleRenderJob(new CleanupJob(m_directRenderer), QQuickWindow::BeforeSynchronizingStage);
        m_directRenderer = nullptr;
    }

    m_node = nullptr;
}

void QQuick3DViewport::cleanupDirectRenderer()
{
    delete m_directRenderer;
    m_directRenderer = nullptr;
}

void QQuick3DViewport::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    if (newGeometry.size() != oldGeometry.size())
        update();
}

QSGNode *QQuick3DViewport::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    // When changing render modes
    if (m_renderModeDirty) {
        if (node) {
            delete node;
            node = nullptr;
            m_node = nullptr;
            m_renderNode = nullptr;
        }
        if (m_directRenderer) {
            delete m_directRenderer;
            m_directRenderer = nullptr;
        }
    }

    m_renderModeDirty = false;

    switch (m_renderMode) {
    // Direct rendering
    case Underlay:
        Q_FALLTHROUGH();
    case Overlay:
        setupDirectRenderer(m_renderMode);
        node = nullptr;
        break;
    case Offscreen:
        node = setupOffscreenRenderer(node);
        break;
    case Inline:
        // QSGRenderNode-based rendering
        node = setupInlineRenderer(node);
        break;
    }

    if (!isforceInputHandlingSet()) {
        // Implicitly enable internal input processing if any item2ds are present.
        const auto inputHandlingEnabled =
                QQuick3DObjectPrivate::get(m_sceneRoot)->sceneManager->inputHandlingEnabled;
        const auto enable = inputHandlingEnabled > 0;
        if (m_enableInputProcessing != enable) {
            m_enableInputProcessing = enable;
            QMetaObject::invokeMethod(this, "updateInputProcessing", Qt::QueuedConnection);
        }
    }

    return node;
}

void QQuick3DViewport::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    if (change == ItemSceneChange) {
        if (value.window) {
            // TODO: if we want to support multiple windows, there has to be a scene manager for
            // every window.
            QQuick3DObjectPrivate::get(m_sceneRoot)->sceneManager->setWindow(value.window);
            if (m_importScene)
                QQuick3DObjectPrivate::get(m_importScene)->sceneManager->setWindow(value.window);
            m_renderStats->setWindow(value.window);
        }
    } else if (change == ItemVisibleHasChanged && isVisible()) {
        update();
    }
}

bool QQuick3DViewport::event(QEvent *event)
{
    if (m_enableInputProcessing && event->isPointerEvent())
        return internalPick(static_cast<QPointerEvent *>(event));
    else
        return QQuickItem::event(event);
}

void QQuick3DViewport::componentComplete()
{
    QQuickItem::componentComplete();
    Q_QUICK3D_PROFILE_REGISTER(this);
}

void QQuick3DViewport::setCamera(QQuick3DCamera *camera)
{
    if (m_camera == camera)
        return;

    if (camera && !camera->parentItem())
        camera->setParentItem(m_sceneRoot);
    if (camera)
        camera->updateGlobalVariables(QRect(0, 0, width(), height()));

    QQuick3DObjectPrivate::attachWatcherPriv(m_sceneRoot, this, &QQuick3DViewport::setCamera, camera, m_camera);

    m_camera = camera;
    emit cameraChanged();
    update();
}

void QQuick3DViewport::setMultiViewCameras(QQuick3DCamera **firstCamera, int count)
{
    m_multiViewCameras.clear();
    bool sendChangeSignal = false;
    for (int i = 0; i < count; ++i) {
        QQuick3DCamera *camera = *(firstCamera + i);
        if (camera) {
            if (!camera->parentItem())
                camera->setParentItem(m_sceneRoot);
            camera->updateGlobalVariables(QRect(0, 0, width(), height()));
        }
        if (i == 0) {
            if (m_camera != camera) {
                m_camera = camera;
                sendChangeSignal = true;
            }
        }

        m_multiViewCameras.append(camera);

        // ### do we need attachWatcher stuff? the Xr-provided cameras cannot disappear, although the XrActor (the owner) might
    }

    if (sendChangeSignal)
        emit cameraChanged();

    update();
}

void QQuick3DViewport::setEnvironment(QQuick3DSceneEnvironment *environment)
{
    if (m_environment == environment)
        return;

    m_environment = environment;
    if (m_environment && !m_environment->parentItem())
        m_environment->setParentItem(m_sceneRoot);

    QQuick3DObjectPrivate::attachWatcherPriv(m_sceneRoot, this, &QQuick3DViewport::setEnvironment, environment, m_environment);

    emit environmentChanged();
    update();
}

void QQuick3DViewport::setImportScene(QQuick3DNode *inScene)
{
    // ### We may need consider the case where there is
    // already a scene tree here
    // FIXME : Only the first importScene is an effective one
    if (m_importScene)
        return;

    // FIXME : Check self-import or cross-import
    // Currently it does not work since importScene qml parsed in a reverse order.
    QQuick3DNode *scene = inScene;
    while (scene) {
        if (m_sceneRoot == scene) {
            qmlWarning(this) << "Cannot allow self-import or cross-import!";
            return;
        }

        QQuick3DSceneRootNode *rn = qobject_cast<QQuick3DSceneRootNode *>(scene);
        scene = rn ? rn->view3D()->importScene() : nullptr;
    }

    m_importScene = inScene;
    if (m_importScene)
        updateSceneManagerForImportScene();

    emit importSceneChanged();
    update();
}

void QQuick3DViewport::setRenderMode(QQuick3DViewport::RenderMode renderMode)
{
    if (m_renderMode == renderMode)
        return;

    m_renderMode = renderMode;
    m_renderModeDirty = true;
    emit renderModeChanged();
    update();
}

#if QT_CONFIG(quick_shadereffect)
void QQuick3DViewport::setRenderFormat(QQuickShaderEffectSource::Format format)
{
    if (m_renderFormat == format)
        return;

    m_renderFormat = format;
    m_renderModeDirty = true;
    emit renderFormatChanged();
    update();
}
#endif

/*!
    \qmlproperty int QtQuick3D::View3D::explicitTextureWidth
    \since 6.7

    The width, in pixels, of the item's associated texture. Relevant when a
    fixed texture size is desired that does not depend on the item's size. This
    size has no effect on the geometry of the item (its size and placement
    within the scene), which means the texture's content will appear scaled up
    or down (and possibly stretched) onto the item's area.

    By default the value is \c 0. A value of 0 means that texture's size
    follows the item's size. (\c{texture size in pixels} = \c{item's logical
    size} * \c{device pixel ratio}).

    \note This property is relevant only when \l renderMode is set to \c
    Offscreen. Its value is ignored otherwise.

    \sa explicitTextureHeight, effectiveTextureSize, DebugView
*/
int QQuick3DViewport::explicitTextureWidth() const
{
    return m_explicitTextureWidth;
}

void QQuick3DViewport::setExplicitTextureWidth(int width)
{
    if (m_explicitTextureWidth == width)
        return;

    m_explicitTextureWidth = width;
    emit explicitTextureWidthChanged();
    update();
}

/*!
    \qmlproperty int QtQuick3D::View3D::explicitTextureHeight
    \since 6.7

    The height, in pixels, of the item's associated texture. Relevant when a
    fixed texture size is desired that does not depend on the item's size. This
    size has no effect on the geometry of the item (its size and placement
    within the scene), which means the texture's content will appear scaled up
    or down (and possibly stretched) onto the item's area.

    By default the value is \c 0. A value of 0 means that texture's size
    follows the item's size. (\c{texture size in pixels} = \c{item's logical
    size} * \c{device pixel ratio}).

    \note This property is relevant only when \l renderMode is set to \c
    Offscreen. Its value is ignored otherwise.

    \sa explicitTextureWidth, effectiveTextureSize, DebugView
*/
int QQuick3DViewport::explicitTextureHeight() const
{
    return m_explicitTextureHeight;
}

void QQuick3DViewport::setExplicitTextureHeight(int height)
{
    if (m_explicitTextureHeight == height)
        return;

    m_explicitTextureHeight = height;
    emit explicitTextureHeightChanged();
    update();
}

/*!
    \qmlproperty size QtQuick3D::View3D::effectiveTextureSize
    \since 6.7

    This property exposes the size, in pixels, of the underlying color (and
    depth/stencil) buffers. It is provided for use on the GUI (main) thread, in
    QML bindings or JavaScript.

    This is a read-only property.

    \note This property is relevant only when \l renderMode is set to
    \c Offscreen.

    \sa explicitTextureWidth, explicitTextureHeight, DebugView
*/
QSize QQuick3DViewport::effectiveTextureSize() const
{
    return m_effectiveTextureSize;
}


/*!
    \qmlmethod vector3d View3D::mapFrom3DScene(vector3d scenePos)

    Transforms \a scenePos from scene space (3D) into view space (2D).

    The returned x- and y-values will be be in view coordinates, with the top-left
    corner at [0, 0] and the bottom-right corner at [width, height]. The returned
    z-value contains the distance from the near clip plane of the frustum (clipNear) to
    \a scenePos in scene coordinates. If the distance is negative, that means the \a scenePos
    is behind the active camera. If \a scenePos cannot be mapped to a position in the scene,
    a position of [0, 0, 0] is returned.

    This function requires that \l camera is assigned to the view.

    \sa mapTo3DScene(), {Camera::mapToViewport()}{Camera.mapToViewport()}
*/
QVector3D QQuick3DViewport::mapFrom3DScene(const QVector3D &scenePos) const
{
    if (!m_camera) {
        qmlWarning(this) << "Cannot resolve view position without a camera assigned!";
        return QVector3D(0, 0, 0);
    }

    qreal _width = width();
    qreal _height = height();
    if (_width == 0 || _height == 0)
        return QVector3D(0, 0, 0);

    const QVector3D normalizedPos = m_camera->mapToViewport(scenePos, _width, _height);
    return normalizedPos * QVector3D(float(_width), float(_height), 1);
}

/*!
    \qmlmethod vector3d View3D::mapTo3DScene(vector3d viewPos)

    Transforms \a viewPos from view space (2D) into scene space (3D).

    The x- and y-values of \a viewPos should be in view coordinates, with the top-left
    corner at [0, 0] and the bottom-right corner at [width, height]. The z-value is
    interpreted as the distance from the near clip plane of the frustum (clipNear) in
    scene coordinates.

    If \a viewPos cannot successfully be mapped to a position in the scene, a position of
    [0, 0, 0] is returned.

    This function requires that a \l camera is assigned to the view.

    \sa mapFrom3DScene(), {Camera::mapFromViewport}{Camera.mapFromViewport()}
*/
QVector3D QQuick3DViewport::mapTo3DScene(const QVector3D &viewPos) const
{
    if (!m_camera) {
        qmlWarning(this) << "Cannot resolve scene position without a camera assigned!";
        return QVector3D(0, 0, 0);
    }

    qreal _width = width();
    qreal _height = height();
    if (_width == 0 || _height == 0)
        return QVector3D(0, 0, 0);

    const QVector3D normalizedPos = viewPos / QVector3D(float(_width), float(_height), 1);
    return m_camera->mapFromViewport(normalizedPos, _width, _height);
}

/*!
    \qmlmethod pickResult View3D::pick(float x, float y)

    This method will "shoot" a ray into the scene from view coordinates \a x and \a y
    and return information about the nearest intersection with an object in the scene.

    This can, for instance, be called with mouse coordinates to find the object under the mouse cursor.

    \sa pickAll(), pickSubset(), pickInRect()
*/
QQuick3DPickResult QQuick3DViewport::pick(float x, float y) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return QQuick3DPickResult();

    const QPointF position(qreal(x) * window()->effectiveDevicePixelRatio() * m_widthMultiplier,
                           qreal(y) * window()->effectiveDevicePixelRatio() * m_heightMultiplier);
    std::optional<QSSGRenderRay> rayResult = renderer->getRayFromViewportPos(position);
    if (!rayResult.has_value())
        return QQuick3DPickResult();

    const auto resultList = renderer->syncPick(rayResult.value());
    return getNearestPickResult(resultList);
}

/*!
    \qmlmethod pickResult View3D::pick(float x, float y, Model model)

    This method will "shoot" a ray into the scene from view coordinates \a x and \a y
    and return information about the intersection between the ray and the specified \a model.

    This can, for instance, be called with mouse coordinates to find the object under the mouse cursor.

    \since 6.8

    \sa pickAll(), pickSubset(), pickInRect()
*/
QQuick3DPickResult QQuick3DViewport::pick(float x, float y, QQuick3DModel *model) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return QQuick3DPickResult();

    const QPointF position(qreal(x) * window()->effectiveDevicePixelRatio(),
                           qreal(y) * window()->effectiveDevicePixelRatio());
    std::optional<QSSGRenderRay> rayResult = renderer->getRayFromViewportPos(position);

    if (!rayResult.has_value())
        return QQuick3DPickResult();

    const auto renderNode = static_cast<QSSGRenderNode *>(QQuick3DObjectPrivate::get(model)->spatialNode);
    const auto resultList = renderer->syncPickOne(rayResult.value(), renderNode);
    return getNearestPickResult(resultList);
}

/*!
    \qmlmethod List<pickResult> View3D::pickSubset(float x, float y, list<Model> models)

    This method will "shoot" a ray into the scene from view coordinates \a x and \a y
    and return information about the intersections with the passed in list of \a models.
    This will only check against the list of models passed in.
    The returned list is sorted by distance from the camera with the nearest
    intersections appearing first and the furthest appearing last.

    This can, for instance, be called with mouse coordinates to find the object under the mouse cursor.

    Works with both property list<Model> and dynamic JavaScript arrays of models.

    \since 6.8

    \sa pick(), pickAll(), pickInRect()
*/
QList<QQuick3DPickResult> QQuick3DViewport::pickSubset(float x, float y, const QJSValue &models) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return {};

    QVarLengthArray<QSSGRenderNode*> renderNodes;
    // Check for regular JavaScript array
    if (models.isArray()) {
        const auto length = models.property(QStringLiteral("length")).toInt();
        if (length == 0)
            return {};

        for (int i = 0; i < length; ++i) {
            const auto isQObject = models.property(i).isQObject();
            if (!isQObject) {
                qmlWarning(this) << "Type provided for picking is not a QObject. Needs to be of type QQuick3DModel.";
                continue;
            }
            const auto obj = models.property(i).toQObject();
            const auto model = qobject_cast<QQuick3DModel *>(obj);
            if (!model) {
                qmlWarning(this) << "Type " << obj->metaObject()->className() << " is not supported for picking. Needs to be of type QQuick3DModel.";
                continue;
            }
            const auto priv = QQuick3DObjectPrivate::get(model);
            if (priv && priv->spatialNode) {
                renderNodes.push_back(static_cast<QSSGRenderNode*>(priv->spatialNode));
            }
        }
    } else {
        // Check for property list<Model>
        const auto subsetVariant = models.toVariant();
        if (!subsetVariant.isValid() || !subsetVariant.canConvert<QQmlListReference>())
            return {};

        const auto list = subsetVariant.value<QQmlListReference>();

        // Only support array of models
        if (list.listElementType()->className() != QQuick3DModel::staticMetaObject.className()) {
            qmlWarning(this) << "Type " << list.listElementType()->className() << " is not supported for picking. Needs to be of type QQuick3DModel.";
            return {};
        }
        for (int i = 0; i < list.count(); ++i) {
            auto model = static_cast<QQuick3DModel *>(list.at(i));
            if (!model)
                continue;
            auto priv = QQuick3DObjectPrivate::get(model);
            if (priv && priv->spatialNode) {
                renderNodes.push_back(static_cast<QSSGRenderNode*>(priv->spatialNode));
            }
        }
    }

    if (renderNodes.empty())
        return {};

    const QPointF position(qreal(x) * window()->effectiveDevicePixelRatio(),
                           qreal(y) * window()->effectiveDevicePixelRatio());
    std::optional<QSSGRenderRay> rayResult = renderer->getRayFromViewportPos(position);
    if (!rayResult.has_value())
        return {};

    const auto resultList = renderer->syncPickSubset(rayResult.value(), renderNodes);

    QList<QQuick3DPickResult> processedResultList;
    processedResultList.reserve(resultList.size());
    for (const auto &result : resultList)
        processedResultList.append(processPickResult(result));

    return processedResultList;
}

/*!
    \qmlmethod List<pickResult> View3D::pickAll(float x, float y)

    This method will "shoot" a ray into the scene from view coordinates \a x and \a y
    and return a list of information about intersections with objects in the scene.
    The returned list is sorted by distance from the camera with the nearest
    intersections appearing first and the furthest appearing last.

    This can, for instance, be called with mouse coordinates to find the object under the mouse cursor.

    \since 6.2

    \sa pick(), pickSubset(), pickInRect()
*/
QList<QQuick3DPickResult> QQuick3DViewport::pickAll(float x, float y) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return QList<QQuick3DPickResult>();

    const QPointF position(qreal(x) * window()->effectiveDevicePixelRatio() * m_widthMultiplier,
                           qreal(y) * window()->effectiveDevicePixelRatio() * m_heightMultiplier);
    std::optional<QSSGRenderRay> rayResult = renderer->getRayFromViewportPos(position);
    if (!rayResult.has_value())
        return QList<QQuick3DPickResult>();

    const auto resultList = renderer->syncPickAll(rayResult.value());
    QList<QQuick3DPickResult> processedResultList;
    processedResultList.reserve(resultList.size());
    for (const auto &result : resultList)
        processedResultList.append(processPickResult(result));

    return processedResultList;
}

/*!
    \qmlmethod pickResult View3D::rayPick(vector3d origin, vector3d direction)

    This method will "shoot" a ray into the scene starting at \a origin and in
    \a direction and return information about the nearest intersection with an
    object in the scene.

    This can, for instance, be called with the position and forward vector of
    any object in a scene to see what object is in front of an item. This
    makes it possible to do picking from any point in the scene.

    \since 6.2
*/
QQuick3DPickResult QQuick3DViewport::rayPick(const QVector3D &origin, const QVector3D &direction) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return QQuick3DPickResult();

    const QSSGRenderRay ray(origin, direction);
    const auto resultList = renderer->syncPick(ray);
    return getNearestPickResult(resultList);
}

/*!
    \qmlmethod List<pickResult> View3D::rayPickAll(vector3d origin, vector3d direction)

    This method will "shoot" a ray into the scene starting at \a origin and in
    \a direction and return a list of information about the nearest intersections with
    objects in the scene.
    The list is presorted by distance from the origin along the direction
    vector with the nearest intersections appearing first and the furthest
    appearing last.

    This can, for instance, be called with the position and forward vector of
    any object in a scene to see what objects are in front of an item. This
    makes it possible to do picking from any point in the scene.

    \since 6.2
*/
QList<QQuick3DPickResult> QQuick3DViewport::rayPickAll(const QVector3D &origin, const QVector3D &direction) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return QList<QQuick3DPickResult>();

    const QSSGRenderRay ray(origin, direction);

    const auto resultList = renderer->syncPickAll(ray);
    QList<QQuick3DPickResult> processedResultList;
    processedResultList.reserve(resultList.size());
    for (const auto &result : resultList) {
        auto processedResult = processPickResult(result);
        if (processedResult.hitType() != QQuick3DPickResultEnums::HitType::Null)
            processedResultList.append(processedResult);
    }

    return processedResultList;
}

/*!
    \qmlmethod pickResult View3D::rayPick(vector3d origin, vector3d direction, Model model)

    This method will "shoot" a ray into the scene starting at \a origin and in
    \a direction and return information about the intersection between the ray and the specified \a model.

    \since 6.11
*/
QQuick3DPickResult QQuick3DViewport::rayPick(const QVector3D &origin, const QVector3D &direction, QQuick3DModel *model) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return QQuick3DPickResult();

    const QSSGRenderRay ray(origin, direction);

    const auto renderNode = static_cast<QSSGRenderNode *>(QQuick3DObjectPrivate::get(model)->spatialNode);
    const auto resultList = renderer->syncPickOne(ray, renderNode);
    return getNearestPickResult(resultList);
}

/*!
    \qmlmethod pickResult View3D::closestPointPick(vector3d origin, float radius, Model model)

    This method will find the point on the surface of \a model that is nearest to \a origin, within a distance of
    \a radius. If \a model is \c null, the closest object within \a radius will be found.

    If no such object exists, \c null is returned.

     \since 6.11
*/
QQuick3DPickResult QQuick3DViewport::closestPointPick(const QVector3D &origin, float radius, QQuick3DModel *model) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();

    if (!renderer)
        return QQuick3DPickResult();
    QSSGRenderNode *renderNode = nullptr;
    if (model) {
        renderNode = static_cast<QSSGRenderNode *>(QQuick3DObjectPrivate::get(model)->spatialNode);
        if (Q_UNLIKELY(!renderNode))
            return QQuick3DPickResult{};
    }

    const auto pickResult = renderer->syncPickClosestPoint(origin, radius * radius, renderNode);
    if (!pickResult.has_value())
        return QQuick3DPickResult{};
    return processPickResult(pickResult.value());
}

/*!
    \qmlmethod list<Object3D> View3D::pickInRect(point start, point end)
    \since 6.11

    This method picks all objects within a rectangular region defined by
    \a start and \a end points in view coordinates.

    Returns a list of objects that are within the specified rectangle. This
    can be used for marquee selection, where the user drags a rectangle to
    select multiple objects.

    \sa pick(), pickAll(), pickSubset()
*/
QList<QQuick3DObject *> QQuick3DViewport::pickInRect(const QPointF &start, const QPointF &end) const
{
    const qreal minX = qMin(start.x(), end.x());
    const qreal maxX = qMax(start.x(), end.x());
    const qreal minY = qMin(start.y(), end.y());
    const qreal maxY = qMax(start.y(), end.y());

    const qreal ndc[4] = {
        2.0f * minX / width() - 1.0f, // left
        2.0f * maxX / width() - 1.0f, // right
        // flip Y-coordinates
        1.0f - 2.0f * maxY / height(), // bottom
        1.0f - 2.0f * minY / height(), // top
    };

    const float near = 0.0f;
    const float far = 1.0f;
    enum { L, R, B, T }; // left, right, bottom, top
    const QVector4D ndcCorners[8] = {
        // Near plane
        QVector4D(ndc[L], ndc[B], near, 1.0f), // 0: bottom-left  | n0
        QVector4D(ndc[R], ndc[B], near, 1.0f), // 1: bottom-right | n1
        QVector4D(ndc[L], ndc[T], near, 1.0f), // 3: top-left     | n2
        QVector4D(ndc[R], ndc[T], near, 1.0f), // 2: top-right    | n3
        // Far plane
        QVector4D(ndc[L], ndc[B], far, 1.0f), // 4: bottom-left   | f0
        QVector4D(ndc[R], ndc[B], far, 1.0f), // 5: bottom-right  | f1
        QVector4D(ndc[L], ndc[T], far, 1.0f), // 7: top-left      | f2
        QVector4D(ndc[R], ndc[T], far, 1.0f), // 6: top-right     | f3
    };

    QMatrix4x4 viewProjection;
    if (this->camera()) {
        if (auto camera = static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::get(this->camera())->spatialNode))
            camera->calculateViewProjectionMatrix(camera->localTransform, camera->projection, viewProjection);
    }
    QMatrix4x4 viewProjectionInverted = viewProjection.inverted();

    QVector3D worldCorners[8];
    for (int i = 0; i < 8; ++i) {
        QVector4D worldPoint = viewProjectionInverted * ndcCorners[i];
        worldCorners[i] = worldPoint.toVector3D() / worldPoint.w(); // perspective divide
    }

    // Frustum
    //
    //    f2 ------------f3
    //     |             |
    //     |             |
    //     |             |
    //     |             |
    //    f0 ------------f1
    //
    // n2 -------- n3
    // |           |
    // |   camera  |
    // |           |
    // n0 -------- n1
    //
    // CCW winding if inside the frustum and looking toward the face
    const QSSGFrustum frustum {
        QSSGPlane(worldCorners[0], worldCorners[4], worldCorners[6]), // L [n0, f0, f2]
        QSSGPlane(worldCorners[1], worldCorners[3], worldCorners[7]), // R [n1, n3, f3]
        QSSGPlane(worldCorners[0], worldCorners[1], worldCorners[5]), // B [n0, n1, f1]
        QSSGPlane(worldCorners[2], worldCorners[6], worldCorners[7]), // T [n2, f2, f3]
        QSSGPlane(worldCorners[0], worldCorners[2], worldCorners[3]), // N [n0, n2, n3]
        QSSGPlane(worldCorners[5], worldCorners[7], worldCorners[6]), // F [f1, f3, f2]
    };

    QList<QQuick3DObject *> ret;
    if (QQuick3DSceneRenderer *renderer = getRenderer()) {
        auto nodes = renderer->syncPickInFrustum(frustum);
        for (auto node : nodes) {
            if (QQuick3DObject *m = findFrontendNode(node))
                ret.append(m);
        }
    }

    return ret;
}

void QQuick3DViewport::processPointerEventFromRay(const QVector3D &origin, const QVector3D &direction, QPointerEvent *event) const
{
    internalPick(event, origin, direction);
}

// Note: we have enough information to implement Capability::Hover and Capability::ZPosition,
// but those properties are not currently available in QTouchEvent/QEventPoint

namespace {
class SyntheticTouchDevice : public QPointingDevice
{
public:
    SyntheticTouchDevice(QObject *parent = nullptr)
        : QPointingDevice(QLatin1StringView("QtQuick3D Touch Synthesizer"),
                          0,
                          DeviceType::TouchScreen,
                          PointerType::Finger,
                          Capability::Position,
                          10, 0,
                          QString(), QPointingDeviceUniqueId(),
                          parent)
    {
    }
};
}

/*!
    \qmlmethod void View3D::setTouchpoint(Item target, point position, int pointId, bool pressed)

    Sends a synthetic touch event to \a target, moving the touch point with ID \a pointId to \a position,
    with \a pressed determining if the point is pressed.
    Also sends the appropriate touch release event if \a pointId was previously active on a different
    item.

    \since 6.8
 */

void QQuick3DViewport::setTouchpoint(QQuickItem *target, const QPointF &position, int pointId, bool pressed)
{
    if (pointId >= m_touchState.size())
        m_touchState.resize(pointId + 1);
    auto prevState = m_touchState[pointId];

    const bool sameTarget = prevState.target == target;
    const bool wasPressed = prevState.isPressed;

    const bool isPress = pressed && (!sameTarget || !wasPressed);
    const bool isRelease = !pressed && wasPressed && sameTarget;

    // Hover if we're not active, and we weren't previously active.
    // We assume that we always get a non-active for a target when we release.
    // This function sends a release events if the target is changed.
    if (!sameTarget && wasPressed)
        qWarning("QQuick3DViewport::setTouchpoint missing release event");

    if (!pressed && !wasPressed) {
        // This would be a hover event: skipping
        return;
    }

    m_touchState[pointId] = { target, position, pressed };

    if (!m_syntheticTouchDevice)
        m_syntheticTouchDevice = new SyntheticTouchDevice(this);

    QPointingDevicePrivate *devPriv = QPointingDevicePrivate::get(m_syntheticTouchDevice);

    auto makePoint = [devPriv](int id, QEventPoint::State pointState, QPointF pos, quint64 timestamp) -> QEventPoint {
        auto epd = devPriv->pointById(id);
        auto &ep = epd->eventPoint;
        if (pointState != QEventPoint::State::Stationary)
            ep.setAccepted(false);

        auto res = QMutableEventPoint::withTimeStamp(timestamp, id, pointState, pos, pos, pos);
        QMutableEventPoint::update(res, ep);

        if (pointState == QEventPoint::State::Pressed)
            QMutableEventPoint::setGlobalPressPosition(res, pos);
        else if (ep.state() != QEventPoint::State::Unknown)
            QMutableEventPoint::setGlobalPressPosition(res, ep.globalPressPosition());

        return res;
    };

    auto sendTouchEvent = [&](QQuickItem *t, const QPointF &position, int pointId, QEventPoint::State pointState, quint64 timestamp) -> void {
        QList<QEventPoint> points;
        bool otherPoint = false; // Does the event have another point already?
        for (int i = 0; i < m_touchState.size(); ++i) {
            const auto &ts = m_touchState[i];
            if (ts.target != t)
                continue;
            if (i == pointId) {
                auto newPoint = makePoint(i, pointState, position, timestamp);
                points << newPoint;
            } else if (ts.isPressed) {
                otherPoint = true;
                points << makePoint(i, QEventPoint::Stationary, ts.position, timestamp);
            }
        }

        QEvent::Type type;
        if (pointState == QEventPoint::Pressed && !otherPoint)
            type = QEvent::Type::TouchBegin;
        else if (pointState == QEventPoint::Released && !otherPoint)
            type = QEvent::Type::TouchEnd;
        else
            type = QEvent::Type::TouchUpdate;

        QTouchEvent ev(type, m_syntheticTouchDevice, {}, points);
        ev.setTimestamp(timestamp);

        if (t) {
            // Actually send event:
            auto da = QQuickItemPrivate::get(t)->deliveryAgent();
            bool handled = da->event(&ev);
            Q_UNUSED(handled);
        }

        // Duplicate logic from QQuickWindowPrivate::clearGrabbers
        if (ev.isEndEvent()) {
            for (auto &point : ev.points()) {
                if (point.state() == QEventPoint::State::Released) {
                    ev.setExclusiveGrabber(point, nullptr);
                    ev.clearPassiveGrabbers(point);
                }
            }
        }
    };

    auto timestamp = QDateTime::currentMSecsSinceEpoch();

    // Send a release event to the previous target
    if (prevState.target && !sameTarget)
        sendTouchEvent(prevState.target, prevState.position, pointId, QEventPoint::Released, timestamp);

    // Now send an event for the new state
    QEventPoint::State newState = isPress ? QEventPoint::Pressed : isRelease ? QEventPoint::Released : QEventPoint::Updated;
    sendTouchEvent(target, position, pointId, newState, timestamp);
}

QQuick3DLightmapBaker *QQuick3DViewport::maybeLightmapBaker()
{
    return m_lightmapBaker;
}

QQuick3DLightmapBaker *QQuick3DViewport::lightmapBaker()
{
    if (!m_lightmapBaker)
        m_lightmapBaker= new QQuick3DLightmapBaker(this);

    return m_lightmapBaker;
}

/*!
    \internal
*/
void QQuick3DViewport::bakeLightmap()
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer || !renderer->m_layer->renderData)
        return;

    const bool currentlyBaking = renderer->m_layer->renderData->lightmapBaker != nullptr;

    if (!currentlyBaking)
        lightmapBaker()->bake();
}

/*!
    \internal
*/
void QQuick3DViewport::denoiseLightmap()
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer || !renderer->m_layer->renderData)
        return;

    const bool currentlyBaking = renderer->m_layer->renderData->lightmapBaker != nullptr;

    if (!currentlyBaking)
        lightmapBaker()->denoise();
}


void QQuick3DViewport::setGlobalPickingEnabled(bool isEnabled)
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer)
        return;

    renderer->setGlobalPickingEnabled(isEnabled);
}

void QQuick3DViewport::invalidateSceneGraph()
{
    m_node = nullptr;
}

QQuick3DSceneRenderer *QQuick3DViewport::getRenderer() const
{
    QQuick3DSceneRenderer *renderer = nullptr;
    if (m_node) {
        renderer = m_node->renderer;
    } else if (m_renderNode) {
        renderer = m_renderNode->renderer;
    } else if (m_directRenderer) {
        renderer = m_directRenderer->renderer();
    }
    return renderer;
}

void QQuick3DViewport::updateDynamicTextures()
{
    // Update QSGDynamicTextures that are used for source textures and Quick items
    // Must be called on the render thread.

    const auto &sceneManager = QQuick3DObjectPrivate::get(m_sceneRoot)->sceneManager;
    for (auto *texture : std::as_const(sceneManager->qsgDynamicTextures))
        texture->updateTexture();

    QQuick3DNode *scene = m_importScene;
    while (scene) {
        const auto &importSm = QQuick3DObjectPrivate::get(scene)->sceneManager;
        if (importSm != sceneManager) {
            for (auto *texture : std::as_const(importSm->qsgDynamicTextures))
                texture->updateTexture();
        }

        // if importScene has another import
        QQuick3DSceneRootNode *rn = qobject_cast<QQuick3DSceneRootNode *>(scene);
        scene = rn ? rn->view3D()->importScene() : nullptr;
    }
}

QSGNode *QQuick3DViewport::setupOffscreenRenderer(QSGNode *node)
{
    SGFramebufferObjectNode *n = static_cast<SGFramebufferObjectNode *>(node);

    if (!n) {
        if (!m_node)
            m_node = new SGFramebufferObjectNode;
        n = m_node;
    }

    if (!n->renderer) {
        n->window = window();
        n->renderer = createRenderer();
        if (!n->renderer)
            return nullptr;
        n->renderer->fboNode = n;
        n->quickFbo = this;
        connect(window(), SIGNAL(screenChanged(QScreen*)), n, SLOT(handleScreenChange()));
    }

    const qreal dpr = window()->effectiveDevicePixelRatio();
    const QSize minFboSize = QQuickItemPrivate::get(this)->sceneGraphContext()->minimumFBOSize();
    QSize desiredFboSize = QSize(m_explicitTextureWidth, m_explicitTextureHeight);
    if (desiredFboSize.isEmpty()) {
        desiredFboSize = QSize(width(), height()) * dpr;
        n->devicePixelRatio = dpr;
        // 1:1 mapping between the backing texture and the on-screen quad
        m_widthMultiplier = 1.0f;
        m_heightMultiplier = 1.0f;
    } else {
        QSize itemPixelSize = QSize(width(), height()) * dpr;
        // not 1:1 maping between the backing texture and the on-screen quad
        m_widthMultiplier = desiredFboSize.width() / float(itemPixelSize.width());
        m_heightMultiplier = desiredFboSize.height() / float(itemPixelSize.height());
        n->devicePixelRatio = 1.0;
    }
    desiredFboSize.setWidth(qMax(minFboSize.width(), desiredFboSize.width()));
    desiredFboSize.setHeight(qMax(minFboSize.height(), desiredFboSize.height()));

    if (desiredFboSize != m_effectiveTextureSize) {
        m_effectiveTextureSize = desiredFboSize;
        emit effectiveTextureSizeChanged();
    }

    n->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
    n->setRect(0, 0, width(), height());
    if (checkIsVisible() && isComponentComplete()) {
        n->renderer->synchronize(this, desiredFboSize, n->devicePixelRatio);
        if (n->renderer->m_textureNeedsFlip)
            n->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
        updateDynamicTextures();
        n->scheduleRender();
    }

    return n;
}

QSGNode *QQuick3DViewport::setupInlineRenderer(QSGNode *node)
{
    QQuick3DSGRenderNode *n = static_cast<QQuick3DSGRenderNode *>(node);
    if (!n) {
        if (!m_renderNode)
            m_renderNode = new QQuick3DSGRenderNode;
        n = m_renderNode;
    }

    if (!n->renderer) {
        n->window = window();
        n->renderer = createRenderer();
        if (!n->renderer)
            return nullptr;
    }

    if (!m_effectiveTextureSize.isEmpty()) {
        m_effectiveTextureSize = QSize();
        emit effectiveTextureSizeChanged();
    }

    const QSize targetSize = window()->effectiveDevicePixelRatio() * QSize(width(), height());

    // checkIsVisible, not isVisible, because, for example, a
    // { visible: false; layer.enabled: true } item still needs
    // to function normally.
    if (checkIsVisible() && isComponentComplete()) {
        n->renderer->synchronize(this, targetSize, window()->effectiveDevicePixelRatio());
        updateDynamicTextures();
        n->markDirty(QSGNode::DirtyMaterial);
    }

    return n;
}


void QQuick3DViewport::setupDirectRenderer(RenderMode mode)
{
    auto renderMode = (mode == Underlay) ? QQuick3DSGDirectRenderer::Underlay
                                         : QQuick3DSGDirectRenderer::Overlay;
    if (!m_directRenderer) {
        QQuick3DSceneRenderer *sceneRenderer = createRenderer();
        if (!sceneRenderer)
            return;
        m_directRenderer = new QQuick3DSGDirectRenderer(sceneRenderer, window(), renderMode);
        connect(window(), &QQuickWindow::sceneGraphInvalidated, this, &QQuick3DViewport::cleanupDirectRenderer, Qt::DirectConnection);
    }

    if (!m_effectiveTextureSize.isEmpty()) {
        m_effectiveTextureSize = QSize();
        emit effectiveTextureSizeChanged();
    }

    const QSizeF targetSize = window()->effectiveDevicePixelRatio() * QSizeF(width(), height());
    m_directRenderer->setViewport(QRectF(window()->effectiveDevicePixelRatio() * mapToScene(QPointF(0, 0)), targetSize));
    m_directRenderer->setVisibility(isVisible());
    if (isVisible()) {
        m_directRenderer->preSynchronize();
        m_directRenderer->renderer()->synchronize(this, targetSize.toSize(), window()->effectiveDevicePixelRatio());
        updateDynamicTextures();
        m_directRenderer->requestRender();
    }
}

// This is used for offscreen mode since we need to check if
// this item is used by an effect but hidden
bool QQuick3DViewport::checkIsVisible() const
{
    auto childPrivate = QQuickItemPrivate::get(this);
    return (childPrivate->explicitVisible ||
            (childPrivate->extra.isAllocated() && childPrivate->extra->effectRefCount));

}

/*!
    \internal

    This method processes a pickResult on an object with the objective of checking
    if there are any QQuickItem based subscenes that the QPointerEvent needs to be
    forwarded to (3D -> 2D). If such a subscene is found, the event will be mapped
    to the correct cordinate system, and added to the visitedSubscenes map for later
    event delivery.
*/
void QQuick3DViewport::processPickedObject(const QSSGRenderPickResult &pickResult,
                                           int pointIndex,
                                           QPointerEvent *event,
                                           QFlatMap<QQuickItem *, SubsceneInfo> &visitedSubscenes) const
{
    QQuickItem *subsceneRootItem = nullptr;
    QPointF subscenePosition;
    const auto backendObject = pickResult.m_hitObject;
    const auto frontendObject = findFrontendNode(backendObject);
    if (!frontendObject)
        return;

    // Figure out if there are any QQuickItem based scenes we need to forward
    // the event to, and if the are found, determine how to translate the UV Coords
    // in the pickResult based on what type the object containing the scene is.
    auto frontendObjectPrivate = QQuick3DObjectPrivate::get(frontendObject);
    if (frontendObjectPrivate->type == QQuick3DObjectPrivate::Type::Item2D) {
        // Item2D, this is the case where there is just an embedded Qt Quick 2D Item
        // rendered directly to the scene.
        auto item2D = qobject_cast<QQuick3DItem2D *>(frontendObject);
        if (item2D)
            subsceneRootItem = item2D->contentItem();
        if (!subsceneRootItem || subsceneRootItem->childItems().isEmpty())
            return; // ignore empty 2D subscenes

        // In this case the "UV" coordinates are in pixels in the subscene root item's coordinate system.
        subscenePosition = pickResult.m_localUVCoords.toPointF();

        // The following code will account for custom input masking, as well any
        // transformations that might have been applied to the Item
        if (!subsceneRootItem->childAt(subscenePosition.x(), subscenePosition.y()))
            return;
    } else if (frontendObjectPrivate->type == QQuick3DObjectPrivate::Type::Model) {
        // Model
        int materialSubset = pickResult.m_subset;
        const auto backendModel = static_cast<const QSSGRenderModel *>(backendObject);
        // Get material
        if (backendModel->materials.size() < (pickResult.m_subset + 1))
            materialSubset = backendModel->materials.size() - 1;
        if (materialSubset < 0)
            return;
        const auto backendMaterial = backendModel->materials.at(materialSubset);
        const auto frontendMaterial = static_cast<QQuick3DMaterial*>(findFrontendNode(backendMaterial));
        subsceneRootItem = getSubSceneRootItem(frontendMaterial);

        if (subsceneRootItem) {
            // In this case the pick result really is using UV coordinates.
            subscenePosition = QPointF(subsceneRootItem->x() + pickResult.m_localUVCoords.x() * subsceneRootItem->width(),
                                       subsceneRootItem->y() - pickResult.m_localUVCoords.y() * subsceneRootItem->height() + subsceneRootItem->height());
        }
    }

    //  Add the new event (item and position) to the visitedSubscene map.
    if (subsceneRootItem) {
        SubsceneInfo &subscene = visitedSubscenes[subsceneRootItem]; // create if not found
        subscene.obj = frontendObject;
        if (subscene.eventPointScenePositions.size() != event->pointCount()) {
            // ensure capacity, and use an out-of-scene position rather than 0,0 by default
            constexpr QPointF inf(-qt_inf(), -qt_inf());
            subscene.eventPointScenePositions.resize(event->pointCount(), inf);
        }
        subscene.eventPointScenePositions[pointIndex] = subscenePosition;
    }
}

/*!
    \internal

    This method will try and find a QQuickItem based by looking for sourceItem()
    properties on the primary color channels for the various material types.

    \note for CustomMaterial there is just a best effort given where the first
    associated Texture with a sourceItem property is used.
*/

QQuickItem *QQuick3DViewport::getSubSceneRootItem(QQuick3DMaterial *material) const
{
    if (!material)
        return nullptr;

    QQuickItem *subsceneRootItem = nullptr;
    const auto frontendMaterialPrivate = QQuick3DObjectPrivate::get(material);

    if (frontendMaterialPrivate->type == QQuick3DObjectPrivate::Type::DefaultMaterial) {
        // Default Material
        const auto defaultMaterial = qobject_cast<QQuick3DDefaultMaterial *>(material);
        if (defaultMaterial) {
            // Just check for a diffuseMap for now
            if (defaultMaterial->diffuseMap() && defaultMaterial->diffuseMap()->sourceItem())
                subsceneRootItem = defaultMaterial->diffuseMap()->sourceItem();
        }

    } else if (frontendMaterialPrivate->type == QQuick3DObjectPrivate::Type::PrincipledMaterial) {
        // Principled Material
        const auto principledMaterial = qobject_cast<QQuick3DPrincipledMaterial *>(material);
        if (principledMaterial) {
            // Just check for a baseColorMap for now
            if (principledMaterial->baseColorMap() && principledMaterial->baseColorMap()->sourceItem())
                subsceneRootItem = principledMaterial->baseColorMap()->sourceItem();
        }
    } else if (frontendMaterialPrivate->type == QQuick3DObjectPrivate::Type::SpecularGlossyMaterial) {
        // SpecularGlossy Material
        const auto specularGlossyMaterial = qobject_cast<QQuick3DSpecularGlossyMaterial *>(material);
        if (specularGlossyMaterial) {
            // Just check for a albedoMap for now
            if (specularGlossyMaterial->albedoMap() && specularGlossyMaterial->albedoMap()->sourceItem())
                subsceneRootItem = specularGlossyMaterial->albedoMap()->sourceItem();
        }
    } else if (frontendMaterialPrivate->type == QQuick3DObjectPrivate::Type::CustomMaterial) {
        // Custom Material
        const auto customMaterial = qobject_cast<QQuick3DCustomMaterial *>(material);
        if (customMaterial) {
            // This case is a bit harder because we can not know how the textures will be used
            const auto &texturesInputs = customMaterial->m_dynamicTextureMaps;
            for (const auto &textureInput : texturesInputs) {
                if (auto texture = textureInput->texture()) {
                    if (texture->sourceItem()) {
                        subsceneRootItem = texture->sourceItem();
                        break;
                    }
                }
            }
        }
    }
    return subsceneRootItem;
}


/*!
    \internal
*/
QQuick3DPickResult QQuick3DViewport::getNearestPickResult(const QVarLengthArray<QSSGRenderPickResult, 20> &pickResults) const
{
    for (const auto &result : pickResults) {
        auto pickResult = processPickResult(result);
        if (pickResult.hitType() != QQuick3DPickResultEnums::HitType::Null)
            return pickResult;
    }

    return QQuick3DPickResult();
}

/*!
    \internal
    This method is responsible for going through the visitedSubscenes map and forwarding
    the event with corrected coordinates to each subscene.

*/
bool QQuick3DViewport::forwardEventToSubscenes(QPointerEvent *event,
                                                bool useRayPicking,
                                                QQuick3DSceneRenderer *renderer,
                                                const QFlatMap<QQuickItem *, SubsceneInfo> &visitedSubscenes) const
{
    // Now deliver the entire event (all points) to each relevant subscene.
    // Maybe only some points fall inside, but QQuickDeliveryAgentPrivate::deliverMatchingPointsToItem()
    // makes reduced-subset touch events that contain only the relevant points, when necessary.
    bool ret = false;

    QVarLengthArray<QPointF, 16> originalScenePositions;
    originalScenePositions.resize(event->pointCount());
    for (int pointIndex = 0; pointIndex < event->pointCount(); ++pointIndex)
        originalScenePositions[pointIndex] = event->point(pointIndex).scenePosition();
    for (auto subscene : visitedSubscenes) {
        QQuickItem *subsceneRoot = subscene.first;
        auto &subsceneInfo = subscene.second;
        Q_ASSERT(subsceneInfo.eventPointScenePositions.size() == event->pointCount());
        auto da = QQuickItemPrivate::get(subsceneRoot)->deliveryAgent();
        for (int pointIndex = 0; pointIndex < event->pointCount(); ++pointIndex) {
            const auto &pt = subsceneInfo.eventPointScenePositions.at(pointIndex);
            // By tradition, QGuiApplicationPrivate::processTouchEvent() has set the local position to the scene position,
            // and Qt Quick expects it to arrive that way: then QQuickDeliveryAgentPrivate::translateTouchEvent()
            // copies it into the scene position before localizing.
            // That might be silly, we might change it eventually, but gotta stay consistent for now.
            QEventPoint &ep = event->point(pointIndex);
            QMutableEventPoint::setPosition(ep, pt);
            QMutableEventPoint::setScenePosition(ep, pt);
        }

        if (event->isBeginEvent())
            da->setSceneTransform(nullptr);
        if (da->event(event)) {
            ret = true;
            if (QQuickDeliveryAgentPrivate::anyPointGrabbed(event) && !useRayPicking) {
                // In case any QEventPoint was grabbed, the relevant QQuickDeliveryAgent needs to know
                // how to repeat the picking/coordinate transformation for each update,
                // because delivery will bypass internalPick() due to the grab, and it's
                // more efficient to avoid whole-scene picking each time anyway.
                auto frontendObjectPrivate = QQuick3DObjectPrivate::get(subsceneInfo.obj);
                const bool item2Dcase = (frontendObjectPrivate->type == QQuick3DObjectPrivate::Type::Item2D);
                ViewportTransformHelper *transform = new ViewportTransformHelper;
                transform->viewport = const_cast<QQuick3DViewport *>(this);
                transform->renderer = renderer;
                transform->sceneParentNode = static_cast<QSSGRenderNode*>(frontendObjectPrivate->spatialNode);
                transform->targetItem = subsceneRoot;
                transform->scaleX = window()->effectiveDevicePixelRatio() * m_widthMultiplier;
                transform->scaleY = window()->effectiveDevicePixelRatio() * m_heightMultiplier;
                transform->uvCoordsArePixels = item2Dcase;
                transform->setOnDeliveryAgent(da);
                qCDebug(lcPick) << event->type() << "created ViewportTransformHelper on" << da;
            }
        } else if (event->type() != QEvent::HoverMove) {
            qCDebug(lcPick) << subsceneRoot << "didn't want" << event;
        }
        event->setAccepted(false); // reject implicit grab and let it keep propagating
    }
    if (visitedSubscenes.isEmpty()) {
        event->setAccepted(false);
    } else {
        for (int pointIndex = 0; pointIndex < event->pointCount(); ++pointIndex)
            QMutableEventPoint::setScenePosition(event->point(pointIndex), originalScenePositions.at(pointIndex));
    }

    // Normally this would occur in QQuickWindowPrivate::clearGrabbers(...) but
    // for ray based input, input never goes through QQuickWindow (since events
    // are generated from within scene space and not window/screen space).
    if (event->isEndEvent() && useRayPicking) {
        if (event->isSinglePointEvent()) {
            if (static_cast<QSinglePointEvent *>(event)->buttons() == Qt::NoButton) {
                auto &firstPt = event->point(0);
                event->setExclusiveGrabber(firstPt, nullptr);
                event->clearPassiveGrabbers(firstPt);
            }
        } else {
            for (auto &point : event->points()) {
                if (point.state() == QEventPoint::State::Released) {
                    event->setExclusiveGrabber(point, nullptr);
                    event->clearPassiveGrabbers(point);
                }
            }
        }
    }

    return ret;
}


bool QQuick3DViewport::internalPick(QPointerEvent *event, const QVector3D &origin, const QVector3D &direction) const
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer || !event)
        return false;

    QFlatMap<QQuickItem*, SubsceneInfo> visitedSubscenes;
    const bool useRayPicking = !direction.isNull();

    for (int pointIndex = 0; pointIndex < event->pointCount(); ++pointIndex) {
        auto &eventPoint = event->point(pointIndex);
        QVarLengthArray<QSSGRenderPickResult, 20> pickResults;
        if (Q_UNLIKELY(useRayPicking))
            pickResults = getPickResults(renderer, origin, direction);
        else
            pickResults = getPickResults(renderer, eventPoint);

        if (!pickResults.isEmpty())
            for (const auto &pickResult : pickResults)
                processPickedObject(pickResult, pointIndex, event, visitedSubscenes);
        else
            eventPoint.setAccepted(false); // let it fall through the viewport to Items underneath
    }

    return forwardEventToSubscenes(event, useRayPicking, renderer, visitedSubscenes);
}

bool QQuick3DViewport::singlePointPick(QSinglePointEvent *event, const QVector3D &origin, const QVector3D &direction)
{
    QQuick3DSceneRenderer *renderer = getRenderer();
    if (!renderer || !event)
        return false;

    QSSGRenderRay ray(origin, direction);

    Q_ASSERT(event->pointCount() == 1);
    QPointF originalPosition = event->point(0).scenePosition();

    auto pickResults = renderer->syncPickAll(ray);

    bool delivered = false;

    constexpr float jitterLimit = 2.5; // TODO: add property for this?
    bool withinJitterLimit = false;

    for (const auto &pickResult : pickResults) {
        auto [item, position] = getItemAndPosition(pickResult);
        if (!item)
            break;
        if (item == m_prevMouseItem && (position - m_prevMousePos).manhattanLength() < jitterLimit && !event->button()) {
            withinJitterLimit = true;
            break;
        }
        auto da = QQuickItemPrivate::get(item)->deliveryAgent();
        QEventPoint &ep = event->point(0);
        QMutableEventPoint::setPosition(ep, position);
        QMutableEventPoint::setScenePosition(ep, position);
        if (da->event(event)) {
            delivered = true;
            if (event->type() == QEvent::MouseButtonPress) {
                m_prevMouseItem = item;
                m_prevMousePos = position;
                withinJitterLimit = true;
            }
            break;
        }
    }

    QMutableEventPoint::setScenePosition(event->point(0), originalPosition);
    if (!withinJitterLimit)
        m_prevMouseItem = nullptr;

    // Normally this would occur in QQuickWindowPrivate::clearGrabbers(...) but
    // for ray based input, input never goes through QQuickWindow (since events
    // are generated from within scene space and not window/screen space).
    if (event->isEndEvent()) {
        if (event->buttons() == Qt::NoButton) {
            auto &firstPt = event->point(0);
            event->setExclusiveGrabber(firstPt, nullptr);
            event->clearPassiveGrabbers(firstPt);
        }
    }

    return delivered;
}

QPair<QQuickItem *, QPointF> QQuick3DViewport::getItemAndPosition(const QSSGRenderPickResult &pickResult) const
{
    QQuickItem *subsceneRootItem = nullptr;
    QPointF subscenePosition;
    const auto backendObject = pickResult.m_hitObject;
    const auto frontendObject = findFrontendNode(backendObject);
    if (!frontendObject)
        return {};
    auto frontendObjectPrivate = QQuick3DObjectPrivate::get(frontendObject);
    if (frontendObjectPrivate->type == QQuick3DObjectPrivate::Type::Item2D) {
        // Item2D, this is the case where there is just an embedded Qt Quick 2D Item
        // rendered directly to the scene.
        auto item2D = qobject_cast<QQuick3DItem2D *>(frontendObject);
        if (item2D)
            subsceneRootItem = item2D->contentItem();
        if (!subsceneRootItem || subsceneRootItem->childItems().isEmpty())
            return {}; // ignore empty 2D subscenes

        // In this case the "UV" coordinates are in pixels in the subscene root item's coordinate system.
        subscenePosition = pickResult.m_localUVCoords.toPointF();

        // The following code will account for custom input masking, as well any
        // transformations that might have been applied to the Item
        if (!subsceneRootItem->childAt(subscenePosition.x(), subscenePosition.y()))
            return {};
    } else if (frontendObjectPrivate->type == QQuick3DObjectPrivate::Type::Model) {
        // Model
        int materialSubset = pickResult.m_subset;
        const auto backendModel = static_cast<const QSSGRenderModel *>(backendObject);
        // Get material
        if (backendModel->materials.size() < (pickResult.m_subset + 1))
            materialSubset = backendModel->materials.size() - 1;
        if (materialSubset < 0)
            return {};
        const auto backendMaterial = backendModel->materials.at(materialSubset);
        const auto frontendMaterial = static_cast<QQuick3DMaterial *>(findFrontendNode(backendMaterial));
        subsceneRootItem = getSubSceneRootItem(frontendMaterial);

        if (subsceneRootItem) {
            // In this case the pick result really is using UV coordinates.
            subscenePosition = QPointF(subsceneRootItem->x() + pickResult.m_localUVCoords.x() * subsceneRootItem->width(),
                                       subsceneRootItem->y() - pickResult.m_localUVCoords.y() * subsceneRootItem->height() + subsceneRootItem->height());
        }
    }
    return {subsceneRootItem, subscenePosition};
}

QVarLengthArray<QSSGRenderPickResult, 20> QQuick3DViewport::getPickResults(QQuick3DSceneRenderer *renderer,
                                                                           const QVector3D &origin,
                                                                           const QVector3D &direction) const
{
    const QSSGRenderRay ray(origin, direction);
    return renderer->syncPickAll(ray);
}

QVarLengthArray<QSSGRenderPickResult, 20> QQuick3DViewport::getPickResults(QQuick3DSceneRenderer *renderer, const QEventPoint &eventPoint) const
{
    QVarLengthArray<QSSGRenderPickResult, 20> pickResults;
    QPointF realPosition = eventPoint.position() * window()->effectiveDevicePixelRatio();
    // correct when mapping is not 1:1 due to explicit backing texture size
    realPosition.rx() *= m_widthMultiplier;
    realPosition.ry() *= m_heightMultiplier;
    std::optional<QSSGRenderRay> rayResult = renderer->getRayFromViewportPos(realPosition);
    if (rayResult.has_value())
        pickResults = renderer->syncPickAll(rayResult.value());
    return pickResults;
}

/*!
    \internal

    This provides a way to lookup frontendNodes with a backend node taking into consideration both
    the scene and the importScene
*/
QQuick3DObject *QQuick3DViewport::findFrontendNode(const QSSGRenderGraphObject *backendObject) const
{
    if (!backendObject)
        return nullptr;

    const auto sceneManager = QQuick3DObjectPrivate::get(m_sceneRoot)->sceneManager;
    QQuick3DObject *frontendObject = sceneManager->lookUpNode(backendObject);
    if (!frontendObject && m_importScene) {
        const auto importSceneManager = QQuick3DObjectPrivate::get(m_importScene)->sceneManager;
        frontendObject = importSceneManager->lookUpNode(backendObject);
    }
    return frontendObject;
}

QQuick3DPickResult QQuick3DViewport::processPickResult(const QSSGRenderPickResult &pickResult) const
{
    if (!pickResult.m_hitObject)
        return QQuick3DPickResult();

    QQuick3DObject *frontendObject = findFrontendNode(pickResult.m_hitObject);

    QQuick3DModel *model = qobject_cast<QQuick3DModel *>(frontendObject);
    if (model) {
        auto itemAndPosition = getItemAndPosition(pickResult);
        return QQuick3DPickResult(model,
                                  ::sqrtf(pickResult.m_distanceSq),
                                  pickResult.m_localUVCoords,
                                  pickResult.m_scenePosition,
                                  pickResult.m_localPosition,
                                  pickResult.m_faceNormal,
                                  pickResult.m_sceneNormal,
                                  pickResult.m_instanceIndex,
                                  itemAndPosition.first);
    }

    QQuick3DItem2D *frontend2DItem = qobject_cast<QQuick3DItem2D *>(frontendObject);
    if (frontend2DItem && frontend2DItem->contentItem()) {
        // Check if the pick is inside the content item (since the ray just intersected on the items plane)
        const QPointF subscenePosition = pickResult.m_localUVCoords.toPointF();
        const auto child = frontend2DItem->contentItem()->childAt(subscenePosition.x(), subscenePosition.y());
        if (child) {
            return QQuick3DPickResult(child,
                                      ::sqrtf(pickResult.m_distanceSq),
                                      QVector2D(frontend2DItem->contentItem()->mapToItem(child, subscenePosition)),
                                      pickResult.m_scenePosition,
                                      pickResult.m_localPosition,
                                      pickResult.m_faceNormal);
        }
    }

    return QQuick3DPickResult();

}

// Returns the first found scene manager of objects children
QQuick3DSceneManager *QQuick3DViewport::findChildSceneManager(QQuick3DObject *inObject, QQuick3DSceneManager *manager)
{
    if (manager)
        return manager;

    auto children = QQuick3DObjectPrivate::get(inObject)->childItems;
    for (auto child : children) {
        if (auto m = QQuick3DObjectPrivate::get(child)->sceneManager) {
            manager = m;
            break;
        }
        manager = findChildSceneManager(child, manager);
    }
    return manager;
}

void QQuick3DViewport::updateInputProcessing()
{
    // This should be called from the gui thread.
    setAcceptTouchEvents(m_enableInputProcessing);
    setAcceptHoverEvents(m_enableInputProcessing);
    setAcceptedMouseButtons(m_enableInputProcessing ? Qt::AllButtons : Qt::NoButton);
}

void QQuick3DViewport::onReleaseCachedResources()
{
    if (auto renderer = getRenderer())
        renderer->releaseCachedResources();
}

/*!
    \qmlproperty List<QtQuick3D::Object3D> View3D::extensions

    This property contains a list of user extensions that should be used with this \l View3D.

    \sa RenderExtension
*/
QQmlListProperty<QQuick3DObject> QQuick3DViewport::extensions()
{
    return QQmlListProperty<QQuick3DObject>{ this,
                                              &m_extensionListDirty,
                                              &QQuick3DExtensionListHelper::extensionAppend,
                                              &QQuick3DExtensionListHelper::extensionCount,
                                              &QQuick3DExtensionListHelper::extensionAt,
                                              &QQuick3DExtensionListHelper::extensionClear,
                                              &QQuick3DExtensionListHelper::extensionReplace,
                                              &QQuick3DExtensionListHelper::extensionRemoveLast};
}

/*!
    \internal
 */
void QQuick3DViewport::rebuildExtensionList()
{
    m_extensionListDirty = true;
    update();
}

/*!
    \internal

    Private constructor for the QQuick3DViewport class so we can differentiate between
    a regular QQuick3DViewport and one created for a specific usage, like XR.
 */
QQuick3DViewport::QQuick3DViewport(PrivateInstanceType type, QQuickItem *parent)
    : QQuick3DViewport(parent)
{
    m_isXrViewInstance = type == PrivateInstanceType::XrViewInstance;
}

void QQuick3DViewport::updateCameraForLayer(const QQuick3DViewport &view3D, QSSGRenderLayer &layerNode)
{
    layerNode.explicitCameras.clear();
    if (!view3D.m_multiViewCameras.isEmpty()) {
        for (QQuick3DCamera *camera : std::as_const(view3D.m_multiViewCameras))
            layerNode.explicitCameras.append(static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::get(camera)->spatialNode));
    } else if (view3D.camera()) {
        if (QSSGRenderCamera *camera = static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::get(view3D.camera())->spatialNode))
            layerNode.explicitCameras.append(camera);
    }

    // Ensure these have a parent. All nodes need to be in the node tree somewhere, even if they're technically "parentless"
    // or we'll not assign a storage slot for them or update them.
    for (QSSGRenderCamera *camera : std::as_const(layerNode.explicitCameras)) {
        if (!camera->parent)
            layerNode.addChild(*camera);
    }
}

void QQuick3DViewport::updateSceneManagerForImportScene()
{
    auto privateObject = QQuick3DObjectPrivate::get(m_importScene);
    if (!privateObject->sceneManager) {
        // If object doesn't already have scene manager, check from its children
        QQuick3DSceneManager *manager = findChildSceneManager(m_importScene);
        // If still not found, use the one from the scene root (scenes defined outside of an view3d)
        if (!manager)
            manager = QQuick3DObjectPrivate::get(m_sceneRoot)->sceneManager;
        if (manager) {
            manager->setWindow(window());
            privateObject->refSceneManager(*manager);
        }

        // In the case m_sceneRoot doesn't have a valid sceneManager,
        // it means the view3d is not valid now.
        if (!privateObject->sceneManager)
            return;

    }
    connect(privateObject->sceneManager, &QQuick3DSceneManager::needsUpdate,
            this, &QQuickItem::update, Qt::UniqueConnection);
    connect(privateObject->sceneManager, &QObject::destroyed,
            this, [&](QObject *) {
                auto privateObject = QQuick3DObjectPrivate::get(m_importScene);
                privateObject->sceneManager = nullptr;
                updateSceneManagerForImportScene();
            }, Qt::DirectConnection);

    QQuick3DNode *scene = m_importScene;
    while (scene) {
        QQuick3DSceneRootNode *rn = qobject_cast<QQuick3DSceneRootNode *>(scene);
        scene = rn ? rn->view3D()->importScene() : nullptr;

        if (scene) {
            connect(QQuick3DObjectPrivate::get(scene)->sceneManager,
                    &QQuick3DSceneManager::needsUpdate,
                    this, &QQuickItem::update, Qt::UniqueConnection);
        }
    }
}

/*!
    \qmlproperty enumeration QtQuick3D::View3D::renderOverrides

    Controls how Qt Quick 3D performs rendering.

    \value View3D.None Rendering proceeds normally. Internal passes are scheduled as required by the
    features used in the application.

    \value View3D.DisableInternalPasses Disables Qt Quick 3D's internal rendering passes.
    When this mode is set, the application is responsible for producing and presenting the final frame.
    Typically, this involves implementing custom \l {User passes} and presenting the result to the
    viewport using \l {SimpleQuadRenderer} or a custom \l {QQuick3DRenderExtension}{render extension}.
*/

QQuick3DViewport::RenderOverrides QQuick3DViewport::renderOverrides() const
{
    return m_renderOverrides;
}

void QQuick3DViewport::setRenderOverrides(RenderOverrides newRenderOverrides)
{
    if (m_renderOverrides == newRenderOverrides)
        return;
    m_renderOverrides = newRenderOverrides;
    emit renderOverridesChanged();

    update();
}

QT_END_NAMESPACE
