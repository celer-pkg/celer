// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qabstractphysicsnode_p.h"
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <foundation/PxTransform.h>

#include "qphysicsworld_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PhysicsNode
    \inherits Node
    \inqmlmodule QtQuick3D.Physics
    \since 6.4
    \brief Base type for all objects in the physics scene.

    PhysicsNode is the base type for all the objects that take part in the physics simulation. These
    objects have a position in three-dimensional space and a geometrical shape.
*/

/*!
    \qmlproperty list<CollisionShape> PhysicsNode::collisionShapes

    This property contains the list of collision shapes. These shapes will be combined and act as a
    single rigid body when interacting with other bodies.

    \sa {Qt Quick 3D Physics Shapes and Bodies}{Shapes and Bodies overview documentation}
*/

/*!
    \qmlproperty bool PhysicsNode::sendContactReports
    This property determines whether this body will send contact reports when colliding with other
    bodies.

    Default value: \c{false}
*/

/*!
    \qmlproperty bool PhysicsNode::receiveContactReports
    This property determines whether this body will receive contact reports when colliding with
    other bodies. If activated, this means that the bodyContact signal will be emitted on a
    collision with a body that has sendContactReports set to true.

    Default value: \c{false}
*/

/*!
    \qmlproperty bool PhysicsNode::sendTriggerReports
    This property determines whether this body will send reports when entering or leaving a trigger
    body.

    Default value: \c{false}
*/

/*!
    \qmlproperty bool PhysicsNode::receiveTriggerReports
    This property determines whether this body will receive reports when entering or leaving a
    trigger body.

    Default value: \c{false}
*/

/*!
    \qmlproperty int PhysicsNode::filterGroup
    This property determines what filter group this body is part of.

    Default value is \c 0.

    Range: \c{[0, 32]}

    \sa PhysicsNode::filterIgnoreGroups
*/

/*!
    \qmlproperty int PhysicsNode::filterIgnoreGroups
    This property determines what groups this body should filter out collisions with.

    \note This number is interpreted as a bitmask, meaning that if bit \c i is set then collisions
    with \l filterGroup number \c i will be filtered. For instance, to filter groups \c{1}, \c{3}
    and \c{4} then set the value to \c{0b11010}.

    \sa PhysicsNode::filterGroup
*/

/*!
    \qmlsignal PhysicsNode::bodyContact(PhysicsNode *body, list<vector3D> positions,
   list<vector3D> impulses, list<vector3D> normals)

    This signal is emitted when there is a collision between a dynamic body and any
    other body. The \l {PhysicsNode::} {receiveContactReports} in this body and \l {PhysicsNode::}
    {sendContactReports} in the colliding body need to be set to true. The parameters \a body, \a
    positions, \a impulses and \a normals contain the other body, position, impulse force and normal
    for each contact point at the same index.

    \sa CharacterController::shapeHit
    \sa PhysicsWorld::reportKinematicKinematicCollisions
    \sa PhysicsWorld::reportStaticKinematicCollisions
*/

/*!
    \qmlsignal PhysicsNode::enteredTriggerBody(TriggerBody *body)

    This signal is emitted when this body enters the specified trigger \a body.

    \note Only emitted when receiveTriggerReports is \c true
    \sa receiveTriggerReports exitedTriggerBody
*/

/*!
    \qmlsignal PhysicsNode::exitedTriggerBody(TriggerBody *body)

    This signal is emitted when this body exits the specified trigger \a body.

    \note Only emitted when receiveTriggerReports is \c true
    \sa receiveTriggerReports enteredTriggerBody
*/

QAbstractPhysicsNode::QAbstractPhysicsNode()
{
    QPhysicsWorld::registerNode(this);
}

QAbstractPhysicsNode::~QAbstractPhysicsNode()
{
    for (auto shape : std::as_const(m_collisionShapes))
        shape->disconnect(this);
    QPhysicsWorld::deregisterNode(this);
}

QQmlListProperty<QAbstractCollisionShape> QAbstractPhysicsNode::collisionShapes()
{
    return QQmlListProperty<QAbstractCollisionShape>(
            this, nullptr, QAbstractPhysicsNode::qmlAppendShape,
            QAbstractPhysicsNode::qmlShapeCount, QAbstractPhysicsNode::qmlShapeAt,
            QAbstractPhysicsNode::qmlClearShapes);
}

const QVector<QAbstractCollisionShape *> &QAbstractPhysicsNode::getCollisionShapesList() const
{
    return m_collisionShapes;
}

void QAbstractPhysicsNode::updateFromPhysicsTransform(const physx::PxTransform &transform)
{
    const auto pos = transform.p;
    const auto rotation = transform.q;
    const auto qtPosition = QVector3D(pos.x, pos.y, pos.z);
    const auto qtRotation = QQuaternion(rotation.w, rotation.x, rotation.y, rotation.z);

    // Get this nodes parent transform
    const QQuick3DNode *parentNode = static_cast<QQuick3DNode *>(parentItem());

    if (!parentNode) {
        // then it is the same space
        setRotation(qtRotation);
        setPosition(qtPosition);
    } else {
        setPosition(parentNode->mapPositionFromScene(qtPosition));
        const auto relativeRotation = parentNode->sceneRotation().inverted() * qtRotation;
        setRotation(relativeRotation);
    }
}

bool QAbstractPhysicsNode::sendContactReports() const
{
    return m_sendContactReports;
}

void QAbstractPhysicsNode::setSendContactReports(bool sendContactReports)
{
    if (m_sendContactReports == sendContactReports)
        return;

    m_sendContactReports = sendContactReports;
    emit sendContactReportsChanged(m_sendContactReports);
}

bool QAbstractPhysicsNode::receiveContactReports() const
{
    return m_receiveContactReports;
}

void QAbstractPhysicsNode::setReceiveContactReports(bool receiveContactReports)
{
    if (m_receiveContactReports == receiveContactReports)
        return;

    m_receiveContactReports = receiveContactReports;
    emit receiveContactReportsChanged(m_receiveContactReports);
}

bool QAbstractPhysicsNode::sendTriggerReports() const
{
    return m_sendTriggerReports;
}

void QAbstractPhysicsNode::setSendTriggerReports(bool sendTriggerReports)
{
    if (m_sendTriggerReports == sendTriggerReports)
        return;

    m_sendTriggerReports = sendTriggerReports;
    emit sendTriggerReportsChanged(m_sendTriggerReports);
}

bool QAbstractPhysicsNode::receiveTriggerReports() const
{
    return m_receiveTriggerReports;
}

void QAbstractPhysicsNode::setReceiveTriggerReports(bool receiveTriggerReports)
{
    if (m_receiveTriggerReports == receiveTriggerReports)
        return;

    m_receiveTriggerReports = receiveTriggerReports;
    emit receiveTriggerReportsChanged(m_receiveTriggerReports);
}

void QAbstractPhysicsNode::registerContact(QAbstractPhysicsNode *body,
                                           const QVector<QVector3D> &positions,
                                           const QVector<QVector3D> &impulses,
                                           const QVector<QVector3D> &normals)
{
    emit bodyContact(body, positions, impulses, normals);
}

void QAbstractPhysicsNode::onShapeDestroyed(QObject *object)
{
    m_collisionShapes.removeAll(static_cast<QAbstractCollisionShape *>(object));
}

void QAbstractPhysicsNode::onShapeNeedsRebuild(QObject * /*object*/)
{
    m_shapesDirty = true;
}

void QAbstractPhysicsNode::qmlAppendShape(QQmlListProperty<QAbstractCollisionShape> *list,
                                          QAbstractCollisionShape *shape)
{
    if (shape == nullptr)
        return;
    QAbstractPhysicsNode *self = static_cast<QAbstractPhysicsNode *>(list->object);
    self->m_collisionShapes.push_back(shape);
    self->m_hasStaticShapes = self->m_hasStaticShapes || shape->isStaticShape();

    if (shape->parentItem() == nullptr) {
        // If the material has no parent, check if it has a hierarchical parent that's a
        // QQuick3DObject and re-parent it to that, e.g., inline materials
        QQuick3DObject *parentItem = qobject_cast<QQuick3DObject *>(shape->parent());
        if (parentItem) {
            shape->setParentItem(parentItem);
        } else { // If no valid parent was found, make sure the material refs our scene manager
            const auto &scenManager = QQuick3DObjectPrivate::get(self)->sceneManager;
            if (scenManager)
                QQuick3DObjectPrivate::refSceneManager(shape, *scenManager);
            // else: If there's no scene manager, defer until one is set, see itemChange()
        }
    }

    // Make sure materials are removed when destroyed
    connect(shape, &QAbstractCollisionShape::destroyed, self,
            &QAbstractPhysicsNode::onShapeDestroyed);

    // Connect to rebuild signal
    connect(shape, &QAbstractCollisionShape::needsRebuild, self,
            &QAbstractPhysicsNode::onShapeNeedsRebuild);
}

QAbstractCollisionShape *
QAbstractPhysicsNode::qmlShapeAt(QQmlListProperty<QAbstractCollisionShape> *list, qsizetype index)
{
    QAbstractPhysicsNode *self = static_cast<QAbstractPhysicsNode *>(list->object);
    return self->m_collisionShapes.at(index);
}

qsizetype QAbstractPhysicsNode::qmlShapeCount(QQmlListProperty<QAbstractCollisionShape> *list)
{
    QAbstractPhysicsNode *self = static_cast<QAbstractPhysicsNode *>(list->object);
    return self->m_collisionShapes.count();
}

void QAbstractPhysicsNode::qmlClearShapes(QQmlListProperty<QAbstractCollisionShape> *list)
{
    QAbstractPhysicsNode *self = static_cast<QAbstractPhysicsNode *>(list->object);
    for (const auto &shape : std::as_const(self->m_collisionShapes)) {
        if (shape->parentItem() == nullptr)
            QQuick3DObjectPrivate::get(shape)->derefSceneManager();
    }
    self->m_hasStaticShapes = false;
    for (auto shape : std::as_const(self->m_collisionShapes))
        shape->disconnect(self);
    self->m_collisionShapes.clear();
}

int QAbstractPhysicsNode::filterGroup() const
{
    return m_filterGroup;
}

void QAbstractPhysicsNode::setfilterGroup(int newfilterGroup)
{
    if (m_filterGroup == newfilterGroup)
        return;
    m_filterGroup = newfilterGroup;
    m_filtersDirty = true;
    emit filterGroupChanged();
}

int QAbstractPhysicsNode::filterIgnoreGroups() const
{
    return m_filterIgnoreGroups;
}

void QAbstractPhysicsNode::setFilterIgnoreGroups(int newFilterIgnoreGroups)
{
    if (m_filterIgnoreGroups == newFilterIgnoreGroups)
        return;
    m_filterIgnoreGroups = newFilterIgnoreGroups;
    m_filtersDirty = true;
    emit filterIgnoreGroupsChanged();
}

QT_END_NAMESPACE
