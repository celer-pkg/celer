// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dcamera_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>

#include "qquick3dquaternionutils_p.h"
#include "qquick3dnode_p_p.h"

#include <QtMath>
#include <QtQuick3DUtils/private/qssgutils_p.h>

#include "qquick3dutils_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Camera
    \inherits Node
    \inqmlmodule QtQuick3D
    \brief Defines an abstract base for Cameras.

    A Camera defines how the content of the 3D scene is projected onto a 2D surface,
    such as a View3D. A scene needs at least one Camera in order to visualize its
    contents.

    It is possible to position and rotate the Camera like any other spatial \l{QtQuick3D::Node}{Node} in
    the scene. The \l{QtQuick3D::Node}{Node}'s location and orientation determines where the Camera is in
    the scene, and what direction it is facing. The default orientation of the camera
    has its forward vector pointing along the negative Z axis and its up vector along
    the positive Y axis.

    Together with the position and orientation, the frustum defines which parts of
    a scene are visible to the Camera and how they are projected onto the 2D surface.
    The different Camera subtypes provide multiple options to determine the shape of the
    Camera's frustum.

    \list
    \li PerspectiveCamera provides a camera with a pyramid-shaped frustum, where objects are
    projected so that those further away from the camera appear to be smaller. This is the
    most commonly used camera type, and corresponds to how most real world cameras work.
    \li OrthographicCamera provides a camera where the lines of the frustum are parallel,
    making the perceived scale of an object unaffected by its distance to the camera. Typical
    use cases for this type of camera are CAD (Computer-Assisted Design) applications and
    cartography.
    \li FrustumCamera is a perspective camera type where the frustum can be freely customized
    by the coordinates of its intersection with the near plane. It can be useful if an
    asymmetrical camera frustum is needed.
    \li CustomCamera is a camera type where the projection matrix can be freely customized,
    and can be useful for advanced users who wish to calculate their own projection matrix.
    \endlist

    To illustrate the difference, these screenshots show the same scene as projected by a
    PerspectiveCamera and an OrthographicCamera. Notice how the red box is smaller than the
    green box in the image rendered using the perspective projection.
    \table
    \header
    \li Perspective camera
    \li Orthographic camera
    \row
    \li \image perspectivecamera.png
               {Perspective camera view of plate}
    \li \image orthographiccamera.png
               {Orthographic camera view of plate}
    \endtable

    \sa {Qt Quick 3D - View3D Example}
*/

/*!
    \internal
*/
QQuick3DCamera::QQuick3DCamera(QQuick3DNodePrivate &dd, QQuick3DNode *parent)
    : QQuick3DNode(dd, parent) {}

/*!
    \qmlproperty bool Camera::frustumCullingEnabled

    When this property is \c true, objects outside the camera frustum will be culled, meaning they will
    not be passed to the renderer. By default this property is set to \c false. For scenes where all or
    most objects are inside the camera frustum, frustum culling is an unnecessary performance overhead.
    But for complex scenes where large parts are located outside the camera's view, enabling frustum
    culling may improve performance.
*/
bool QQuick3DCamera::frustumCullingEnabled() const
{
    return m_frustumCullingEnabled;
}

void QQuick3DCamera::setFrustumCullingEnabled(bool frustumCullingEnabled)
{
    if (m_frustumCullingEnabled == frustumCullingEnabled)
        return;

    m_frustumCullingEnabled = frustumCullingEnabled;
    emit frustumCullingEnabledChanged();
    update();
}

/*!
    \qmlproperty Node Camera::lookAtNode

    If this property is set to a \c non-null value, the rotation of this camera is automatically
    updated so that this camera keeps looking at the specified node whenever the scene position of
    this camera or the specified node changes.
    By default this property is set to \c{null}.

    \sa lookAt
*/
QQuick3DNode *QQuick3DCamera::lookAtNode() const
{
    return m_lookAtNode;
}

void QQuick3DCamera::setLookAtNode(QQuick3DNode *node)
{
    if (m_lookAtNode == node)
        return;

    if (m_lookAtNode) {
        disconnect(m_lookAtNode, &QQuick3DNode::scenePositionChanged, this, &QQuick3DCamera::updateLookAt);
        disconnect(this, &QQuick3DNode::scenePositionChanged, this, &QQuick3DCamera::updateLookAt);
    }

    m_lookAtNode = node;

    if (m_lookAtNode) {
        connect(m_lookAtNode, &QQuick3DNode::scenePositionChanged, this, &QQuick3DCamera::updateLookAt);
        connect(this, &QQuick3DNode::scenePositionChanged, this, &QQuick3DCamera::updateLookAt);
    }

    emit lookAtNodeChanged();
    updateLookAt();
}

/*!
    \qmlmethod vector3d Camera::mapToViewport(vector3d scenePos)

    Transforms \a scenePos from global scene space (3D) into viewport space (2D).

    The returned position is normalized, with the top-left of the viewport
    at [0, 0] and the bottom-right at [1, 1]. The returned z-value will contain
    the distance from the near clip plane of the frustum (clipNear) to \a scenePos in
    scene coordinates. If the distance is negative, the point is behind camera.

    If \a scenePos cannot successfully be mapped to a position in the viewport, a
    position of [0, 0, 0] is returned.

    \sa mapFromViewport(), {View3D::mapFrom3DScene()}{View3D.mapFrom3DScene()}
*/
QVector3D QQuick3DCamera::mapToViewport(const QVector3D &scenePos) const
{
    QSSGRenderCamera *cameraNode = static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::get(this)->spatialNode);
    if (!cameraNode)
        return QVector3D(0, 0, 0);

    QVector4D scenePosRightHand(scenePos, 1);

    // Transform position
    const QMatrix4x4 sceneToCamera = sceneTransform().inverted();
    const QMatrix4x4 projectionViewMatrix = cameraNode->projection * sceneToCamera;
    const QVector4D transformedScenePos = QSSGUtils::mat44::transform(projectionViewMatrix, scenePosRightHand);

    if (qFuzzyIsNull(transformedScenePos.w()) || qIsNaN(transformedScenePos.w()))
        return QVector3D(0, 0, 0);

    // Normalize scenePosView between [-1, 1]
    QVector3D scenePosView = transformedScenePos.toVector3D() / transformedScenePos.w();

    // Set z to be the scene distance from clipNear so that the return value
    // can be used as argument to viewportToscene() to reverse the call.
    const QVector4D clipNearPos(scenePosView.x(), scenePosView.y(), -1, 1);
    auto invProj = projectionViewMatrix.inverted();
    const QVector4D clipNearPosTransformed = QSSGUtils::mat44::transform(invProj, clipNearPos);
    if (qFuzzyIsNull(clipNearPosTransformed.w()) || qIsNaN(clipNearPosTransformed.w()))
        return QVector3D(0, 0, 0);
    const QVector4D clipNearPosScene = clipNearPosTransformed / clipNearPosTransformed.w();
    QVector4D clipFarPos = clipNearPos;
    clipFarPos.setZ(0);
    const QVector4D clipFarPosTransformed = QSSGUtils::mat44::transform(invProj, clipFarPos);
    if (qFuzzyIsNull(clipFarPosTransformed.w()) || qIsNaN(clipFarPosTransformed.w()))
        return QVector3D(0, 0, 0);
    const QVector4D clipFarPosScene = clipFarPosTransformed / clipFarPosTransformed.w();
    const QVector3D direction = (clipFarPosScene - clipNearPosScene).toVector3D();
    const QVector3D scenePosVec = (scenePosRightHand - clipNearPosScene).toVector3D();

    const float distanceToScenePos = scenePosVec.length()
            * (QVector3D::dotProduct(direction, scenePosVec) > 0.f ? 1 : -1);

    scenePosView.setZ(distanceToScenePos);

    // Convert x and y to be between [0, 1]
    scenePosView.setX((scenePosView.x() / 2) + 0.5f);
    scenePosView.setY((scenePosView.y() / 2) + 0.5f);
    // And convert origin from bottom-left to top-left
    scenePosView.setY(1 - scenePosView.y());
    return scenePosView;
}

/*!
    \qmlmethod vector3d Camera::mapFromViewport(vector3d viewportPos)

    Transforms \a viewportPos from viewport space (2D) into global scene space (3D).

    The x- and y-values of \a viewportPos must be normalized, with the top-left
    of the viewport at [0, 0] and the bottom-right at [1, 1]. The z-value is interpreted
    as the distance from the near clip plane of the frustum (clipNear).

    If \a viewportPos cannot successfully be mapped to a position in the scene, a position
    of [0, 0, 0] is returned.

    \sa mapToViewport, {View3D::mapTo3DScene()}{View3D.mapTo3DScene()}
*/
QVector3D QQuick3DCamera::mapFromViewport(const QVector3D &viewportPos) const
{
    QSSGRenderCamera *cameraNode = static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::get(this)->spatialNode);
    if (!cameraNode)
        return QVector3D(0, 0, 0);

    // Pick two positions in the frustum
    QVector4D clipNearPos(viewportPos, 1);
    // Convert origin from top-left to bottom-left
    clipNearPos.setY(1 - clipNearPos.y());
    // Convert to homogenous position between [-1, 1]
    clipNearPos.setX((clipNearPos.x() * 2.0f) - 1.0f);
    clipNearPos.setY((clipNearPos.y() * 2.0f) - 1.0f);
    QVector4D clipFarPos = clipNearPos;
    // clipNear: z = -1, clipFar: z = 1. It's recommended to use 0 as
    // far pos instead of clipFar because of infinite projection issues.
    clipNearPos.setZ(-1);
    clipFarPos.setZ(0);

    // Transform position to scene
    const QMatrix4x4 sceneToCamera = sceneTransform().inverted();
    const QMatrix4x4 projectionViewMatrixInv = (cameraNode->projection * sceneToCamera).inverted();
    const QVector4D transformedClipNearPos = QSSGUtils::mat44::transform(projectionViewMatrixInv, clipNearPos);
    const QVector4D transformedClipFarPos = QSSGUtils::mat44::transform(projectionViewMatrixInv, clipFarPos);

    if (qFuzzyIsNull(transformedClipNearPos.w()) || qIsNaN(transformedClipNearPos.w()) ||
        qFuzzyIsNull(transformedClipFarPos.w()) || qIsNaN(transformedClipFarPos.w()))
        return QVector3D(0, 0, 0);

    // Reverse the projection
    const QVector3D clipNearPosScene = transformedClipNearPos.toVector3D()
            / transformedClipNearPos.w();
    const QVector3D clipFarPosScene = transformedClipFarPos.toVector3D()
            / transformedClipFarPos.w();

    // Calculate the position in the scene
    const QVector3D direction = (clipFarPosScene - clipNearPosScene).normalized();
    const float distanceFromClipNear = viewportPos.z();
    QVector3D scenePos = clipNearPosScene + (direction * distanceFromClipNear);

    return scenePos;
}

/*!
 * \internal
 */
QVector3D QQuick3DCamera::mapToViewport(const QVector3D &scenePos,
                                        qreal width,
                                        qreal height)
{
    // We can be called before a spatial node is created, if that is the case, create the node now.
    if (QSSGRenderCamera *cameraNode = static_cast<QSSGRenderCamera *>(updateSpatialNode(QQuick3DObjectPrivate::get(this)->spatialNode))) {
        QQuick3DObjectPrivate::get(this)->spatialNode = cameraNode;
        QSSGRenderCamera::Configuration camConfig { cameraNode->getDpr(), 1.0f /* ssaa multiplier */ };
        QSSGRenderCamera::calculateProjectionInternal(*cameraNode, QRect(0, 0, width * camConfig.dpr, height * camConfig.dpr), camConfig);
    }

    return QQuick3DCamera::mapToViewport(scenePos);
}

/*!
 * \internal
 */
QVector3D QQuick3DCamera::mapFromViewport(const QVector3D &viewportPos,
                                          qreal width,
                                          qreal height)
{
    // We can be called before a spatial node is created, if that is the case, create the node now.
    if (QSSGRenderCamera *cameraNode = static_cast<QSSGRenderCamera *>(updateSpatialNode(QQuick3DObjectPrivate::get(this)->spatialNode))) {
        QQuick3DObjectPrivate::get(this)->spatialNode = cameraNode;
        QSSGRenderCamera::Configuration camConfig { cameraNode->getDpr(), 1.0f /* ssaa multiplier */ };
        QSSGRenderCamera::calculateProjectionInternal(*cameraNode, QRect(0, 0, width * camConfig.dpr, height * camConfig.dpr));
    }

    return QQuick3DCamera::mapFromViewport(viewportPos);
}

/*!
    \qmlmethod vector3d Camera::lookAt(vector3d scenePos)
    \since 5.15

    Sets the rotation value of the Camera so that it is pointing at \a scenePos.
*/

void QQuick3DCamera::lookAt(const QVector3D &scenePos)
{
    // Assumption: we never want the camera to roll.
    // We use Euler angles here to avoid roll to sneak in through numerical instability.

    const auto &targetPosition = scenePos;
    auto sourcePosition = scenePosition();

    QVector3D targetVector = sourcePosition - targetPosition;

    float yaw = qRadiansToDegrees(atan2(targetVector.x(), targetVector.z()));

    QVector2D p(targetVector.x(), targetVector.z()); // yaw vector projected to horizontal plane
    float pitch = qRadiansToDegrees(atan2(p.length(), targetVector.y())) - 90;

    const float previousRoll = eulerRotation().z();
    setEulerRotation(QVector3D(pitch, yaw, previousRoll));
}

/*!
    \qmlmethod vector3d Camera::lookAt(QtQuick3D::Node node)
    \since 5.15

    Sets the rotation value of the Camera so that it is pointing at \a node.
*/

void QQuick3DCamera::lookAt(QQuick3DNode *node)
{
    if (!node)
        return;
    lookAt(node->scenePosition());
}

void QQuick3DCamera::updateGlobalVariables(const QRectF &inViewport)
{
    QSSGRenderCamera *node = static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::get(this)->spatialNode);
    if (node)
        QSSGRenderCamera::calculateProjectionInternal(*node, inViewport);
}

/*!
 * \internal
 */
QSSGRenderGraphObject *QQuick3DCamera::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderCamera(QQuick3DObjectPrivate::get(this)->type);
    }

    QQuick3DNode::updateSpatialNode(node);

    QSSGRenderCamera *camera = static_cast<QSSGRenderCamera *>(node);
    if (qUpdateIfNeeded(camera->enableFrustumClipping, m_frustumCullingEnabled))
        camera->markDirty(QSSGRenderCamera::DirtyFlag::CameraDirty);
    if (qUpdateIfNeeded(camera->levelOfDetailPixelThreshold, m_levelOfDetailBias))
        camera->markDirty(QSSGRenderCamera::DirtyFlag::CameraDirty);
    if (qUpdateIfNeeded(camera->layerMask, quint32(QQuick3DNodePrivate::get(this)->m_tag)))
        camera->markDirty(QSSGRenderCamera::DirtyFlag::LayerMaskDirty);

    return node;
}

void QQuick3DCamera::updateLookAt()
{
    if (m_lookAtNode)
        lookAt(m_lookAtNode);
}

/*!
    \qmlproperty real Camera::levelOfDetailBias
    \since 6.5

    This property changes the size a model needs to be when rendered before the
    automatic level of detail meshes are used. Each generated level of detail
    mesh contains an ideal size value that each level should be shown, which is
    a ratio of how much of the rendered scene will be that mesh. A model that
    represents only a few pixels on screen will not require the full geometry
    to look correct, so a lower level of detail mesh will be used instead in
    this case. This value is a bias to the ideal value such that a value smaller
    than \c 1.0 will require an even smaller rendered size before switching to
    a lesser level of detail. Values above \c 1.0 will lead to lower levels of detail
    being used sooner.  A value of \c 0.0 will disable the usage of levels of detail
    completely.

    The default value is \c 1.0

    \note This property will only have an effect on Models with geomtry containing
    levels of detail.

    \sa Model::levelOfDetailBias
*/

float QQuick3DCamera::levelOfDetailBias() const
{
    return m_levelOfDetailBias;
}

void QQuick3DCamera::setLevelOfDetailBias(float newLevelOfDetailBias)
{
    if (qFuzzyCompare(m_levelOfDetailBias, newLevelOfDetailBias))
        return;
    m_levelOfDetailBias = newLevelOfDetailBias;
    emit levelOfDetailBiasChanged();
    update();
}

QT_END_NAMESPACE

