// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CHARACTERCONTROLLER_H
#define CHARACTERCONTROLLER_H

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

#include <QtQuick3DPhysics/qtquick3dphysicsglobal.h>
#include <QtQuick3DPhysics/private/qabstractphysicsbody_p.h>
#include <QtQml/QQmlEngine>
#include <QVector3D>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QCharacterController : public QAbstractPhysicsBody
{
    Q_OBJECT
    Q_PROPERTY(QVector3D movement READ movement WRITE setMovement NOTIFY movementChanged)
    Q_PROPERTY(QVector3D gravity READ gravity WRITE setGravity NOTIFY gravityChanged)
    Q_PROPERTY(bool midAirControl READ midAirControl WRITE setMidAirControl NOTIFY
                       midAirControlChanged)
    Q_PROPERTY(Collisions collisions READ collisions NOTIFY collisionsChanged)
    Q_PROPERTY(bool enableShapeHitCallback READ enableShapeHitCallback WRITE
                       setEnableShapeHitCallback NOTIFY enableShapeHitCallbackChanged)
    QML_NAMED_ELEMENT(CharacterController)
public:
    QCharacterController();

    enum class Collision {
        None = 0,
        Side = 1 << 0,
        Up = 1 << 1,
        Down = 1 << 2,
    };
    Q_DECLARE_FLAGS(Collisions, Collision)
    Q_FLAG(Collisions)

    const QVector3D &movement() const;
    void setMovement(const QVector3D &newMovement);
    const QVector3D &gravity() const;
    void setGravity(const QVector3D &newGravity);
    QVector3D getDisplacement(float deltaTime);
    bool getTeleport(QVector3D &position);

    bool midAirControl() const;
    void setMidAirControl(bool newMidAirControl);

    Q_INVOKABLE void teleport(const QVector3D &position);

    const Collisions &collisions() const;
    void setCollisions(const Collisions &newCollisions);
    QAbstractPhysXNode *createPhysXBackend() final;

    Q_REVISION(6, 6) bool enableShapeHitCallback() const;
    Q_REVISION(6, 6) void setEnableShapeHitCallback(bool newEnableShapeHitCallback);

signals:
    void movementChanged();
    void gravityChanged();

    void midAirControlChanged();

    void collisionsChanged();
    void enableShapeHitCallbackChanged();
    void shapeHit(QAbstractPhysicsNode *body, const QVector3D &position, const QVector3D &impulse,
                  const QVector3D &normal);

private:
    QVector3D m_movement;
    QVector3D m_gravity;
    bool m_midAirControl = true;

    QVector3D m_freeFallVelocity; // actual speed at start of next tick, if free fall

    QVector3D m_teleportPosition;
    bool m_teleport = false;
    Collisions m_collisions;
    bool m_enableShapeHitCallback = false;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCharacterController::Collisions)

QT_END_NAMESPACE

#endif // CHARACTERCONTROLLER_H
