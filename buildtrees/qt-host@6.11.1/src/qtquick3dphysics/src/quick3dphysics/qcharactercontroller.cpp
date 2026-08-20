// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qcharactercontroller_p.h"

#include "physxnode/qphysxcharactercontroller_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype CharacterController
    \inqmlmodule QtQuick3D.Physics
    \inherits PhysicsBody
    \since 6.4
    \brief Controls the motion of a character.

    The CharacterController type controls the motion of a character.

    A character is an entity that moves under external control, but is still constrained
    by physical barriers and (optionally) subject to gravity.  This is in contrast to
    \l{DynamicRigidBody}{dynamic rigid bodies} which are either completely controlled by
    the physics simulation (for non-kinematic bodies); or move exactly where placed,
    regardless of barriers (for kinematic objects).

    To control the motion of a character controller, set \l movement to the desired velocity.

    For a first-person view, the camera is typically placed inside a character controller.

    \note \l {PhysicsNode::collisionShapes}{collisionShapes} must be set to
    a single \l {CapsuleShape}. No other shapes are supported.

    \note The character controller is able to scale obstacles that are lower than one fourth of
    the capsule shape's height.

    \sa {Qt Quick 3D Physics Shapes and Bodies}{Shapes and Bodies overview documentation}
*/

/*!
    \qmlproperty vector3d CharacterController::movement

    This property defines the controlled motion of the character. This is the velocity the character
    would move in the absence of gravity and without interacting with other physics objects.

    This property does not reflect the actual velocity of the character. If the character is stuck
    against terrain, the character can move slower than the speed defined by \c movement. Conversely, if the
    character is in free fall, it may move much faster.

    Default value: \c{(0, 0, 0)}
*/

/*!
    \qmlproperty vector3d CharacterController::gravity

    This property defines the gravitational acceleration that applies to the character.
    For a character that walks on the ground, it should typically be set to
    \l{PhysicsWorld::gravity}{PhysicsWorld.gravity}. A floating character that has movement
    controls in three dimensions will normally have gravity \c{(0, 0, 0)}.

    Default value: \c{(0, 0, 0)}.
*/

/*!
    \qmlproperty bool CharacterController::midAirControl

    This property defines whether the \l movement property has effect when the character is in free
    fall. This is only relevant if \l gravity in not null. A value of \c true means that the
    character will change direction in mid-air when \c movement changes. A value of \c false means that
    the character will continue on its current trajectory until it hits another object.

    Default value: \c true
*/

/*!
    \qmlproperty Collisions CharacterController::collisions
    \readonly

    This property holds the current collision state of the character. It is either \c None for no
    collision, or an OR combination of \c Side, \c Up, and \c Down:

     \value   CharacterController.None
         The character is not touching anything. If gravity is non-null, this means that the
         character is in free fall.
     \value   CharacterController.Side
         The character is touching something on its side.
     \value   CharacterController.Up
         The character is touching something above it.
     \value   CharacterController.Down
         The character is touching something below it. In standard gravity, this means
         that the character is on the ground.

     \note The directions are defined relative to standard gravity: \c Up is always along the
           positive y-axis, regardless of the value of \l {gravity}{CharacterController.gravity}
           or \l{PhysicsWorld::gravity}{PhysicsWorld.gravity}
*/

/*!
    \qmlproperty bool CharacterController::enableShapeHitCallback
    \since 6.6

    This property enables/disables the \l {CharacterController::shapeHit} callback for this
    character controller.

    Default value: \c{false}
*/

/*!
    \qmlmethod void CharacterController::teleport(vector3d position)
    Immediately move the character to \a position without checking for collisions.
    The caller is responsible for avoiding overlap with static objects.
*/

/*!
    \qmlsignal CharacterController::shapeHit(PhysicsNode *body, vector3D position, vector3D impulse,
    vector3D normal)
    \since 6.6

    This signal is emitted when \l {CharacterController::}{movement} has been
    called and it would result
    in a collision with a \l {DynamicRigidBody} or a \l {StaticRigidBody} and
    \l {CharacterController::} {enableShapeHitCallback} is set to \c true.
    The parameters \a body, \a position, \a impulse and \a normal contain the body, position,
    impulse force and normal for the contact point.
*/

QCharacterController::QCharacterController() = default;

const QVector3D &QCharacterController::movement() const
{
    return m_movement;
}

void QCharacterController::setMovement(const QVector3D &newMovement)
{
    if (m_movement == newMovement)
        return;
    m_movement = newMovement;
    emit movementChanged();
}

const QVector3D &QCharacterController::gravity() const
{
    return m_gravity;
}

void QCharacterController::setGravity(const QVector3D &newGravity)
{
    if (m_gravity == newGravity)
        return;
    m_gravity = newGravity;
    emit gravityChanged();
}

// Calculate move based on movement/gravity

QVector3D QCharacterController::getDisplacement(float deltaTime)
{
    // Start with basic movement, assuming no other factors
    QVector3D displacement = sceneRotation() * m_movement * deltaTime;

    // modified based on gravity
    const auto g = m_gravity;
    if (!g.isNull()) {

        // Avoid "spider mode": we are also supposed to be in free fall if gravity
        // is pointing away from a surface we are touching. I.e. we are NOT in free
        // fall only if gravity has a component in the direction of one of the collisions.
        // Also: if we have "upwards" free fall velocity, that motion needs to stop
        // when we hit the "ceiling"; i.e we are not in free fall at the moment of impact.
        auto isGrounded = [this](){
            if (m_collisions == Collision::None)
                return false;

            // Standard gravity case first
            if (m_gravity.y() < 0) {
                if (m_collisions & Collision::Down)
                     return true; // We land on the ground
                if ((m_collisions & Collision::Up) && m_freeFallVelocity.y() > 0)
                    return true; // We bump our head on the way up
            }

            // Inverse gravity next: exactly the opposite
            if (m_gravity.y() > 0) {
                if (m_collisions & Collision::Up)
                     return true;
                if ((m_collisions & Collision::Down) && m_freeFallVelocity.y() < 0)
                    return true;
            }

            // The sideways gravity case can't be perfectly handled since we don't
            // know the direction of sideway contacts. We could in theory inspect
            // the mesh, but that is far too complex for an extremely marginal use case.

            if ((m_gravity.x() != 0 || m_gravity.z() != 0) && m_collisions & Collision::Side)
                return true;

            return false;
        };

        bool freeFalling = !isGrounded();
        if (freeFalling) {
            if (!m_midAirControl)
                displacement = {}; // Ignore the movement() controls in true free fall

            displacement += m_freeFallVelocity * deltaTime;
            m_freeFallVelocity += g * deltaTime;
        } else {
            m_freeFallVelocity = displacement / deltaTime + g * deltaTime;
            if (m_midAirControl) // free fall only straight down
                m_freeFallVelocity =
                        QVector3D::dotProduct(m_freeFallVelocity, g.normalized()) * g.normalized();
        }
        const QVector3D gravityAcceleration = 0.5 * deltaTime * deltaTime * g;
        displacement += gravityAcceleration; // always add gravitational acceleration, in case we start
                                             // to fall. If we don't, PhysX will move us back to the ground.
    }

    return displacement;
}

bool QCharacterController::midAirControl() const
{
    return m_midAirControl;
}

void QCharacterController::setMidAirControl(bool newMidAirControl)
{
    if (m_midAirControl == newMidAirControl)
        return;
    m_midAirControl = newMidAirControl;
    emit midAirControlChanged();
}

void QCharacterController::teleport(const QVector3D &position)
{
    m_teleport = true;
    m_teleportPosition = position;
    m_freeFallVelocity = {};
}

bool QCharacterController::getTeleport(QVector3D &position)
{
    if (m_teleport) {
        position = m_teleportPosition;
        m_teleport = false;
        return true;
    }
    return false;
}

const QCharacterController::Collisions &QCharacterController::collisions() const
{
    return m_collisions;
}

void QCharacterController::setCollisions(const Collisions &newCollisions)
{
    if (m_collisions == newCollisions)
        return;
    m_collisions = newCollisions;
    emit collisionsChanged();
}

bool QCharacterController::enableShapeHitCallback() const
{
    return m_enableShapeHitCallback;
}

QAbstractPhysXNode *QCharacterController::createPhysXBackend()
{
    return new QPhysXCharacterController(this);
}

void QCharacterController::setEnableShapeHitCallback(bool newEnableShapeHitCallback)
{
    if (m_enableShapeHitCallback == newEnableShapeHitCallback)
        return;
    m_enableShapeHitCallback = newEnableShapeHitCallback;
    emit enableShapeHitCallbackChanged();
}

QT_END_NAMESPACE
