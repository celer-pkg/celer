// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qtquick3dxrglobal_p.h"
#include "qquick3dxritem_p.h"
#include "qquick3dxrview_p.h"
#include <QQuickWindow>
#include <QQuickItem>
#include <QLoggingCategory>

#include <QtQuick3DUtils/private/qssgassert_p.h>

#include "qquick3dxrinputmanager_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);

/*!
    \qmltype XrView
    \inherits Node
    \inqmlmodule QtQuick3D.Xr
    \brief Sets up the view for an Xr application.

    An XrView sets up the view for an XR application.
    The following snippet is from the \l{\qxr Simple Example} and shows
    how to use the type.

    \quotefromfile xr_simple/main.qml
    \printto XrOrigin

    \section1 Platform notes

    \section2 Meta Quest Devices

    To \l{XrView::passthroughEnabled}{enable passthrough} you need to add the
    following permisson your app's \c AndroidManifest.xml file:

    \badcode
    <uses-feature android:name="com.oculus.feature.PASSTHROUGH" android:required="false"/>
    \endcode

*/

QQuick3DXrView::QQuick3DXrView()
    : m_xrRuntimeInfo(&m_xrManager)
{
    init();
}

QQuick3DXrView::~QQuick3DXrView()
{
    m_inDestructor = true;
}

/*!
    \qmlproperty XrOrigin QtQuick3D.Xr::XrView::xrOrigin
    \brief Holds the active XR origin.

    The XR origin is the point in the scene that is considered the origin of the
    XR coordinate system. The XR origin is used to position tracked objects like
    the camera and controllers in the scene. An application can have multiple XrOrigins
    but only one can be active at a time.

    \note This property must be set for the scene to be rendered in XR.

    \sa XrOrigin
*/

QQuick3DXrOrigin *QQuick3DXrView::xrOrigin() const
{
    return m_xrOrigin;
}

/*!
    \qmlproperty SceneEnvironment QtQuick3D.Xr::XrView::environment
    \summary Holds the SceneEnvironment for the XR view.
*/

QQuick3DSceneEnvironment *QQuick3DXrView::environment() const
{
    return m_xrManager.m_vrViewport ? m_xrManager.m_vrViewport->environment() : nullptr;
}

QQuick3DViewport *QQuick3DXrView::view3d() const
{
    return m_xrManager.m_vrViewport;
}

namespace {

struct ClosestPointResult
{
    bool grab = false;
    bool inside = false;
    float distance = 0;
    QVector3D position;
    QVector3D scenePosition;
    QVector3D normal;
    QVector3D sceneNormal;
    QVector2D uvPosition;
    QPointF point2D;
    QQuickItem *quickItem = nullptr;
};

// We can assume that the transform for an XrItem always has uniform scaling,
// so the closest point in local space is also closest in scene space
// We're not interested in hits from the side; but we probably want a margin around the item
static ClosestPointResult closestPointOnXrItem(const QQuick3DXrItem *xrItem, const QVector3D &pos, const float margin = 0)
{
    const auto mappedPos = xrItem->mapPositionFromScene(pos);
    const qreal width = xrItem->width();
    const qreal height = xrItem->height();

    const float x = mappedPos.x();
    const float y = mappedPos.y();
    const float z = mappedPos.z();

    const float x1 = 0;
    const float y1 = -height;
    const float x2 = width;
    const float y2 = 0;

    const bool insideMargin = x1 - margin <= x && x <= x2 + margin && y1 - margin <= y && y <= y2 + margin;
    if (!insideMargin)
        return {};
    const bool inside = x1 <= x && x <= x2 && y1 <= y && y <= y2 && z <= 0;

    const float distance = mappedPos.z();

    const QVector3D position{ mappedPos.x(), mappedPos.y(), 0 };
    const QVector3D scenePosition = xrItem->mapPositionToScene(position);
    const QVector3D normal{ 0, 0, 1 };
    const QVector3D sceneNormal = xrItem->mapDirectionToScene(normal);

    return {
        true,
        inside,
        distance,
        position,
        scenePosition,
        normal,
        sceneNormal,
        QVector2D{},
        QPointF{mappedPos.x(), -mappedPos.y()},
        xrItem->contentItem()
    };
}


static ClosestPointResult closestPointOnModel(QQuick3DXrView *view, QQuick3DModel *model, const QVector3D &pos, const float radius = 0)
{
    auto pickResult = view->closestPointPick(pos, radius, model);
    if (pickResult.objectHit() != model)
        return {};

    const QVector3D offset = pos - pickResult.scenePosition();

    const float signedDistance = QVector3D::dotProduct(offset, pickResult.sceneNormal());
    const bool inside = signedDistance < 0;

    QPointF quickPoint;
    QQuickItem *quickItem = pickResult.itemHit();
    auto uv = pickResult.uvPosition();
    if (quickItem) {
        quickPoint = QPointF(quickItem->x() + uv.x() * quickItem->width(),
                             quickItem->y() - uv.y() * quickItem->height() + quickItem->height());
    }
    return {
        true,
        inside,
        signedDistance,
        pickResult.position(),
        pickResult.scenePosition(),
        pickResult.normal(),
        pickResult.sceneNormal().normalized(), //#### TODO: fix sceneNormal
        uv,
        quickPoint,
        quickItem
    };
}


static inline ClosestPointResult closestPointOnItem(QQuick3DXrView *view, QQuick3DXrView::TouchTarget target, const QVector3D &pos, const float margin)
{
    if (auto *asXrItem = std::get_if<QQuick3DXrItem*>(&target))
        return closestPointOnXrItem(*asXrItem, pos, margin);
    if (auto *asModel =std::get_if<QQuick3DModel*>(&target))
        return closestPointOnModel(view, *asModel, pos, margin);
    return {};
}

static inline void sendTouch(QQuick3DXrView *view, const ClosestPointResult &point, int pointId, bool active)
{
    if (point.quickItem)
        view->setTouchpoint(point.quickItem, point.point2D, pointId, active);
}

} // anonymous namespace


// Sends appropriate touch events.
// Updates the touchState and returns true if this target grabs the touch point.
// touchState is input/output, and input contains the previous state if touchState->grabbed is true
bool QQuick3DXrView::handleVirtualTouch(TouchTarget target, const QVector3D &pos, TouchState *touchState, QVector3D *offset)
{
    constexpr qreal sideMargin = 10;
    constexpr qreal hoverHeight = 10;
    constexpr qreal cancelDepth = 50;

    constexpr qreal releaseHeight = 2; // Minimum vertical movement to count as a press/release when below
    constexpr qreal releaseHeightSquared = releaseHeight * releaseHeight;
    constexpr qreal smallDistance = 0.5; // Any movement shorter than this distance is insignificant
    constexpr qreal sidewaysMax = 3; // Any horizontal movement larger than this distance means this is never going to be a press/release below the surface
    constexpr qreal sidewaysMaxSquared = sidewaysMax * sidewaysMax;
    constexpr qreal releaseSpeed = 5.0 / 1000; // Minimum vertical speed (cm/ms) to classify as a press/release
    constexpr int interval = 100; // Time between checks to determine insignificant movement

    const bool wasGrabbed = touchState->grabbed;
    const bool wasPressed = touchState->pressed;
    const qreal wasInside = wasGrabbed && touchState->touchDepth < 0.0;

    const QVector3D oldScenePos = touchState->previous;
    const QVector3D oldSceneNormal = touchState->normal;

    // Note: We always grab the closest point on the surface. This means that for thin and flat models,
    // we will snap to the backside, which is probably not what the user wants. If this turns out to
    // be a problem when we evaluate this, we can change this algorithm to ignore points that have a
    // normal vector pointing the wrong way.

    auto closest = closestPointOnItem(this, target, pos, sideMargin);

    bool grab = false;
    bool pressed = false;

    bool resetTimer = false;
    float z = closest.distance; // assumes signed distance

    if (wasGrabbed) {
        // maintain grab as long as we don't move outside
        grab = closest.grab && z > -cancelDepth && z < hoverHeight;

        // if grab is false, we're definitely not pressed
        if (grab) {
            // We do a press/release transition if we
            // 1. Moved between outside and inside
            // OR
            // 2. Had a fast enough vertical movement below the surface, without a horizontal movement

            if (wasInside && closest.inside) {
                // We stayed inside; let's see if we've had a strong vertical movement
                int duration = touchState->timer.elapsed();
                pressed = wasPressed; // We maintain pressed state unless proven otherwise
                const QVector3D movement = pos - oldScenePos; // Movement is positive away from the surface
                const qreal verticalDistance = QVector3D::dotProduct(movement, oldSceneNormal);
                const qreal verticalDistanceSquared = verticalDistance * verticalDistance;
                const qreal horizontalDistanceSquared = movement.lengthSquared() - verticalDistance*verticalDistance;

                if (verticalDistanceSquared >= releaseHeightSquared && horizontalDistanceSquared < releaseHeightSquared) {
                    // We have moved a significant distance vertically, without moving horizontally,
                    // so this is a press/release (no-op if it's a press when already pressed)
                    pressed = verticalDistance < 0;
                    resetTimer = true;
                } else if (duration >= interval) {
                    // Check to see if this can become a press/release in the future
                    if (horizontalDistanceSquared >= sidewaysMaxSquared) {
                        // This is sideways motion, and not a press/release
                        resetTimer = true;
                    } else if (pressed ? verticalDistance < smallDistance : verticalDistance > -smallDistance ) {
                        // we haven't had a significant vertical movement in the right direction (positive when pressed),
                        // so this is not the start of a press/release
                        resetTimer = true;
                    } else {
                        // If the speed is high enough, this can still become a press/release in the future
                        // (If we're pressed, we're looking for positive speed)
                        const qreal verticalSpeed = verticalDistance / duration;
                        const bool sufficientSpeed = pressed ? verticalSpeed > releaseSpeed : verticalSpeed < -releaseSpeed;
                        resetTimer = !sufficientSpeed;
                    }
                }
            } else {
                // Either hover or move between outside and inside
                pressed = closest.inside; // closest.inside is only true if z < 0, so we don't have to check for that
                resetTimer = true;
            }
        }
    } else {
        // only grab if hovering, not if inside: we only want to register a press
        // coming from a hover, not when approaching from below
        grab = closest.grab && closest.distance > 0 && closest.distance < hoverHeight;
        resetTimer = true;
    }

    if (grab) {
        if (offset && z < 0)
            *offset = closest.scenePosition - pos;
        touchState->grabbed = true;
        touchState->target = target;
        touchState->touchDistance = qMax(z, 0.0); // Documented to be 0 when pressed
        touchState->touchDepth = z;
        touchState->pressed = pressed;
        touchState->cursorPos = closest.point2D;
        touchState->surfacePoint = closest.scenePosition;
        touchState->normal = closest.sceneNormal;
        touchState->uvPosition = closest.uvPosition;
        if (resetTimer) {
            touchState->previous = pos;
            touchState->timer.start();
        }

        sendTouch(this, closest, touchState->pointId, pressed);
        return true;
    }

    if (wasGrabbed) {
        touchState->grabbed = false;
        touchState->pressed = false;
        touchState->target = {};
        sendTouch(this, closest, touchState->pointId, false);
    }

    return false;
}

/*!
    \qmlproperty bool QtQuick3D.Xr::XrView::passthroughEnabled
    \summary Holds whether passthrough is enabled for the XR view.
*/
bool QQuick3DXrView::passthroughEnabled() const
{
    return m_xrManager.isPassthroughEnabled();
}

/*!
    \qmlproperty QQuick3DXrRuntimeInfo QtQuick3D.Xr::XrView::runtimeInfo
    \summary Provides information about the XR runtime for the XR view.
*/

QQuick3DXrRuntimeInfo *QQuick3DXrView::runtimeInfo() const
{
    return &m_xrRuntimeInfo;
}

void QQuick3DXrView::setEnvironment(QQuick3DSceneEnvironment *environment)
{
    QQuick3DViewport *view = m_xrManager.m_vrViewport;

    // If the view is not created yet, we can't set the environment which means we need to
    // set it again once the view is created...
    if (!view) {
        m_pendingSceneEnvironment = environment;
        return;
    }

    auto oldEnvironment = view->environment();
    if (oldEnvironment == environment)
        return;

    if (oldEnvironment)
        disconnect(oldEnvironment);

    view->setEnvironment(environment);

    // The view will always have an environment, setting the environment to null will just mean the default environment
    // is used. So querying the environment from the view is always valid (and we should do it here to make sure we're
    // in sync with the view).
    environment = view->environment();

    handleClearColorChanged();
    handleAAChanged();

    connect(environment, &QQuick3DSceneEnvironment::backgroundModeChanged, this, &QQuick3DXrView::handleClearColorChanged);
    connect(environment, &QQuick3DSceneEnvironment::clearColorChanged, this, &QQuick3DXrView::handleClearColorChanged);
    connect(environment, &QQuick3DSceneEnvironment::antialiasingModeChanged, this, &QQuick3DXrView::handleAAChanged);
    connect(environment, &QQuick3DSceneEnvironment::antialiasingQualityChanged, this, &QQuick3DXrView::handleAAChanged);

    emit environmentChanged(environment);
}

/*!
    \qmlproperty bool QtQuick3D.Xr::XrView::passthroughSupported
    \summary Indicates whether passthrough is supported for the XR view.
*/

bool QQuick3DXrView::passthroughSupported() const
{
    if (!m_xrManager.isValid())
        return false;

    return m_xrManager.supportsPassthrough();
}

void QQuick3DXrView::setPassthroughEnabled(bool enable)
{
    if (!m_xrManager.isValid()) {
        qWarning("Attempted to set passthrough mode without a valid XR manager");
        return;
    }

    const bool orgPassthroughEnabled = m_xrManager.isPassthroughEnabled();
    if (!m_xrManager.setPassthroughEnabled(enable)) {
        if (enable)
            qWarning("Enabling passthrough is not supported.");
        else
            qWarning("Disabling passthrough is not supported.");
        return;
    }

    if (orgPassthroughEnabled != m_xrManager.isPassthroughEnabled())
        emit passthroughEnabledChanged();
}

/*!
    \qmlproperty enumeration QtQuick3D.Xr::XrView::fixedFoveation
    \brief Controls the level of fixed foveated rendering for the XrView.
    \default XrView.HighFoveation

    Foveated rendering reduces GPU load by reducing image quality (resolution)
    in areas where the difference is less perceptible to the eye. With fixed
    foveated rendering, the areas with reduced visual fidelity are fixed and do
    not change. On some platforms, there is no concept of fixed foveated
    rendering or control over it. For example, VisionOS-based devices perform
    dynamic, eye-tracked foveation; thus, the value of this property is
    ignored in practice. Other devices, such as the Meta Quest 3, only
    support fixed foveation, which makes this property relevant.

    The value can be one of:
    \value XrView.NoFoveation 0, no foveation.
    \value XrView.LowFoveation 1, low foveation.
    \value XrView.MediumFoveation 2, medium foveation.
    \value XrView.HighFoveation 3, high foveation.

    Where supported, the default is \c HighFoveation. Therefore, changing this
    value in applications should be rarely needed in practice.
*/

QQuick3DXrView::FoveationLevel QQuick3DXrView::fixedFoveation() const
{
    return FoveationLevel(m_xrManager.getFixedFoveationLevel());
}

void QQuick3DXrView::setFixedFoveation(FoveationLevel level)
{
    const auto orgFoviationLevel = m_xrManager.getFixedFoveationLevel();
    m_xrManager.setFixedFoveationLevel(QtQuick3DXr::FoveationLevel(level));
    if (orgFoviationLevel != m_xrManager.getFixedFoveationLevel())
        emit fixedFoveationChanged();
}

/*!
    \qmlproperty bool QtQuick3D.Xr::XrView::isQuitOnSessionEndEnabled
    \brief Holds whether the application should quit when the XR session ends.
*/

bool QQuick3DXrView::isQuitOnSessionEndEnabled() const
{
    return m_quitOnSessionEnd;
}

void QQuick3DXrView::setQuitOnSessionEnd(bool enable)
{
    if (m_quitOnSessionEnd == enable)
        return;

    m_quitOnSessionEnd = enable;
    emit quitOnSessionEndChanged();
}
/*!
    \qmlproperty RenderStats QtQuick3D.Xr::XrView::renderStats
    \summary Holds rendering statistics for the XR view.
*/

QQuick3DRenderStats *QQuick3DXrView::renderStats() const
{
    return m_xrManager.m_vrViewport ? m_xrManager.m_vrViewport->renderStats() : nullptr;
}

void QQuick3DXrView::updateViewportGeometry()
{
    auto contentItem = m_xrManager.m_quickWindow->contentItem();
    auto viewport = m_xrManager.m_vrViewport;
    if (viewport->height() != contentItem->height())
        viewport->setHeight(contentItem->height());
    if (viewport->width() != contentItem->width())
        viewport->setWidth(contentItem->width());
    if (viewport->x() != contentItem->x())
        viewport->setX(contentItem->x());
    if (viewport->y() != contentItem->y())
        viewport->setY(contentItem->y());
}

void QQuick3DXrView::handleSessionEnded()
{
    emit sessionEnded();
    if (m_quitOnSessionEnd)
        QCoreApplication::quit();
}

void QQuick3DXrView::handleClearColorChanged()
{
    auto env = environment();

    if (env) {
        if (env->backgroundMode() == QQuick3DSceneEnvironment::Color)
            m_xrManager.m_quickWindow->setColor(env->clearColor());
        else if (env->backgroundMode() == QQuick3DSceneEnvironment::Transparent)
            m_xrManager.m_quickWindow->setColor(Qt::transparent);
    }
}

void QQuick3DXrView::handleAAChanged()
{
    auto env = environment();
    int samples = 1;
    if (env && env->antialiasingMode() == QQuick3DSceneEnvironment::MSAA) {
        switch (env->antialiasingQuality()) {
        case QQuick3DSceneEnvironment::Medium:
            samples = 2;
            break;
        case QQuick3DSceneEnvironment::High:
            samples = 4;
            break;
        case QQuick3DSceneEnvironment::VeryHigh:
            samples = 8;
            break;
        }
    }
    m_xrManager.setSamples(samples);
}

bool QQuick3DXrView::init()
{
    if (m_isInitialized) {
        qWarning("Already initialized!");
        return false;
    }

    connect(&m_xrManager, &QQuick3DXrManager::sessionEnded, this, &QQuick3DXrView::handleSessionEnded);
    connect(&m_xrManager, &QQuick3DXrManager::frameReady, this, &QQuick3DXrView::frameReady);
    connect(&m_xrManager, &QQuick3DXrManager::referenceSpaceChanged, this, &QQuick3DXrView::referenceSpaceChanged);
    connect(&m_xrManager, &QQuick3DXrManager::multiViewRenderingEnabledChanged, this, &QQuick3DXrView::multiViewRenderingEnabledChanged);
    connect(&m_xrManager, &QQuick3DXrManager::initialized, this, &QQuick3DXrView::init, Qt::UniqueConnection);

    if (!m_xrManager.isReady() && !m_xrManager.initialize()) {
        qCDebug(lcQuick3DXr, "Waiting for XR platform to be initialized");

        return false;
    }

    if (!m_xrManager.initialize()) {
        QString errorString = m_xrManager.errorString();
        if (errorString.isEmpty())
            errorString = tr("Failed to initialize XR platform");
        qWarning("\n%s\n", qPrintable(errorString));
        QMetaObject::invokeMethod(this, "initializeFailed", Qt::QueuedConnection, errorString);
        return false;
    }

    // Create View3D
    QSSG_CHECK_X(m_xrManager.m_vrViewport == nullptr, "View3D already created!");
    auto viewport = new QQuick3DViewport(QQuick3DViewport::PrivateInstanceType::XrViewInstance);
    viewport->setRenderMode(QQuick3DViewport::Underlay);
    auto contentItem = m_xrManager.m_quickWindow->contentItem();
    viewport->setParentItem(contentItem);
    m_xrManager.m_vrViewport = viewport;
    viewport->setImportScene(this);

    contentItem->forceActiveFocus(Qt::MouseFocusReason);

    connect(contentItem, &QQuickItem::heightChanged, this, &QQuick3DXrView::updateViewportGeometry);
    connect(contentItem, &QQuickItem::widthChanged, this, &QQuick3DXrView::updateViewportGeometry);
    connect(contentItem, &QQuickItem::xChanged, this, &QQuick3DXrView::updateViewportGeometry);
    connect(contentItem, &QQuickItem::yChanged, this, &QQuick3DXrView::updateViewportGeometry);

    QQuick3DSceneEnvironment *env = environment();
    if (env) {
        connect(env, &QQuick3DSceneEnvironment::backgroundModeChanged, this, &QQuick3DXrView::handleClearColorChanged);
        connect(env, &QQuick3DSceneEnvironment::clearColorChanged, this, &QQuick3DXrView::handleClearColorChanged);
        connect(env, &QQuick3DSceneEnvironment::antialiasingModeChanged, this, &QQuick3DXrView::handleAAChanged);
        connect(env, &QQuick3DSceneEnvironment::antialiasingQualityChanged, this, &QQuick3DXrView::handleAAChanged);
    }

    // NOTE: If we've called async, we need to make sure the environment, etc. is set again
    setEnvironment(m_pendingSceneEnvironment);
    m_pendingSceneEnvironment = nullptr;

    m_xrManager.update();

    m_isInitialized = true;

    return m_isInitialized;
}

/*!
    \qmlmethod pickResult XrView::rayPick(vector3d origin, vector3d direction)

    This method will \e shoot a ray into the scene starting at \a origin and in
    \a direction and return information about the nearest intersection with an
    object in the scene.

    For example, pass the position and forward vector of
    any object in a scene to see what object is in front of an item. This
    makes it possible to do picking from any point in the scene.
 */
QQuick3DPickResult QQuick3DXrView::rayPick(const QVector3D &origin, const QVector3D &direction) const
{
    if (!m_xrManager.m_vrViewport)
        return QQuick3DPickResult();

    return m_xrManager.m_vrViewport->rayPick(origin, direction);
}

/*!
    \qmlmethod List<pickResult> XrView::rayPickAll(vector3d origin, vector3d direction)

    This method will \e shoot a ray into the scene starting at \a origin and in
    \a direction and return a list of information about the nearest intersections with
    objects in the scene.
    The list is presorted by distance from the origin along the direction
    vector with the nearest intersections appearing first and the furthest
    appearing last.

    This can, for instance, be called with the position and forward vector of
    any object in a scene to see what objects are in front of an item. This
    makes it possible to do picking from any point in the scene.
 */
QList<QQuick3DPickResult> QQuick3DXrView::rayPickAll(const QVector3D &origin, const QVector3D &direction) const
{
    if (!m_xrManager.m_vrViewport)
        return {};

    return m_xrManager.m_vrViewport->rayPickAll(origin, direction);
}

/*!
    \qmlmethod pickResult XrView::rayPick(vector3d origin, vector3d direction, Model model)

    This method will "shoot" a ray into the scene starting at \a origin and in
    \a direction and return information about the intersection between the ray and the specified \a model.

    \since 6.11
*/
QQuick3DPickResult QQuick3DXrView::rayPick(const QVector3D &origin, const QVector3D &direction, QQuick3DModel *model) const
{
    if (!m_xrManager.m_vrViewport)
        return QQuick3DPickResult();

    return m_xrManager.m_vrViewport->rayPick(origin, direction, model);
}

/*!
    \qmlmethod pickResult XrView::closestPointPick(vector3d origin, float radius, Model model)

    This method will find the point on the surface of \a model that is nearest to \a origin, within a distance of
    \a radius. If \a model is \c null, the closest object within \a radius will be found.

    If no such object exists, \c null is returned.

    \since 6.11
*/
QQuick3DPickResult QQuick3DXrView::closestPointPick(const QVector3D &origin, float radius, QQuick3DModel *model) const
{
    if (!m_xrManager.m_vrViewport)
        return QQuick3DPickResult();

    return m_xrManager.m_vrViewport->closestPointPick(origin, radius, model);
}

/*!
    \qmlmethod void XrView::setTouchpoint(Item target, point position, int pointId, bool pressed)

    Sends a synthetic touch event to \a target, moving the touch point with ID \a pointId to \a position,
    with \a pressed determining if the point is pressed.
    Also sends the appropriate touch release event if \a pointId was previously active on a different
    item.
*/

void QQuick3DXrView::setTouchpoint(QQuickItem *target, const QPointF &position, int pointId, bool pressed)
{
    view3d()->setTouchpoint(target, position, pointId, pressed);
}

// TODO: Maybe do a proper QQuick3DXrViewPrivate instead
struct QQuick3DXrView::XrTouchStates
{
    QHash<int, TouchState> points;
};

/*!
    \qmlmethod vector3d XrView::processTouch(vector3d position, int pointId)

    This method will search for an XrItem, or a Model with a
    \l{Texture::sourceItem}{sourceItem texture}, near \a position and send a
    virtual touch event with touch point ID \a pointId if \a position maps to
    a point on the surface.

    The return value is the offset between \a position and the touched point on
    the surface. This can be used to prevent a hand model from passing through
    an XrItem.

    \sa XrHandModel

*/

QVector3D QQuick3DXrView::processTouch(const QVector3D &pos, int pointId)
{
    QVector3D offset;

    if (!m_touchStates)
        m_touchStates = new XrTouchStates;
    TouchState &state = m_touchStates->points[pointId];
    state.pointId = pointId; // in case it's a new point that was default-constructed

    bool grabbed = false;
    const bool prevTarget = !std::holds_alternative<std::monostate>(state.target);
    if (prevTarget) {
        grabbed = handleVirtualTouch(state.target, pos, &state, &offset);
    }
    if (!grabbed) {
        for (auto *item : std::as_const(m_xrItems)) {
            // For simplicity, try handleVirtualTouch on target again, even if we know it's not going to be different
            if (item->visible())
                grabbed = handleVirtualTouch(item, pos, &state, &offset);
            if (grabbed)
                break;
        }
    }

    if (!grabbed) {
        constexpr float pickRadius = 10.0;
        auto pickResult = closestPointPick(pos, pickRadius);
        //qDebug() << "pick result" << pickResult.objectHit() << pickResult.position();
        if (QQuick3DModel *obj = pickResult.objectHit())
            grabbed = handleVirtualTouch(obj, pos, &state, &offset); // will call closestPointPick again, but this makes the code simpler
    }

    return offset;
}

/*!
    \qmlmethod object XrView::touchpointState(int pointId)

    This method returns the state of the touch point with ID \a pointId.
    The state is represented by a map from property names to values:

    \table
    \header
    \li Key
    \li Type
    \li Description
    \row
    \li \c grabbed
    \li \c bool
    \li Is the point grabbed by an item? If \c false, all other values are \c undefined.
    \row
    \li \c target
    \li XrItem
    \li The item that is grabbing the touch point, or \c null if there is no XrItem.
    \row
    \li \c pressed
    \li \c bool
    \li Is the touch point pressed?
    \row
    \li \c cursorPos
    \li \c point
    \li The 2D position of the touch point within \c target
    \row
    \li \c touchDistance
    \li \c real
    \li The distance from the plane to the touch point. It will be \c 0 if \c pressed is \c true.
    \row
    \li \c surfacePoint
    \li \c vector3d
    \li The position of the touch point in scene space. [since 6.11]
    \row
    \li \c normal
    \li \c vector3d
    \li The normal vector at the touch point in scene space. [since 6.11]
    \row
    \li \c uvPosition
    \li \c vector2d
    \li The UV position at the touch point. [since 6.11]
    \row
    \li \c model
    \li \c Model
    \li The model that is grabbing the touch point, or \c null if there is no Model. [since 6.11]
    \endtable
 */

#define Q_TOUCHPOINT_STATE(prop) { QStringLiteral(#prop), QVariant::fromValue(it->prop) }
QVariantMap QQuick3DXrView::touchpointState(int pointId) const
{
    auto constexpr end = QHash<int, TouchState>::const_iterator();
    auto it = m_touchStates ? m_touchStates->points.constFind(pointId) : end;

    if (it == end)
        return { { QStringLiteral("grabbed"), QVariant::fromValue(false) } };
    auto *itemPointer = std::get_if<QQuick3DXrItem *>(&it->target);
    QQuick3DXrItem *xrItem = itemPointer ? *itemPointer : nullptr;
    auto *modelPointer = std::get_if<QQuick3DModel *>(&it->target);
    QQuick3DModel *model = modelPointer ? *modelPointer : nullptr;
    return { { QStringLiteral("target"), QVariant::fromValue(xrItem) },
             Q_TOUCHPOINT_STATE(grabbed),
             Q_TOUCHPOINT_STATE(pressed),
             Q_TOUCHPOINT_STATE(cursorPos),
             Q_TOUCHPOINT_STATE(surfacePoint),
             Q_TOUCHPOINT_STATE(normal),
             Q_TOUCHPOINT_STATE(touchDistance),
             Q_TOUCHPOINT_STATE(uvPosition),
             { QStringLiteral("model"), QVariant::fromValue(model) }};
}
#undef Q_TOUCHPOINT_STATE

/*!
    \qmlproperty enumeration QtQuick3D.Xr::XrView::referenceSpace
    \brief Gets or sets the reference space for the XR view.

     It can be one of:
     \value XrView.ReferenceSpaceUnknown
     \value XrView.ReferenceSpaceLocal Origin is at the default view position (typically defined by a "reset view" operation).
     \value XrView.ReferenceSpaceStage Origin is at floor height in the center of the user's defined area.
     \value XrView.ReferenceSpaceLocalFloor Origin is at floor height, below the default view position.

    \c ReferenceSpaceLocal is mainly useful for seated applications where the content is not positioned
    relative to the floor, for example floating menus. The content will move when the user resets the view.

    \c ReferenceSpaceStage is mainly useful for room-scale applications where the user will move freely within the
    playing area. The content will not move when the user resets the view.

    \c ReferenceSpaceLocalFloor is mainly useful for stationary applications (seated or standing) where the content is
    positioned relative to the floor. The content will move when the user resets the view.

    \note On visionOS, the reference space is always \c ReferenceSpaceLocalFloor and
    cannot be changed. This means that applications designed with \c ReferenceSpaceLocal
    will have their origin at floor level on visionOS, which can cause content to appear
    at an unexpected position. To handle this, applications can check the
    \l referenceSpace property at runtime and adjust the vertical position of their
    content accordingly. For example:

    \code
    y: xrView.referenceSpace === XrView.ReferenceSpaceLocalFloor ? 130 : 0
    \endcode

    \default XrView.ReferenceSpaceLocal
*/

QQuick3DXrView::ReferenceSpace QQuick3DXrView::referenceSpace() const
{
    return ReferenceSpace(m_xrManager.getReferenceSpace());
}

void QQuick3DXrView::setReferenceSpace(ReferenceSpace newReferenceSpace)
{
    m_xrManager.setReferenceSpace(QtQuick3DXr::ReferenceSpace(newReferenceSpace));
}

bool QQuick3DXrView::depthSubmissionEnabled() const
{
    if (!m_xrManager.isValid()) {
        qWarning("Attempted to check depth submission mode without a valid XR manager");
        return false;
    }

    return m_xrManager.isDepthSubmissionEnabled();
}

/*!
    \qmlproperty bool QtQuick3D.Xr::XrView::multiViewRenderingSupported

    \brief This read-only property reports the availability of \l{Multiview Rendering}.

    \sa multiViewRenderingEnabled
 */
bool QQuick3DXrView::isMultiViewRenderingSupported() const
{
    if (!m_xrManager.isValid())
        return false;

    return m_xrManager.isMultiViewRenderingSupported();
}

/*!
    \qmlproperty bool QtQuick3D.Xr::XrView::multiViewRenderingEnabled

    \brief This is a read-only property that reports if \l{Multiview Rendering} is enabled for the XR view.

    \default true

    This property tells you if multiview rendering is actually in use at run time.
    When not supported, the value will flip back to \c false.

    Enabling multiview rendering is recommended. It can improve performance and reduce
    CPU and GPU power usage. It defaults to disabled to ensure maximum
    compatibility. Developers are encouraged to verify that their application
    renders as expected with multiViewRenderingEnabled set to \c true and then
    leave it set afterward.

    \note Certain Qt Quick and Quick 3D features that involve shader code that is
    provided by the application may need this code to be modified to be multiview
    compatible. Examples of these are custom 2D and 3D materials and
    postprocessing effects. The \l {Multiview Rendering} documentation provides
    more information on this and how to disable multiview rendering.

    \sa multiViewRenderingSupported {Multiview Rendering}
*/
bool QQuick3DXrView::multiViewRenderingEnabled() const
{
    if (!m_xrManager.isValid())
        return false;

    return m_xrManager.isMultiViewRenderingEnabled();
}

void QQuick3DXrView::registerXrItem(QQuick3DXrItem *newXrItem)
{
    m_xrItems.append(newXrItem);
}

void QQuick3DXrView::unregisterXrItem(QQuick3DXrItem *xrItem)
{
    m_xrItems.removeAll(xrItem);
}

/*!
    \qmlproperty bool QtQuick3D.Xr::XrView::depthSubmissionEnabled
    \brief Controls whether submitting the depth buffer to the XR compositor
    is enabled.
    \default false

    By default, the depth buffer used by the 3D scene in the XrView is not exposed
    to the XR compositor. However, in some platforms, depth submission is implicit
    and cannot be disabled or controlled by the application. An example of this is
    VisionOS. Changing this property has no effect on those platforms. Elsewhere,
    with OpenXR in particular, support depends on the OpenXR implementation used
    at run time.

    It is always safe to set depthSubmissionEnabled to \c true. It will just have
    no effect when not supported by the underlying stack. To be sure, you can
    inspect the debug output to see if depth submission is in use.
    Submitting the depth buffer may improve reprojections that the XR compositor
    may perform. Reprojection could happen, for example, when the system cannot
    maintain the target frame rate and thus has to resort to predicting frame
    contents to improve and stabilize the user's perception of the
    scene and reduce possible motion sickness. However, the application and
    Qt have no control over data usage. It could also happen that
    submitting depth data has no practical effects and is ignored by the
    underlying XR runtime and compositor.

    In practice, submitting the depth buffer implies rendering into a depth
    texture provided by the XR runtime instead of the intermediate texture/render buffer
    created and managed by Qt. Rendering into a depth texture has certain lower-level
    consequences that can have a performance impact:

    When using \l{QtQuick3D::SceneEnvironment::antialiasingMode}{multisample antialiasing}
    (MSAA), enabling depth submission implies rendering into a multisample depth
    texture and resolving the samples into the non-multisample depth texture provided by
    the XR runtime. Without depth submission,
    the resolve step would not be necessary. In addition, some 3D APIs
    do not support resolving multisample depth-stencil data (see
    the \l{QRhi::ResolveDepthStencil} flag for details). Without this support,
    attempts to  enable depth submission in combination with MSAA are gracefully ignored.

    Even when MSAA is not used, enabling depth submission triggers writing out
    depth data with 3D APIs that have control over this. The store operation for
    depth/stencil data is typically indicated by Qt as unnecessary, which can
    have positive performance impacts on tiled GPU architectures. This is not
    done with depth submission because depth data must always be written out
    from Qt's perspective.

    \note We recommended that developers test their applications with depth
    submission enabled, evaluate the advantages and disadvantages, and make a
    conscious choice based on their testing if they wish to enable it or not.
*/

void QQuick3DXrView::setDepthSubmissionEnabled(bool enable)
{
    if (!m_xrManager.isValid()) {
        qWarning("Attempted to set depth submission mode without a valid XR manager");
        return;
    }

    const bool orgDepthSubmission = m_xrManager.isDepthSubmissionEnabled();

    m_xrManager.setDepthSubmissionEnabled(enable);

    if (orgDepthSubmission != m_xrManager.isDepthSubmissionEnabled())
        emit depthSubmissionEnabledChanged();
}

void QQuick3DXrView::setXROrigin(QQuick3DXrOrigin *newXrOrigin)
{
    if (m_xrOrigin == newXrOrigin)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DXrView::setXROrigin, newXrOrigin, m_xrOrigin);

    m_xrOrigin = newXrOrigin;

    // Make sure the XrOrigin has a parent item, if it hasn't, we're it.
    if (m_xrOrigin && !m_xrOrigin->parentItem())
        m_xrOrigin->setParentItem(this);

    m_xrManager.setXROrigin(m_xrOrigin);

    emit xrOriginChanged();
}

QQuick3DViewport *QQuick3DXrViewPrivate::getView3d(QQuick3DXrView *view)
{
    QSSG_ASSERT(view != nullptr, return nullptr);
    return view->view3d();
}

/*!
    \qmlsignal XrView::initializeFailed(const QString &errorString)

    Emitted when initialization fails, and there is a new \a errorString
    describing the failure.
 */

/*!
    \qmlsignal XrView::sessionEnded()

    Emitted when the session ends.
 */

/*!
    \qmlsignal XrView::frameReady()
    \internal

    Emitted when a new frame is ready.
 */

QT_END_NAMESPACE
