// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qdynamicrigidbody_p.h"
#include "qphysicscommands_p.h"
#include "qphysicsworld_p.h"
#include "physxnode/qphysxdynamicbody_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype DynamicRigidBody
    \inqmlmodule QtQuick3D.Physics
    \inherits PhysicsBody
    \since 6.4
    \brief A physical body that can move or be moved.

    This type defines a dynamic rigid body: an object that is part of the physics
    scene and behaves like a physical object with mass and velocity.

    \note \l{TriangleMeshShape}{triangle mesh}, \l{HeightFieldShape}{height field} and
    \l{PlaneShape}{plane} geometry shapes are not allowed as collision shapes when
    \l isKinematic is \c false.
*/

/*!
    \qmlproperty real DynamicRigidBody::mass

    This property defines the mass of the body. Note that this is only used when massMode is not
    \c {DynamicRigidBody.CustomDensity} or \c {DynamicRigidBody.DefaultDensity}. Also note that
    a value of 0 is interpreted as infinite mass and that negative numbers are not allowed.

    Default value: \c 1

    Range: \c{[0, inf]}

    \sa massMode
*/

/*!
    \qmlproperty real DynamicRigidBody::density

    This property defines the density of the body. This is only used when massMode is set to \c
    {DynamicRigidBody.CustomDensity}.

    Default value: \c{0.001}

    Range: \c{(0, inf]}
    \sa massMode
*/

/*!
    \qmlproperty AxisLock DynamicRigidBody::linearAxisLock

    This property locks the linear velocity of the body along the axes defined by the
    DynamicRigidBody.AxisLock enum. To lock several axes just bitwise-or their enum values.

    Available options:

    \value  DynamicRigidBody.None
            No axis lock.

    \value  DynamicRigidBody.LockX
            Lock X axis.

    \value  DynamicRigidBody.LockY
            Lock Y axis.

    \value  DynamicRigidBody.LockZ
            Lock Z axis.

    Default value: \c{DynamicRigidBody.None}
*/

/*!
    \qmlproperty AxisLock DynamicRigidBody::angularAxisLock

    This property locks the angular velocity of the body along the axes defined by the
    DynamicRigidBody.AxisLock enum. To lock several axes just bitwise-or their enum values.

    Available options:

    \value  DynamicRigidBody.None
            No axis lock.

    \value  DynamicRigidBody.LockX
            Lock X axis.

    \value  DynamicRigidBody.LockY
            Lock Y axis.

    \value  DynamicRigidBody.LockZ
            Lock Z axis.

    Default value: \c{DynamicRigidBody.None}
*/

/*!
    \qmlproperty bool DynamicRigidBody::isKinematic
    This property defines whether the object is kinematic or not. A kinematic object does not get
    influenced by external forces and can be seen as an object of infinite mass. If this property is
    set then in every simulation frame the physical object will be moved to its target position
    regardless of external forces. Note that to move and rotate the kinematic object you need to use
    the kinematicPosition, kinematicRotation, kinematicEulerRotation and kinematicPivot properties.

    Default value: \c{false}

    \sa kinematicPosition, kinematicRotation, kinematicEulerRotation, kinematicPivot
*/

/*!
    \qmlproperty bool DynamicRigidBody::gravityEnabled
    This property defines whether the object is going to be affected by gravity or not.

    Default value: \c{true}
*/

/*!
    \qmlproperty MassMode DynamicRigidBody::massMode

    This property holds the enum which describes how mass and inertia are calculated for this body.

    Available options:

    \value  DynamicRigidBody.DefaultDensity
            Use the density specified in the \l {PhysicsWorld::}{defaultDensity} property in
            PhysicsWorld to calculate mass and inertia assuming a uniform density.

    \value  DynamicRigidBody.CustomDensity
            Use specified density in the specified in the \l {DynamicRigidBody::}{density} to
            calculate mass and inertia assuming a uniform density.

    \value  DynamicRigidBody.Mass
            Use the specified mass to calculate inertia assuming a uniform density.

    \value  DynamicRigidBody.MassAndInertiaTensor
            Use the specified mass value and inertia tensor.

    \value  DynamicRigidBody.MassAndInertiaMatrix
            Use the specified mass value and calculate inertia from the specified inertia
            matrix.

    Default value: \c{DynamicRigidBody.DefaultDensity}
*/

/*!
    \qmlproperty vector3d DynamicRigidBody::inertiaTensor

    Defines the inertia tensor vector, using a parameter specified in mass space coordinates.

    This is the diagonal vector of a 3x3 diagonal matrix, if you have a non diagonal world/actor
    space inertia tensor then you should use \l{DynamicRigidBody::inertiaMatrix}{inertiaMatrix}
    instead.

    The inertia tensor components must be positive and a value of 0 in any component is
    interpreted as infinite inertia along that axis. Note that this is only used when
    massMode is set to \c DynamicRigidBody.MassAndInertiaTensor.

    Default value: \c{(1, 1, 1)}

    \sa massMode, inertiaMatrix
*/

/*!
    \qmlproperty vector3d DynamicRigidBody::centerOfMassPosition

    Defines the position of the center of mass relative to the body. Note that this is only used
    when massMode is set to \c DynamicRigidBody.MassAndInertiaTensor.

    Default value: \c{(0, 0, 0)}

    \sa massMode, inertiaTensor
*/

/*!
    \qmlproperty quaternion DynamicRigidBody::centerOfMassRotation

    Defines the rotation of the center of mass pose, i.e. it specifies the orientation of the body's
    principal inertia axes relative to the body. Note that this is only used when massMode is set to
    \c DynamicRigidBody.MassAndInertiaTensor.

    Default value: \c{(1, 0, 0, 0)}

    \sa massMode, inertiaTensor
*/

/*!
    \qmlproperty list<real> DynamicRigidBody::inertiaMatrix

    Defines the inertia tensor matrix. This is a 3x3 matrix in column-major order. Note that this
    matrix is expected to be diagonalizable. Note that this is only used when massMode is set to
    \c DynamicRigidBody.MassAndInertiaMatrix.

    Default value: A 3x3 identity matrix

    \sa massMode, inertiaTensor
*/

/*!
    \qmlproperty vector3d DynamicRigidBody::kinematicPosition
    \since 6.5

    Defines the position of the object when it is kinematic, i.e. when \l isKinematic is set to \c
    true. On each iteration of the simulation the physical object will be updated according to this
    value.

    Default value: \c{(0, 0, 0)}

    \sa isKinematic, kinematicRotation, kinematicEulerRotation, kinematicPivot
*/

/*!
    \qmlproperty quaternion DynamicRigidBody::kinematicRotation
    \since 6.5

    Defines the rotation of the object when it is kinematic, i.e. when \l isKinematic is set to \c
    true. On each iteration of the simulation the physical object will be updated according to this
    value.

    Default value: \c{(1, 0, 0, 0)}

    \sa isKinematic, kinematicPosition, kinematicEulerRotation, kinematicPivot
*/

/*!
    \qmlproperty vector3d DynamicRigidBody::kinematicEulerRotation
    \since 6.5

    Defines the euler rotation of the object when it is kinematic, i.e. when \l isKinematic is set to \c
    true. On each iteration of the simulation the physical object will be updated according to this
    value.

    Default value: \c{(0, 0, 0)}

    \sa isKinematic, kinematicPosition, kinematicEulerRotation, kinematicPivot
*/

/*!
    \qmlproperty vector3d DynamicRigidBody::kinematicPivot
    \since 6.5

    Defines the pivot of the object when it is kinematic, i.e. when \l isKinematic is set to \c
    true. On each iteration of the simulation the physical object will be updated according to this
    value.

    Default value: \c{(0, 0, 0)}

    \sa isKinematic, kinematicPosition, kinematicEulerRotation, kinematicRotation
*/

/*!
    \qmlproperty bool DynamicRigidBody::isSleeping
    \since 6.9

    Is set to \c{true} if the body is sleeping. While it is technically possible to set this property
    it should be seen as a read-only property that is set on every frame the physics simulation is
    running.
*/

/*!
    \qmlmethod void DynamicRigidBody::applyCentralForce(vector3d force)

    Applies  a \a force on the center of the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::applyForce(vector3d force, vector3d position)

    Applies a \a force at a \a position on the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::applyTorque(vector3d torque)

    Applies a \a torque on the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::applyCentralImpulse(vector3d impulse)

    Applies an \a impulse on the center of the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::applyImpulse(vector3d impulse, vector3d position)

    Applies an \a impulse at a \a position on the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::applyTorqueImpulse(vector3d impulse)

    Applies a torque \a impulse on the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::setAngularVelocity(vector3d angularVelocity)

    Sets the \a angularVelocity of the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::setLinearVelocity(vector3d linearVelocity)

    Sets the \a linearVelocity of the body.
*/

/*!
    \qmlmethod void DynamicRigidBody::reset(vector3d position, vector3d eulerRotation)

    Resets the body's \a position and \a eulerRotation.
*/

QDynamicRigidBody::QDynamicRigidBody() = default;

QDynamicRigidBody::~QDynamicRigidBody()
{
    qDeleteAll(m_commandQueue);
    m_commandQueue.clear();
}

const QQuaternion &QDynamicRigidBody::centerOfMassRotation() const
{
    return m_centerOfMassRotation;
}

void QDynamicRigidBody::setCenterOfMassRotation(const QQuaternion &newCenterOfMassRotation)
{
    if (qFuzzyCompare(m_centerOfMassRotation, newCenterOfMassRotation))
        return;
    m_centerOfMassRotation = newCenterOfMassRotation;

    // Only inertia tensor is using rotation
    if (m_massMode == MassMode::MassAndInertiaTensor)
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaTensor(m_mass, m_inertiaTensor));

    emit centerOfMassRotationChanged();
}

const QVector3D &QDynamicRigidBody::centerOfMassPosition() const
{
    return m_centerOfMassPosition;
}

void QDynamicRigidBody::setCenterOfMassPosition(const QVector3D &newCenterOfMassPosition)
{
    if (qFuzzyCompare(m_centerOfMassPosition, newCenterOfMassPosition))
        return;

    switch (m_massMode) {
    case MassMode::MassAndInertiaTensor: {
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaTensor(m_mass, m_inertiaTensor));
        break;
    }
    case MassMode::MassAndInertiaMatrix: {
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaMatrix(m_mass, m_inertiaMatrix));
        break;
    }
    case MassMode::DefaultDensity:
    case MassMode::CustomDensity:
    case MassMode::Mass:
        break;
    }

    m_centerOfMassPosition = newCenterOfMassPosition;
    emit centerOfMassPositionChanged();
}

QDynamicRigidBody::MassMode QDynamicRigidBody::massMode() const
{
    return m_massMode;
}

void QDynamicRigidBody::setMassMode(const MassMode newMassMode)
{
    if (m_massMode == newMassMode)
        return;

    switch (newMassMode) {
    case MassMode::DefaultDensity: {
        auto world = QPhysicsWorld::getWorld(this);
        if (world) {
            m_commandQueue.enqueue(new QPhysicsCommandSetDensity(world->defaultDensity()));
        } else {
            qWarning() << "No physics world found, cannot set default density.";
        }
        break;
    }
    case MassMode::CustomDensity: {
        m_commandQueue.enqueue(new QPhysicsCommandSetDensity(m_density));
        break;
    }
    case MassMode::Mass: {
        m_commandQueue.enqueue(new QPhysicsCommandSetMass(m_mass));
        break;
    }
    case MassMode::MassAndInertiaTensor: {
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaTensor(m_mass, m_inertiaTensor));
        break;
    }
    case MassMode::MassAndInertiaMatrix: {
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaMatrix(m_mass, m_inertiaMatrix));
        break;
    }
    }

    m_massMode = newMassMode;
    emit massModeChanged();
}

const QVector3D &QDynamicRigidBody::inertiaTensor() const
{
    return m_inertiaTensor;
}

void QDynamicRigidBody::setInertiaTensor(const QVector3D &newInertiaTensor)
{
    if (qFuzzyCompare(m_inertiaTensor, newInertiaTensor))
        return;
    m_inertiaTensor = newInertiaTensor;

    if (m_massMode == MassMode::MassAndInertiaTensor)
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaTensor(m_mass, m_inertiaTensor));

    emit inertiaTensorChanged();
}

const QList<float> &QDynamicRigidBody::readInertiaMatrix() const
{
    return m_inertiaMatrixList;
}

static bool fuzzyEquals(const QList<float> &a, const QList<float> &b)
{
    if (a.length() != b.length())
        return false;

    const int length = a.length();
    for (int i = 0; i < length; i++)
        if (!qFuzzyCompare(a[i], b[i]))
            return false;

    return true;
}

void QDynamicRigidBody::setInertiaMatrix(const QList<float> &newInertiaMatrix)
{
    if (fuzzyEquals(m_inertiaMatrixList, newInertiaMatrix))
        return;

    m_inertiaMatrixList = newInertiaMatrix;
    const int elemsToCopy = qMin(m_inertiaMatrixList.length(), 9);
    memcpy(m_inertiaMatrix.data(), m_inertiaMatrixList.data(), elemsToCopy * sizeof(float));
    memset(m_inertiaMatrix.data() + elemsToCopy, 0, (9 - elemsToCopy) * sizeof(float));

    if (m_massMode == MassMode::MassAndInertiaMatrix)
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaMatrix(m_mass, m_inertiaMatrix));

    emit inertiaMatrixChanged();
}

const QMatrix3x3 &QDynamicRigidBody::inertiaMatrix() const
{
    return m_inertiaMatrix;
}

float QDynamicRigidBody::mass() const
{
    return m_mass;
}

bool QDynamicRigidBody::isKinematic() const
{
    return m_isKinematic;
}

bool QDynamicRigidBody::gravityEnabled() const
{
    return m_gravityEnabled;
}

void QDynamicRigidBody::setMass(float mass)
{
    if (mass < 0.f || qFuzzyCompare(m_mass, mass))
        return;

    switch (m_massMode) {
    case QDynamicRigidBody::MassMode::Mass:
        m_commandQueue.enqueue(new QPhysicsCommandSetMass(mass));
        break;
    case QDynamicRigidBody::MassMode::MassAndInertiaTensor:
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaTensor(mass, m_inertiaTensor));
        break;
    case QDynamicRigidBody::MassMode::MassAndInertiaMatrix:
        m_commandQueue.enqueue(new QPhysicsCommandSetMassAndInertiaMatrix(mass, m_inertiaMatrix));
        break;
    case QDynamicRigidBody::MassMode::DefaultDensity:
    case QDynamicRigidBody::MassMode::CustomDensity:
        break;
    }

    m_mass = mass;
    emit massChanged(m_mass);
}

float QDynamicRigidBody::density() const
{
    return m_density;
}

void QDynamicRigidBody::setDensity(float density)
{
    if (qFuzzyCompare(m_density, density))
        return;

    if (m_massMode == MassMode::CustomDensity)
        m_commandQueue.enqueue(new QPhysicsCommandSetDensity(density));

    m_density = density;
    emit densityChanged(m_density);
}

void QDynamicRigidBody::setIsKinematic(bool isKinematic)
{
    if (m_isKinematic == isKinematic)
        return;

    if (hasStaticShapes() && !isKinematic) {
        qWarning()
                << "Cannot make body containing trimesh/heightfield/plane non-kinematic, ignoring.";
        return;
    }

    m_isKinematic = isKinematic;
    m_commandQueue.enqueue(new QPhysicsCommandSetIsKinematic(m_isKinematic));
    emit isKinematicChanged(m_isKinematic);
}

void QDynamicRigidBody::setGravityEnabled(bool gravityEnabled)
{
    if (m_gravityEnabled == gravityEnabled)
        return;

    m_gravityEnabled = gravityEnabled;
    m_commandQueue.enqueue(new QPhysicsCommandSetGravityEnabled(m_gravityEnabled));
    emit gravityEnabledChanged();
}

void QDynamicRigidBody::setAngularVelocity(const QVector3D &angularVelocity)
{
    m_commandQueue.enqueue(new QPhysicsCommandSetAngularVelocity(angularVelocity));
}

QDynamicRigidBody::AxisLock QDynamicRigidBody::linearAxisLock() const
{
    return m_linearAxisLock;
}

void QDynamicRigidBody::setLinearAxisLock(AxisLock newAxisLockLinear)
{
    if (m_linearAxisLock == newAxisLockLinear)
        return;
    m_linearAxisLock = newAxisLockLinear;
    emit linearAxisLockChanged();
}

QDynamicRigidBody::AxisLock QDynamicRigidBody::angularAxisLock() const
{
    return m_angularAxisLock;
}

void QDynamicRigidBody::setAngularAxisLock(AxisLock newAxisLockAngular)
{
    if (m_angularAxisLock == newAxisLockAngular)
        return;
    m_angularAxisLock = newAxisLockAngular;
    emit angularAxisLockChanged();
}

QQueue<QPhysicsCommand *> &QDynamicRigidBody::commandQueue()
{
    return m_commandQueue;
}

void QDynamicRigidBody::updateDefaultDensity(float defaultDensity)
{
    if (m_massMode == MassMode::DefaultDensity)
        m_commandQueue.enqueue(new QPhysicsCommandSetDensity(defaultDensity));
}

void QDynamicRigidBody::applyCentralForce(const QVector3D &force)
{
    m_commandQueue.enqueue(new QPhysicsCommandApplyCentralForce(force));
}

void QDynamicRigidBody::applyForce(const QVector3D &force, const QVector3D &position)
{
    m_commandQueue.enqueue(new QPhysicsCommandApplyForce(force, position));
}

void QDynamicRigidBody::applyTorque(const QVector3D &torque)
{
    m_commandQueue.enqueue(new QPhysicsCommandApplyTorque(torque));
}

void QDynamicRigidBody::applyCentralImpulse(const QVector3D &impulse)
{
    m_commandQueue.enqueue(new QPhysicsCommandApplyCentralImpulse(impulse));
}

void QDynamicRigidBody::applyImpulse(const QVector3D &impulse, const QVector3D &position)
{
    m_commandQueue.enqueue(new QPhysicsCommandApplyImpulse(impulse, position));
}

void QDynamicRigidBody::applyTorqueImpulse(const QVector3D &impulse)
{
    m_commandQueue.enqueue(new QPhysicsCommandApplyTorqueImpulse(impulse));
}

void QDynamicRigidBody::setLinearVelocity(const QVector3D &linearVelocity)
{
    m_commandQueue.enqueue(new QPhysicsCommandSetLinearVelocity(linearVelocity));
}

void QDynamicRigidBody::reset(const QVector3D &position, const QVector3D &eulerRotation)
{
    m_commandQueue.enqueue(new QPhysicsCommandReset(position, eulerRotation));
}

void QDynamicRigidBody::setKinematicRotation(const QQuaternion &rotation)
{
    if (m_kinematicRotation == rotation)
        return;

    m_kinematicRotation = rotation;
    emit kinematicRotationChanged(m_kinematicRotation);
    emit kinematicEulerRotationChanged(m_kinematicRotation.getEulerRotation());
}

QQuaternion QDynamicRigidBody::kinematicRotation() const
{
    return m_kinematicRotation.getQuaternionRotation();
}

void QDynamicRigidBody::setKinematicEulerRotation(const QVector3D &rotation)
{
    if (m_kinematicRotation == rotation)
        return;

    m_kinematicRotation = rotation;
    emit kinematicEulerRotationChanged(m_kinematicRotation);
    emit kinematicRotationChanged(m_kinematicRotation.getQuaternionRotation());
}

QVector3D QDynamicRigidBody::kinematicEulerRotation() const
{
    return m_kinematicRotation.getEulerRotation();
}

void QDynamicRigidBody::setKinematicPivot(const QVector3D &pivot)
{
    m_kinematicPivot = pivot;
    emit kinematicPivotChanged(m_kinematicPivot);
}

QVector3D QDynamicRigidBody::kinematicPivot() const
{
    return m_kinematicPivot;
}

bool QDynamicRigidBody::isSleeping() const
{
    return m_isSleeping;
}

void QDynamicRigidBody::setIsSleeping(bool newIsSleeping)
{
    if (m_isSleeping == newIsSleeping)
        return;

    m_isSleeping = newIsSleeping;
    emit isSleepingChanged(newIsSleeping);
}

QAbstractPhysXNode *QDynamicRigidBody::createPhysXBackend()
{
    return new QPhysXDynamicBody(this);
}

void QDynamicRigidBody::setKinematicPosition(const QVector3D &position)
{
    m_kinematicPosition = position;
    emit kinematicPositionChanged(m_kinematicPosition);
}

QVector3D QDynamicRigidBody::kinematicPosition() const
{
    return m_kinematicPosition;
}

QT_END_NAMESPACE
