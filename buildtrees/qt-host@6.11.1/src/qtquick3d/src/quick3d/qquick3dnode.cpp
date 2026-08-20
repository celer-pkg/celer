// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dnode_p.h"
#include "qquick3dnode_p_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3D/private/qquick3dobject_p.h>

#include <QtMath>

QT_BEGIN_NAMESPACE

QQuick3DNodePrivate::QQuick3DNodePrivate(QQuick3DObjectPrivate::Type t,
                                         QQuick3DContentLayer::LayerFlag layerFlag)
    : QQuick3DObjectPrivate(t)
    , m_tag(layerFlag)
{

}

QQuick3DNodePrivate::~QQuick3DNodePrivate()
{

}

void QQuick3DNodePrivate::init()
{

}

void QQuick3DNodePrivate::setIsHiddenInEditor(bool isHidden)
{
    Q_Q(QQuick3DNode);
    if (isHidden == m_isHiddenInEditor)
        return;
    m_isHiddenInEditor = isHidden;
    q->update();
}

void QQuick3DNodePrivate::setLocalTransform(const QMatrix4x4 &transform)
{
    Q_Q(QQuick3DNode);

    // decompose the 4x4 affine transform into scale, rotation and translation
    QVector3D scale;
    QQuaternion rotation;
    QVector3D position;

    if (QSSGUtils::mat44::decompose(transform, position, scale, rotation)) {
        q->setScale(scale);
        q->setRotation(rotation);
        q->setPosition(position);
    }

    // We set the local transform as-is regardless if it could be decomposed or not.
    // We'd likely want some way to notify about this, but for now the transform
    // can potentially change silently.
    m_localTransform = transform;
    // Note: If any of the transform properties are set before the update
    // the explicit local transform should be ignored by setting this value to false.
    m_hasExplicitLocalTransform = true;

    q->update();
}

/*!
    \qmltype Node
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief The base component for an object that exists in a 3D scene.

    The Node type serves as the base class for other spatial types, such as, \l Model, \l Camera, \l Light.
    These objects represent an entity that exists in the 3D scene, due to having a position and
    other properties in the 3D world. With the exception of the root node(s), all Node types are
    transformed relative to their parent Node, that is, in local coordinates. In many ways the Node
    type serves the same purpose in Qt Quick 3D scenes as \l Item does for Qt Quick scenes.

    In addition to types deriving from Node, it is also possible to parent other types to
    a Node.  This includes QObject instances, where the Node merely serves as the
    \l{QObject::parent()}{QObject parent}, and \l{Qt Quick 3D Scenes with 2D Content}{Qt
    Quick items}.

    Wrapping other objects for the purpose of grouping them into components or sub-trees can be
    a convenient way to, for example, animated a group of nodes as a whole. This snippet shows how
    to use Node to animate a camera:

    \qml
    Node {
        PerspectiveCamera {
            position: Qt.vector3d(0, 0, -600)
        }

        SequentialAnimation on eulerRotation.y {
            loops: Animation.Infinite
            PropertyAnimation {
                duration: 5000
                from: 0
                to: 360
            }
        }
    }
    \endqml

    Node has to be used also if creating a scene outside of \l View3D, for example for the
    purpose of switching scenes on the fly, or showing the same scene on multiple views.

    \qml
    Node {
        id: standAloneScene

        DirectionalLight {}

        Model {
            source: "#Sphere"
            materials: [ DefaultMaterial {} ]
        }

        PerspectiveCamera {
            z: 600
        }
    }

    View3D {
        importScene: standAloneScene
    }
    \endqml
*/

QQuick3DNode::QQuick3DNode(QQuick3DNode *parent)
    : QQuick3DObject(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::Node)), parent)
{
    Q_D(QQuick3DNode);
    d->init();
}

QQuick3DNode::QQuick3DNode(QQuick3DNodePrivate &dd, QQuick3DNode *parent)
    : QQuick3DObject(dd, parent)
{
    Q_ASSERT_X(QSSGRenderGraphObject::isNodeType(dd.type), "", "Type needs to be identified as a node type!");

    Q_D(QQuick3DNode);
    d->init();
}

QQuick3DNode::~QQuick3DNode() {}

/*!
    \qmlproperty real QtQuick3D::Node::x

    This property contains the x value of the position translation in
    local coordinate space.

    \sa position
*/
float QQuick3DNode::x() const
{
    Q_D(const QQuick3DNode);
    return d->m_position.x();
}

/*!
    \qmlproperty real QtQuick3D::Node::y

    This property contains the y value of the position translation in
    local coordinate space.

    \sa position
*/
float QQuick3DNode::y() const
{
    Q_D(const QQuick3DNode);
    return d->m_position.y();
}

/*!
    \qmlproperty real QtQuick3D::Node::z

    This property contains the z value of the position translation in
    local coordinate space.

    \sa position
*/
float QQuick3DNode::z() const
{
    Q_D(const QQuick3DNode);
    return d->m_position.z();
}

/*!
    \qmlproperty quaternion QtQuick3D::Node::rotation

    This property contains the rotation values for the node.
    These values are stored as a quaternion.
*/
QQuaternion QQuick3DNode::rotation() const
{
    Q_D(const QQuick3DNode);
    return d->m_rotation;
}

/*!
    \qmlproperty vector3d QtQuick3D::Node::position

    This property contains the position translation in local coordinate space.

    \sa x, y, z
*/
QVector3D QQuick3DNode::position() const
{
    Q_D(const QQuick3DNode);
    return d->m_position;
}


/*!
    \qmlproperty vector3d QtQuick3D::Node::scale

    This property contains the scale values for the x, y, and z axis.
*/
QVector3D QQuick3DNode::scale() const
{
    Q_D(const QQuick3DNode);
    return d->m_scale;
}

/*!
    \qmlproperty vector3d QtQuick3D::Node::pivot

    This property contains the pivot values for the x, y, and z axis.  These
    values are used as the pivot points when applying rotations to the node.

*/
QVector3D QQuick3DNode::pivot() const
{
    Q_D(const QQuick3DNode);
    return d->m_pivot;
}

/*!
    \qmlproperty real QtQuick3D::Node::opacity

    This property contains the local opacity value of the Node.  Since Node
    objects are not necessarily visible, this value might not have any effect,
    but this value is inherited by all children of the Node, which might be visible.

*/
float QQuick3DNode::localOpacity() const
{
    Q_D(const QQuick3DNode);
    return d->m_opacity;
}

/*!
    \qmlproperty bool QtQuick3D::Node::visible

    When this property is true, the Node (and its children) can be visible.

*/
bool QQuick3DNode::visible() const
{
    Q_D(const QQuick3DNode);
    return d->m_visible;
}

/*!
    \qmlproperty int QtQuick3D::Node::staticFlags
    \since 5.15

    This property defines the static flags that are used to evaluate how the node is rendered.
    Currently doesn't do anything but act as a placeholder for a future implementation.
*/
int QQuick3DNode::staticFlags() const
{
    Q_D(const QQuick3DNode);
    return d->m_staticFlags;
}

QQuick3DNode *QQuick3DNode::parentNode() const
{
    // The parent of a QQuick3DNode should never be anything else than a (subclass
    // of) QQuick3DNode (but the children/leaf nodes can be something else).
    return static_cast<QQuick3DNode *>(parentItem());
}

/*!
    \qmlproperty vector3d QtQuick3D::Node::forward
    \readonly

    This property returns a normalized vector of the nodes forward direction
    in scene space.

    \sa up, right, mapDirectionToScene
*/
QVector3D QQuick3DNode::forward() const
{
    return mapDirectionToScene(QVector3D(0, 0, -1)).normalized();
}

/*!
    \qmlproperty vector3d QtQuick3D::Node::up
    \readonly

    This property returns a normalized vector of the nodes up direction
    in scene space.

    \sa forward, right, mapDirectionToScene
*/
QVector3D QQuick3DNode::up() const
{
    return mapDirectionToScene(QVector3D(0, 1, 0)).normalized();
}

/*!
    \qmlproperty vector3d QtQuick3D::Node::right
    \readonly

    This property returns a normalized vector of the nodes right direction
    in scene space.

    \sa forward, up, mapDirectionToScene
*/
QVector3D QQuick3DNode::right() const
{
    return mapDirectionToScene(QVector3D(1, 0, 0)).normalized();
}
/*!
    \qmlproperty vector3d QtQuick3D::Node::scenePosition
    \readonly

    This property returns the position of the node in scene space.

    \note This is sometimes also referred to as the global position. But
    then in the meaning "global in the 3D world", and not "global to the
    screen or desktop" (which is usually the interpretation in other Qt APIs).
    \note the position will be reported in the same orientation as the node.

    \sa mapPositionToScene
*/
QVector3D QQuick3DNode::scenePosition() const
{
    return QSSGUtils::mat44::getPosition(sceneTransform());
}

/*!
    \qmlproperty quaternion QtQuick3D::Node::sceneRotation
    \readonly

    This property returns the rotation of the node in scene space.
*/
QQuaternion QQuick3DNode::sceneRotation() const
{
    Q_D(const QQuick3DNode);
    return QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(d->sceneRotationMatrix())).normalized();
}

/*!
    \qmlproperty vector3d QtQuick3D::Node::sceneScale
    \readonly

    This property returns the scale of the node in scene space.
*/
QVector3D QQuick3DNode::sceneScale() const
{
    return QSSGUtils::mat44::getScale(sceneTransform());
}

/*!
    \qmlproperty matrix4x4 QtQuick3D::Node::sceneTransform
    \readonly

    This property returns the global transform matrix for this node.
    \note the return value will be in right-handed coordinates.
*/
QMatrix4x4 QQuick3DNode::sceneTransform() const
{
    Q_D(const QQuick3DNode);
    if (d->m_sceneTransformDirty)
        const_cast<QQuick3DNodePrivate *>(d)->calculateGlobalVariables();
    return d->m_sceneTransform;
}

void QQuick3DNodePrivate::calculateGlobalVariables()
{
    Q_Q(QQuick3DNode);
    m_sceneTransformDirty = false;
    QMatrix4x4 localTransform = QSSGRenderNode::calculateTransformMatrix(m_position, m_scale, m_pivot, m_rotation);
    QQuick3DNode *parent = q->parentNode();
    if (!parent) {
        m_sceneTransform = localTransform;
        m_hasInheritedUniformScale = true;
        return;
    }
    QQuick3DNodePrivate *privateParent = QQuick3DNodePrivate::get(parent);

    if (privateParent->m_sceneTransformDirty)
        privateParent->calculateGlobalVariables();
    m_sceneTransform = privateParent->m_sceneTransform * localTransform;

    // Check if we have an ancestor with non-uniform scale. This will decide whether
    // or not we can use the sceneTransform to extract sceneRotation and sceneScale.
    m_hasInheritedUniformScale = privateParent->m_hasInheritedUniformScale;
    if (m_hasInheritedUniformScale) {
        const QVector3D ps = privateParent->m_scale;
        m_hasInheritedUniformScale = qFuzzyCompare(ps.x(), ps.y()) && qFuzzyCompare(ps.x(), ps.z());
    }
}

QMatrix4x4 QQuick3DNodePrivate::localRotationMatrix() const
{
    return QMatrix4x4(m_rotation.toRotationMatrix());
}

QMatrix4x4 QQuick3DNodePrivate::sceneRotationMatrix() const
{
    Q_Q(const QQuick3DNode);

    if (m_sceneTransformDirty) {
        // Ensure m_hasInheritedUniformScale is up to date
        const_cast<QQuick3DNodePrivate *>(this)->calculateGlobalVariables();
    }

    if (m_hasInheritedUniformScale) {
        // When we know that every node up to the root have a uniform scale, we can extract the
        // rotation directly from the sceneTransform(). This is optimizing, since we reuse that
        // matrix for more than just calculating the sceneRotation.
        QMatrix4x4 rotationMatrix = q->sceneTransform();
        QSSGUtils::mat44::normalize(rotationMatrix);
        return rotationMatrix;
    }

    // When we have an ancestor that has a non-uniform scale, we cannot extract
    // the rotation from the sceneMatrix directly. Instead, we need to calculate
    // it separately, which is slightly more costly.
    const QMatrix4x4 parentRotationMatrix = QQuick3DNodePrivate::get(q->parentNode())->sceneRotationMatrix();
    return parentRotationMatrix * localRotationMatrix();
}

void QQuick3DNodePrivate::emitChangesToSceneTransform()
{
    Q_Q(QQuick3DNode);
    const QVector3D prevPosition = QSSGUtils::mat44::getPosition(m_sceneTransform);
    const QQuaternion prevRotation = QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(m_sceneTransform)).normalized();
    const QVector3D prevScale = QSSGUtils::mat44::getScale(m_sceneTransform);
    QVector3D prevForward, prevUp, prevRight;
    QVector3D newForward, newUp, newRight;
    // Do direction (forward, up, right) calculations only if they have connections
    bool emitDirectionChanges = (m_directionConnectionCount > 0);
    if (emitDirectionChanges) {
        // Instead of calling forward(), up() and right(), calculate them here.
        // This way m_sceneTransform isn't updated due to m_sceneTransformDirty and
        // common theDirMatrix operations are not duplicated.
        QMatrix3x3 theDirMatrix = m_sceneTransform.normalMatrix();
        prevForward = QSSGUtils::mat33::transform(theDirMatrix, QVector3D(0, 0, -1)).normalized();
        prevUp = QSSGUtils::mat33::transform(theDirMatrix, QVector3D(0, 1, 0)).normalized();
        prevRight = QSSGUtils::mat33::transform(theDirMatrix, QVector3D(1, 0, 0)).normalized();
    }

    calculateGlobalVariables();

    const QVector3D newPosition = QSSGUtils::mat44::getPosition(m_sceneTransform);
    const QQuaternion newRotation = QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(m_sceneTransform)).normalized();
    const QVector3D newScale = QSSGUtils::mat44::getScale(m_sceneTransform);
    if (emitDirectionChanges) {
        QMatrix3x3 theDirMatrix = m_sceneTransform.normalMatrix();
        newForward = QSSGUtils::mat33::transform(theDirMatrix, QVector3D(0, 0, -1)).normalized();
        newUp = QSSGUtils::mat33::transform(theDirMatrix, QVector3D(0, 1, 0)).normalized();
        newRight = QSSGUtils::mat33::transform(theDirMatrix, QVector3D(1, 0, 0)).normalized();
    }

    const bool positionChanged = prevPosition != newPosition;
    const bool rotationChanged = prevRotation != newRotation;
    const bool scaleChanged = !qFuzzyCompare(prevScale, newScale);

    if (!positionChanged && !rotationChanged && !scaleChanged)
        return;

    emit q->sceneTransformChanged();

    if (positionChanged)
        emit q->scenePositionChanged();
    if (rotationChanged)
        emit q->sceneRotationChanged();
    if (scaleChanged)
        emit q->sceneScaleChanged();
    if (emitDirectionChanges) {
        const bool forwardChanged = prevForward != newForward;
        const bool upChanged = prevUp != newUp;
        const bool rightChanged = prevRight != newRight;
        if (forwardChanged)
            Q_EMIT q->forwardChanged();
        if (upChanged)
            Q_EMIT q->upChanged();
        if (rightChanged)
            Q_EMIT q->rightChanged();
    }
}

bool QQuick3DNodePrivate::isSceneTransformRelatedSignal(const QMetaMethod &signal) const
{
    // Return true if its likely that we need to emit
    // the given signal when our global transform changes.
    static const QMetaMethod sceneTransformSignal = QMetaMethod::fromSignal(&QQuick3DNode::sceneTransformChanged);
    static const QMetaMethod scenePositionSignal = QMetaMethod::fromSignal(&QQuick3DNode::scenePositionChanged);
    static const QMetaMethod sceneRotationSignal = QMetaMethod::fromSignal(&QQuick3DNode::sceneRotationChanged);
    static const QMetaMethod sceneScaleSignal = QMetaMethod::fromSignal(&QQuick3DNode::sceneScaleChanged);

    return (signal == sceneTransformSignal
            || signal == scenePositionSignal
            || signal == sceneRotationSignal
            || signal == sceneScaleSignal);
}

bool QQuick3DNodePrivate::isDirectionRelatedSignal(const QMetaMethod &signal) const
{
    // Return true if its likely that we need to emit
    // the given signal when our global transform changes.
    static const QMetaMethod forwardSignal = QMetaMethod::fromSignal(&QQuick3DNode::forwardChanged);
    static const QMetaMethod upSignal = QMetaMethod::fromSignal(&QQuick3DNode::upChanged);
    static const QMetaMethod rightSignal = QMetaMethod::fromSignal(&QQuick3DNode::rightChanged);

    return (signal == forwardSignal
            || signal == upSignal
            || signal == rightSignal);
}

void QQuick3DNode::connectNotify(const QMetaMethod &signal)
{
    Q_D(QQuick3DNode);
    // Since we want to avoid calculating the global transform in the frontend
    // unnecessary, we keep track of the number of connections/QML bindings
    // that needs it. If there are no connections, we can skip calculating it
    // whenever our geometry changes (unless someone asks for it explicitly).
    if (d->isSceneTransformRelatedSignal(signal))
        d->m_sceneTransformConnectionCount++;
    if (d->isDirectionRelatedSignal(signal))
        d->m_directionConnectionCount++;
}

void QQuick3DNode::disconnectNotify(const QMetaMethod &signal)
{
    Q_D(QQuick3DNode);
    if (d->isSceneTransformRelatedSignal(signal))
        d->m_sceneTransformConnectionCount--;
    if (d->isDirectionRelatedSignal(signal))
        d->m_directionConnectionCount--;
}

void QQuick3DNode::componentComplete()
{
    Q_D(QQuick3DNode);
    QQuick3DObject::componentComplete();
    if (d->m_sceneTransformConnectionCount > 0 || d->m_directionConnectionCount > 0)
        d->emitChangesToSceneTransform();
}

void QQuick3DNodePrivate::markSceneTransformDirty()
{
    Q_Q(QQuick3DNode);
    // Note: we recursively set m_sceneTransformDirty to true whenever our geometry
    // changes. But we only set it back to false if someone actually queries our global
    // transform (because only then do we need to calculate it). This means that if no
    // one ever does that, m_sceneTransformDirty will remain true, perhaps through out
    // the life time of the node. This is in contrast with the backend, which need to
    // update dirty transform nodes for every scene graph sync (and clear the backend
    // dirty transform flags - QQuick3DObjectPrivate::dirtyAttributes).
    // This means that for most nodes, calling markSceneTransformDirty() should be
    // cheap, since we normally expect to return early in the following test.
    if (m_sceneTransformDirty)
        return;

    m_sceneTransformDirty = true;

    if (m_sceneTransformConnectionCount > 0 || m_directionConnectionCount > 0)
        emitChangesToSceneTransform();

    auto children = QQuick3DObjectPrivate::get(q)->childItems;
    for (auto child : children) {
        if (auto node = qobject_cast<QQuick3DNode *>(child)) {
            QQuick3DNodePrivate::get(node)->markSceneTransformDirty();
        }
    }
}

void QQuick3DNode::setX(float x)
{
    Q_D(QQuick3DNode);
    if (qFuzzyCompare(d->m_position.x(), x))
        return;

    d->m_position.setX(x);
    d->markSceneTransformDirty();
    emit positionChanged();
    emit xChanged();
    update();
}

void QQuick3DNode::setY(float y)
{
    Q_D(QQuick3DNode);
    if (qFuzzyCompare(d->m_position.y(), y))
        return;

    d->m_position.setY(y);
    d->markSceneTransformDirty();
    emit positionChanged();
    emit yChanged();
    update();
}

void QQuick3DNode::setZ(float z)
{
    Q_D(QQuick3DNode);
    if (qFuzzyCompare(d->m_position.z(), z))
        return;

    d->m_position.setZ(z);
    d->markSceneTransformDirty();
    emit positionChanged();
    emit zChanged();
    update();
}

void QQuick3DNode::setRotation(const QQuaternion &rotation)
{
    Q_D(QQuick3DNode);
    if (d->m_rotation == rotation)
        return;

    d->m_hasExplicitLocalTransform = false;
    d->m_rotation = rotation;
    d->markSceneTransformDirty();
    emit rotationChanged();
    emit eulerRotationChanged();

    update();
}

void QQuick3DNode::setPosition(const QVector3D &position)
{
    Q_D(QQuick3DNode);
    if (d->m_position == position)
        return;

    const bool xUnchanged = qFuzzyCompare(position.x(), d->m_position.x());
    const bool yUnchanged = qFuzzyCompare(position.y(), d->m_position.y());
    const bool zUnchanged = qFuzzyCompare(position.z(), d->m_position.z());

    d->m_position = position;
    d->markSceneTransformDirty();
    emit positionChanged();

    if (!xUnchanged)
        emit xChanged();
    if (!yUnchanged)
        emit yChanged();
    if (!zUnchanged)
        emit zChanged();

    d->m_hasExplicitLocalTransform = false;

    update();
}

void QQuick3DNode::setScale(const QVector3D &scale)
{
    Q_D(QQuick3DNode);
    if (d->m_scale == scale)
        return;

    d->m_hasExplicitLocalTransform = false;
    d->m_scale = scale;
    d->markSceneTransformDirty();
    emit scaleChanged();
    update();
}

void QQuick3DNode::setPivot(const QVector3D &pivot)
{
    Q_D(QQuick3DNode);
    if (d->m_pivot == pivot)
        return;

    d->m_hasExplicitLocalTransform = false;
    d->m_pivot = pivot;
    d->markSceneTransformDirty();
    emit pivotChanged();
    update();
}

void QQuick3DNode::setLocalOpacity(float opacity)
{
    Q_D(QQuick3DNode);
    if (qFuzzyCompare(d->m_opacity, opacity))
        return;

    d->m_opacity = opacity;
    emit localOpacityChanged();
    update();
}

void QQuick3DNode::setVisible(bool visible)
{
    Q_D(QQuick3DNode);
    if (d->m_visible == visible)
        return;

    d->m_visible = visible;
    emit visibleChanged();
    update();
}

void QQuick3DNode::setStaticFlags(int staticFlags)
{
    Q_D(QQuick3DNode);
    if (d->m_staticFlags == staticFlags)
        return;

    d->m_staticFlags = staticFlags;
    emit staticFlagsChanged();
    update();
}

void QQuick3DNode::setEulerRotation(const QVector3D &eulerRotation) {
    Q_D(QQuick3DNode);

    if (d->m_rotation == eulerRotation)
        return;

    d->m_hasExplicitLocalTransform = false;
    d->m_rotation = eulerRotation;

    emit rotationChanged();
    d->markSceneTransformDirty();
    emit eulerRotationChanged();
    update();
}

/*!
    \qmlmethod void QtQuick3D::Node::rotate(real degrees, vector3d axis, enumeration space)

    Rotates this node around an \a axis by the given \a degrees. The specified
    rotation will be added to the node's current rotation. The axis can
    be specified relative to different \a {space}s.

    \value Node.LocalSpace
           Axis is relative to the local orientation of this node.
    \value Node.ParentSpace
           Axis is relative to the local orientation of the parent node.
    \value Node.SceneSpace
           Axis is relative to the scene.

*/
void QQuick3DNode::rotate(qreal degrees, const QVector3D &axis, TransformSpace space)
{
    Q_D(QQuick3DNode);

    const QQuaternion addRotationQuat = QQuaternion::fromAxisAndAngle(axis, float(degrees));
    const QMatrix4x4 addRotationMatrix = QMatrix4x4(addRotationQuat.toRotationMatrix());
    QMatrix4x4 newRotationMatrix;

    switch (space) {
    case LocalSpace:
        newRotationMatrix = d->localRotationMatrix() * addRotationMatrix;
        break;
    case ParentSpace:
        newRotationMatrix = addRotationMatrix * d->localRotationMatrix();
        break;
    case SceneSpace:
        if (const auto parent = parentNode()) {
            const QMatrix4x4 lrm = d->localRotationMatrix();
            const QMatrix4x4 prm = QQuick3DNodePrivate::get(parent)->sceneRotationMatrix();
            newRotationMatrix = prm.inverted() * addRotationMatrix * prm * lrm;
        } else {
            newRotationMatrix = d->localRotationMatrix() * addRotationMatrix;
        }
        break;
    }

    const QQuaternion newRotationQuaternion = QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(newRotationMatrix)).normalized();

    if (d->m_rotation == newRotationQuaternion)
        return;

    d->m_hasExplicitLocalTransform = false;
    d->m_rotation = newRotationQuaternion;
    d->markSceneTransformDirty();

    emit rotationChanged();
    emit eulerRotationChanged();

    update();
}

QSSGRenderGraphObject *QQuick3DNode::updateSpatialNode(QSSGRenderGraphObject *node)
{
    Q_D(QQuick3DNode);
    if (!node) {
        markAllDirty();
        node = new QSSGRenderNode();
    }
    QQuick3DObject::updateSpatialNode(node);
    auto spacialNode = static_cast<QSSGRenderNode *>(node);
    bool transformIsDirty = false;

    if (spacialNode->pivot != d->m_pivot) {
        transformIsDirty = true;
        spacialNode->pivot = d->m_pivot;
    }

    if (!qFuzzyCompare(spacialNode->localOpacity, d->m_opacity)) {
        spacialNode->localOpacity = d->m_opacity;
        spacialNode->markDirty(QSSGRenderNode::DirtyFlag::OpacityDirty);
    }

    if (d->m_hasExplicitLocalTransform) {
        spacialNode->localTransform = d->m_localTransform;
        spacialNode->markDirty(QSSGRenderNode::DirtyFlag::TransformDirty);
        d->m_hasExplicitLocalTransform = false;
    } else {
        if (!transformIsDirty && !qFuzzyCompare(d->m_position, QSSGUtils::mat44::getPosition(spacialNode->localTransform)))
            transformIsDirty = true;

        if (!transformIsDirty && !qFuzzyCompare(d->m_scale, QSSGUtils::mat44::getScale(spacialNode->localTransform)))
            transformIsDirty = true;

        if (!transformIsDirty && !qFuzzyCompare(d->m_rotation, QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(spacialNode->localTransform))))
            transformIsDirty = true;

        if (transformIsDirty) {
            spacialNode->localTransform = QSSGRenderNode::calculateTransformMatrix(d->m_position, d->m_scale, d->m_pivot, d->m_rotation);
            spacialNode->markDirty(QSSGRenderNode::DirtyFlag::TransformDirty);
        }
    }

    if (d->m_tag != spacialNode->tag) {
        spacialNode->tag = d->m_tag;
        spacialNode->markDirty(QSSGRenderNode::DirtyFlag::TagDirty);
    }

    spacialNode->staticFlags = d->m_staticFlags;

    // The Hidden in Editor flag overrides the visible value
    if (d->m_isHiddenInEditor)
        spacialNode->setState(QSSGRenderNode::LocalState::Active, false);
    else
        spacialNode->setState(QSSGRenderNode::LocalState::Active, d->m_visible);

    DebugViewHelpers::ensureDebugObjectName(spacialNode, this);

    return spacialNode;
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapPositionToScene(vector3d localPosition)

    Transforms \a localPosition from local space to scene space.

    \note "Scene space" is sometimes also referred to as the "global space". But
    then in the meaning "global in the 3D world", and not "global to the
    screen or desktop" (which is usually the interpretation in other Qt APIs).

    \sa mapPositionFromScene, mapPositionToNode, mapPositionFromNode
*/
QVector3D QQuick3DNode::mapPositionToScene(const QVector3D &localPosition) const
{
    return QSSGUtils::mat44::transform(sceneTransform(), localPosition);
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapPositionFromScene(vector3d scenePosition)

    Transforms \a scenePosition from scene space to local space.

    \sa mapPositionToScene, mapPositionToNode, mapPositionFromNode
*/
QVector3D QQuick3DNode::mapPositionFromScene(const QVector3D &scenePosition) const
{
    return QSSGUtils::mat44::transform(sceneTransform().inverted(), scenePosition);
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapPositionToNode(QtQuick3D::Node node, vector3d localPosition)

    Transforms \a localPosition from the local space of this node to
    the local space of \a node.

    \note If \a node is null, then \a localPosition will be transformed into scene space coordinates.

    \sa mapPositionToScene, mapPositionFromScene, mapPositionFromNode
*/
QVector3D QQuick3DNode::mapPositionToNode(const QQuick3DNode *node, const QVector3D &localPosition) const
{
    const auto scenePositionSelf = mapPositionToScene(localPosition);
    return node ? node->mapPositionFromScene(scenePositionSelf) : scenePositionSelf;
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapPositionFromNode(QtQuick3D::Node node, vector3d localPosition)

    Transforms \a localPosition from the local space of \a node to
    the local space of this node.

    \note If \a node is null, then \a localPosition is interpreted as it is in scene space coordinates.

    \sa mapPositionToScene, mapPositionFromScene, mapPositionToNode
*/
QVector3D QQuick3DNode::mapPositionFromNode(const QQuick3DNode *node, const QVector3D &localPosition) const
{
    const auto scenePositionOther = node ? node->mapPositionToScene(localPosition) : localPosition;
    return mapPositionFromScene(scenePositionOther);
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapDirectionToScene(vector3d localDirection)

    Transforms \a localDirection from local space to scene space.
    The return value is not affected by the (inherited) scale or
    position of the node.

    \note the return value will have the same length as \a localDirection
    (i.e. not normalized).

    \sa mapDirectionFromScene, mapDirectionToNode, mapDirectionFromNode
*/
QVector3D QQuick3DNode::mapDirectionToScene(const QVector3D &localDirection) const
{
    QMatrix3x3 theDirMatrix = sceneTransform().normalMatrix();
    return QSSGUtils::mat33::transform(theDirMatrix, localDirection);
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapDirectionFromScene(vector3d sceneDirection)

    Transforms \a sceneDirection from scene space to local space.
    The return value is not affected by the (inherited) scale or
    position of the node.

    \note the return value will have the same length as \a sceneDirection
    (i.e not normalized).

    \sa mapDirectionToScene, mapDirectionToNode, mapDirectionFromNode
*/
QVector3D QQuick3DNode::mapDirectionFromScene(const QVector3D &sceneDirection) const
{
    QMatrix3x3 theDirMatrix = QSSGUtils::mat44::getUpper3x3(sceneTransform());
    theDirMatrix = theDirMatrix.transposed();
    return QSSGUtils::mat33::transform(theDirMatrix, sceneDirection);
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapDirectionToNode(QtQuick3D::Node node, vector3d localDirection)

    Transforms \a localDirection from this nodes local space to the
    local space of \a node.
    The return value is not affected by the (inherited) scale or
    position of the node.

    \note the return value will have the same length as \a localDirection
    (i.e. not normalized).

    \note if \a node is null, then the returned direction will be transformed into scene space coordinates.

    \sa mapDirectionFromNode, mapDirectionFromScene, mapDirectionToScene
*/
QVector3D QQuick3DNode::mapDirectionToNode(const QQuick3DNode *node, const QVector3D &localDirection) const
{
    const auto sceneDirectionSelf = mapDirectionToScene(localDirection);
    return node ? node->mapDirectionFromScene(sceneDirectionSelf) : sceneDirectionSelf;
}

/*!
    \qmlmethod vector3d QtQuick3D::Node::mapDirectionFromNode(QtQuick3D::Node node, vector3d localDirection)

    Transforms \a localDirection from the local space of \a node to the
    local space of this node.
    The return value is not affected by the (inherited) scale or
    position of the node.

    \note the return value will have the same length as \a localDirection
    (i.e. not normalized).

    \note If \a node is null, then \a localDirection is interpreted as it is in scene space coordinates.

    \sa mapDirectionToNode, mapDirectionFromScene, mapDirectionToScene
*/
QVector3D QQuick3DNode::mapDirectionFromNode(const QQuick3DNode *node, const QVector3D &localDirection) const
{
    const auto sceneDirectionOther = node ? node->mapDirectionToScene(localDirection) : localDirection;
    return mapDirectionFromScene(sceneDirectionOther);
}

void QQuick3DNode::markAllDirty()
{
    Q_D(QQuick3DNode);

    d->markSceneTransformDirty();
    QQuick3DObject::markAllDirty();
}

/*!
    \qmlproperty vector3d QtQuick3D::Node::eulerRotation

    This property contains the rotation values for the x, y, and z axis.
    These values are stored as a vector3d.  Rotation order is assumed to
    be ZXY.

    \sa QQuaternion::fromEulerAngles()
*/

QVector3D QQuick3DNode::eulerRotation() const
{
    const Q_D(QQuick3DNode);

    return d->m_rotation;
}

void QQuick3DNode::itemChange(ItemChange change, const ItemChangeData &)
{
    if (change == QQuick3DObject::ItemParentHasChanged)
        QQuick3DNodePrivate::get(this)->markSceneTransformDirty();
}

/*!
    \qmlproperty int QtQuick3D::Node::layers

    This property defines the content layers that this node belongs to.
    The layers can be used to group nodes that belongs together, for example
    to be rendered together or to be filtered out from rendering.
    Setting this property on the active \l Camera will cause the camera to
    only render nodes that belong to the specified layers.
    The value is a bitmask, where each bit represents a layer.

    By default nodes are assigned to the \c main layer, which is \l {ContentLayer.Layer0}{Layer0}.

    \sa ContentLayer
*/

int QQuick3DNode::layers() const
{
    const Q_D(QQuick3DNode);

    return QQuick3DContentLayer::LayerFlags::fromInt(d->m_tag.value());
}

void QQuick3DNode::setLayers(int newLayers)
{
    Q_D(QQuick3DNode);

    if (newLayers < 0) {
        qWarning("QQuick3DNode::setLayers: value must be non-negative");
        return;
    }

    if (d->m_tag == quint32(newLayers))
        return;

    d->m_tag.setValue(newLayers);
    emit layersChanged();
    update();
}

QT_END_NAMESPACE
