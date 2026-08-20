// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QPHYSICSCOMMANDS_H
#define QPHYSICSCOMMANDS_H

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

#include <QtCore/QList>
#include <QtGui/QVector3D>
#include <QtGui/qgenericmatrix.h>

namespace physx {
class PxRigidBody;
}

QT_BEGIN_NAMESPACE

class QDynamicRigidBody;

class QPhysicsCommand
{
    Q_DISABLE_COPY_MOVE(QPhysicsCommand)
public:
    QPhysicsCommand() = default;
    virtual ~QPhysicsCommand();
    virtual void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) = 0;
};

class QPhysicsCommandApplyCentralForce : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandApplyCentralForce(const QVector3D &inForce);
    ~QPhysicsCommandApplyCentralForce() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D force;
};

class QPhysicsCommandApplyForce : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandApplyForce(const QVector3D &inForce, const QVector3D &inPosition);
    ~QPhysicsCommandApplyForce() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D force;
    QVector3D position;
};

class QPhysicsCommandApplyTorque : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandApplyTorque(const QVector3D &inTorque);
    ~QPhysicsCommandApplyTorque() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D torque;
};

class QPhysicsCommandApplyCentralImpulse : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandApplyCentralImpulse(const QVector3D &inImpulse);
    ~QPhysicsCommandApplyCentralImpulse() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D impulse;
};

class QPhysicsCommandApplyImpulse : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandApplyImpulse(const QVector3D &inImpulse, const QVector3D &inPosition);
    ~QPhysicsCommandApplyImpulse() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D impulse;
    QVector3D position;
};

class QPhysicsCommandApplyTorqueImpulse : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandApplyTorqueImpulse(const QVector3D &inImpulse);
    ~QPhysicsCommandApplyTorqueImpulse() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D impulse;
};

class QPhysicsCommandSetAngularVelocity : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetAngularVelocity(const QVector3D &inAngularVelocity);
    ~QPhysicsCommandSetAngularVelocity() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D angularVelocity;
};

class QPhysicsCommandSetLinearVelocity : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetLinearVelocity(const QVector3D &inLinearVelocity);
    ~QPhysicsCommandSetLinearVelocity() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D linearVelocity;
};

class QPhysicsCommandSetMass : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetMass(float inMass);
    ~QPhysicsCommandSetMass() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    float mass;
};

class QPhysicsCommandSetMassAndInertiaTensor : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetMassAndInertiaTensor(float inMass, const QVector3D &inInertia);
    ~QPhysicsCommandSetMassAndInertiaTensor() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    float mass;
    QVector3D inertia;
};

class QPhysicsCommandSetMassAndInertiaMatrix : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetMassAndInertiaMatrix(float inMass, const QMatrix3x3 &inInertia);
    ~QPhysicsCommandSetMassAndInertiaMatrix() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    float mass;
    QMatrix3x3 inertia;
};

class QPhysicsCommandSetDensity : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetDensity(float inDensity);
    ~QPhysicsCommandSetDensity() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    float density;
};

class QPhysicsCommandSetIsKinematic : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetIsKinematic(bool inIsKinematic);
    ~QPhysicsCommandSetIsKinematic() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    bool isKinematic;
};

class QPhysicsCommandSetGravityEnabled : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandSetGravityEnabled(bool inGravityEnabled);
    ~QPhysicsCommandSetGravityEnabled() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    bool gravityEnabled;
};

class QPhysicsCommandReset : public QPhysicsCommand
{
public:
    explicit QPhysicsCommandReset(QVector3D inPosition, QVector3D inEulerRotation);
    ~QPhysicsCommandReset() override;

    void execute(const QDynamicRigidBody &rigidBody, physx::PxRigidBody &body) override;

private:
    QVector3D position;
    QVector3D eulerRotation;
};

QT_END_NAMESPACE

#endif // QPHYSICSCOMMANDS_H
