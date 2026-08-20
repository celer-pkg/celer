// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef DYNAMICRIGIDBODY_H
#define DYNAMICRIGIDBODY_H

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

#include <QtQuick3DPhysics/private/qabstractphysicsbody_p.h>
#include <QtQml/QQmlEngine>

#include <QtCore/QQueue>
#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

class QPhysicsCommand;

class Q_QUICK3DPHYSICS_EXPORT QDynamicRigidBody : public QAbstractPhysicsBody
{
public:
    enum class MassMode {
        DefaultDensity,
        CustomDensity,
        Mass,
        MassAndInertiaTensor,
        MassAndInertiaMatrix,
    };
    Q_ENUM(MassMode)

    enum AxisLock {
        LockNone = 0,
        LockX = 1,
        LockY = 2,
        LockZ = 4,
    };
    Q_ENUM(AxisLock)

    Q_OBJECT
    Q_PROPERTY(float mass READ mass WRITE setMass NOTIFY massChanged)
    Q_PROPERTY(float density READ density WRITE setDensity NOTIFY densityChanged)

    Q_PROPERTY(AxisLock linearAxisLock READ linearAxisLock WRITE setLinearAxisLock NOTIFY
                       linearAxisLockChanged REVISION(6, 5))
    Q_PROPERTY(AxisLock angularAxisLock READ angularAxisLock WRITE setAngularAxisLock NOTIFY
                       angularAxisLockChanged REVISION(6, 5))

    Q_PROPERTY(bool isKinematic READ isKinematic WRITE setIsKinematic NOTIFY isKinematicChanged)
    Q_PROPERTY(bool gravityEnabled READ gravityEnabled WRITE setGravityEnabled NOTIFY
                       gravityEnabledChanged)

    Q_PROPERTY(MassMode massMode READ massMode WRITE setMassMode NOTIFY massModeChanged)
    Q_PROPERTY(QVector3D inertiaTensor READ inertiaTensor WRITE setInertiaTensor NOTIFY
                       inertiaTensorChanged)
    Q_PROPERTY(QVector3D centerOfMassPosition READ centerOfMassPosition WRITE
                       setCenterOfMassPosition NOTIFY centerOfMassPositionChanged)
    Q_PROPERTY(QQuaternion centerOfMassRotation READ centerOfMassRotation WRITE
                       setCenterOfMassRotation NOTIFY centerOfMassRotationChanged)
    Q_PROPERTY(QList<float> inertiaMatrix READ readInertiaMatrix WRITE setInertiaMatrix NOTIFY
                       inertiaMatrixChanged);

    Q_PROPERTY(QVector3D kinematicPosition READ kinematicPosition WRITE setKinematicPosition NOTIFY
                       kinematicPositionChanged REVISION(6, 5));
    Q_PROPERTY(QVector3D kinematicEulerRotation READ kinematicEulerRotation WRITE
                       setKinematicEulerRotation NOTIFY kinematicEulerRotationChanged REVISION(6,
                                                                                               5));
    Q_PROPERTY(QQuaternion kinematicRotation READ kinematicRotation WRITE setKinematicRotation
                       NOTIFY kinematicRotationChanged REVISION(6, 5));
    Q_PROPERTY(QVector3D kinematicPivot READ kinematicPivot WRITE setKinematicPivot NOTIFY
                       kinematicPivotChanged REVISION(6, 5));

    Q_PROPERTY(bool isSleeping READ isSleeping WRITE setIsSleeping NOTIFY isSleepingChanged
                       REVISION(6, 9));

    // clang-format off
//    // ??? separate simulation control object? --- some of these have default values in the engine, so we need tristate
//    Q_PROPERTY(float sleepThreshold READ sleepThreshold WRITE setSleepThreshold NOTIFY sleepThresholdChanged)
//    Q_PROPERTY(float stabilizationThreshold READ stabilizationThreshold WRITE setStabilizationThreshold NOTIFY stabilizationThresholdChanged)
//    Q_PROPERTY(float contactReportThreshold READ contactReportThreshold WRITE setContactReportThreshold NOTIFY contactReportThresholdChanged)
//    Q_PROPERTY(float maxContactImpulse READ maxContactImpulse WRITE setMaxContactImpulse NOTIFY maxContactImpulseChanged)
//    Q_PROPERTY(float maxDepenetrationVelocity READ maxDepenetrationVelocity WRITE setMaxDepenetrationVelocity NOTIFY maxDepenetrationVelocityChanged)
//    Q_PROPERTY(float maxAngularVelocity READ maxAngularVelocity WRITE setMaxAngularVelocity NOTIFY maxAngularVelocityChanged)
//    Q_PROPERTY(int minPositionIterationCount READ minPositionIterationCount WRITE setMinPositionIterationCount NOTIFY minPositionIterationCountChanged)
//    Q_PROPERTY(int minVelocityIterationCount READ minVelocityIterationCount WRITE setMinVelocityIterationCount NOTIFY minVelocityIterationCountChanged)
    // clang-format on
    QML_NAMED_ELEMENT(DynamicRigidBody)

public:
    QDynamicRigidBody();
    ~QDynamicRigidBody();

    float mass() const;
    void setMass(float mass);

    float density() const;
    void setDensity(float density);

    bool isKinematic() const;
    void setIsKinematic(bool isKinematic);

    Q_REVISION(6, 5) AxisLock linearAxisLock() const;
    Q_REVISION(6, 5) void setLinearAxisLock(AxisLock newAxisLockLinear);

    Q_REVISION(6, 5) AxisLock angularAxisLock() const;
    Q_REVISION(6, 5) void setAngularAxisLock(AxisLock newAxisLockAngular);

    bool gravityEnabled() const;
    void setGravityEnabled(bool gravityEnabled);

    Q_INVOKABLE void applyCentralForce(const QVector3D &force);
    Q_INVOKABLE void applyForce(const QVector3D &force, const QVector3D &position);
    Q_INVOKABLE void applyTorque(const QVector3D &torque);
    Q_INVOKABLE void applyCentralImpulse(const QVector3D &impulse);
    Q_INVOKABLE void applyImpulse(const QVector3D &impulse, const QVector3D &position);
    Q_INVOKABLE void applyTorqueImpulse(const QVector3D &impulse);
    Q_INVOKABLE void setAngularVelocity(const QVector3D &angularVelocity);
    Q_INVOKABLE void setLinearVelocity(const QVector3D &linearVelocity);
    Q_INVOKABLE void reset(const QVector3D &position, const QVector3D &eulerRotation);

    // Internal
    QQueue<QPhysicsCommand *> &commandQueue();

    void updateDefaultDensity(float defaultDensity);

    MassMode massMode() const;
    void setMassMode(const MassMode newMassMode);

    const QVector3D &inertiaTensor() const;
    void setInertiaTensor(const QVector3D &newInertiaTensor);

    const QVector3D &centerOfMassPosition() const;
    void setCenterOfMassPosition(const QVector3D &newCenterOfMassPosition);

    const QQuaternion &centerOfMassRotation() const;
    void setCenterOfMassRotation(const QQuaternion &newCenterOfMassRotation);

    const QList<float> &readInertiaMatrix() const;
    void setInertiaMatrix(const QList<float> &newInertiaMatrix);
    const QMatrix3x3 &inertiaMatrix() const;

    Q_REVISION(6, 5) void setKinematicPosition(const QVector3D &position);
    Q_REVISION(6, 5) QVector3D kinematicPosition() const;

    Q_REVISION(6, 5) void setKinematicRotation(const QQuaternion &rotation);
    Q_REVISION(6, 5) QQuaternion kinematicRotation() const;

    Q_REVISION(6, 5) void setKinematicEulerRotation(const QVector3D &rotation);
    Q_REVISION(6, 5) QVector3D kinematicEulerRotation() const;

    Q_REVISION(6, 5) void setKinematicPivot(const QVector3D &pivot);
    Q_REVISION(6, 5) QVector3D kinematicPivot() const;

    Q_REVISION(6, 9) void setIsSleeping(bool newIsSleeping);
    Q_REVISION(6, 9) bool isSleeping() const;

    QAbstractPhysXNode *createPhysXBackend() final;

Q_SIGNALS:
    void massChanged(float mass);
    void densityChanged(float density);
    void isKinematicChanged(bool isKinematic);
    Q_REVISION(6, 5) void linearAxisLockChanged();
    Q_REVISION(6, 5) void angularAxisLockChanged();
    void gravityEnabledChanged();
    void massModeChanged();
    void inertiaTensorChanged();
    void centerOfMassPositionChanged();
    void centerOfMassRotationChanged();
    void inertiaMatrixChanged();
    Q_REVISION(6, 5) void kinematicPositionChanged(const QVector3D &kinematicPosition);
    Q_REVISION(6, 5) void kinematicRotationChanged(const QQuaternion &kinematicRotation);
    Q_REVISION(6, 5) void kinematicEulerRotationChanged(const QVector3D &kinematicEulerRotation);
    Q_REVISION(6, 5) void kinematicPivotChanged(const QVector3D &kinematicPivot);
    Q_REVISION(6, 9) void isSleepingChanged(bool isSleeping);

private:
    float m_mass = 1.f;
    float m_density = 0.001f;
    QVector3D m_centerOfMassPosition;
    QQuaternion m_centerOfMassRotation;
    QList<float> m_inertiaMatrixList;
    QMatrix3x3 m_inertiaMatrix;
    QVector3D m_inertiaTensor;

    bool m_isKinematic = false;
    AxisLock m_linearAxisLock = AxisLock::LockNone;
    AxisLock m_angularAxisLock = AxisLock::LockNone;
    QQueue<QPhysicsCommand *> m_commandQueue;
    bool m_gravityEnabled = true;
    MassMode m_massMode = MassMode::DefaultDensity;

    QVector3D m_kinematicPosition;
    RotationData m_kinematicRotation;
    QVector3D m_kinematicPivot;
    bool m_isSleeping = false;
};

QT_END_NAMESPACE

#endif // DYNAMICRIGIDBODY_H
