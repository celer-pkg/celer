// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qtriggerbody_p.h"
#include "physxnode/qphysxtriggerbody_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype TriggerBody
    \inherits PhysicsNode
    \inqmlmodule QtQuick3D.Physics
    \since 6.4
    \brief Reports when objects enter a given volume.

    This type defines a trigger body. A trigger body is a body that does not interact
    physically but is used to detect when objects intersect with its volume.
*/

/*!
    \qmlproperty int TriggerBody::collisionCount
    This property returns the number of bodies currently colliding with the trigger body.
*/

/*!
    \qmlsignal TriggerBody::bodyEntered(PhysicsNode *body)
    This signal is emitted when the trigger body is penetrated by the specified \a body.
*/

/*!
    \qmlsignal TriggerBody::bodyExited(PhysicsNode *body)
    This signal is emitted when the trigger body is no longer penetrated by the specified \a body.
*/

QTriggerBody::QTriggerBody() = default;

void QTriggerBody::registerCollision(QAbstractPhysicsNode *collision)
{
    int size = m_collisions.size();
    m_collisions.insert(collision);

    if (size != m_collisions.size()) {
        emit bodyEntered(collision);
        emit collisionCountChanged();
    }
}

void QTriggerBody::deregisterCollision(QAbstractPhysicsNode *collision)
{
    int size = m_collisions.size();
    m_collisions.remove(collision);

    if (size != m_collisions.size()) {
        emit bodyExited(collision);
        emit collisionCountChanged();
    }
}

int QTriggerBody::collisionCount() const
{
    return m_collisions.count();
}

QAbstractPhysXNode *QTriggerBody::createPhysXBackend()
{
    return new QPhysXTriggerBody(this);
}

QT_END_NAMESPACE
