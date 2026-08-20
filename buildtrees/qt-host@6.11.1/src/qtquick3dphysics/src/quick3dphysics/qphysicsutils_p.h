// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QPHYSICSUTILS_P_H
#define QPHYSICSUTILS_P_H

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

#include <QVector3D>
#include <QQuaternion>
#include <foundation/PxTransform.h>
#include <foundation/PxMat33.h>
#include <foundation/PxQuat.h>
#include <foundation/PxVec3.h>

namespace physx {
class PxRigidBody;
}

namespace QPhysicsUtils {

Q_ALWAYS_INLINE physx::PxVec3 toPhysXType(const QVector3D &qvec)
{
    return physx::PxVec3(qvec.x(), qvec.y(), qvec.z());
}

Q_ALWAYS_INLINE physx::PxQuat toPhysXType(const QQuaternion &qquat)
{
    return physx::PxQuat(qquat.x(), qquat.y(), qquat.z(), qquat.scalar());
}

Q_ALWAYS_INLINE physx::PxMat33 toPhysXType(const QMatrix3x3 &m)
{
    return physx::PxMat33(const_cast<float *>(m.constData()));
}

Q_ALWAYS_INLINE QVector3D toQtType(const physx::PxVec3 &vec)
{
    return QVector3D(vec.x, vec.y, vec.z);
}

Q_ALWAYS_INLINE QQuaternion toQtType(const physx::PxQuat &quat)
{
    return QQuaternion(quat.w, quat.x, quat.y, quat.z);
}

Q_ALWAYS_INLINE physx::PxTransform toPhysXTransform(const QVector3D &position,
                                                    const QQuaternion &rotation)
{
    return physx::PxTransform(QPhysicsUtils::toPhysXType(position),
                              QPhysicsUtils::toPhysXType(rotation));
}

Q_ALWAYS_INLINE bool fuzzyEquals(const physx::PxTransform &a, const physx::PxTransform &b)
{
    return qFuzzyCompare(a.p.x, b.p.x) && qFuzzyCompare(a.p.y, b.p.y) && qFuzzyCompare(a.p.z, b.p.z)
            && qFuzzyCompare(a.q.x, b.q.x) && qFuzzyCompare(a.q.y, b.q.y)
            && qFuzzyCompare(a.q.z, b.q.z) && qFuzzyCompare(a.q.w, b.q.w);
}

inline const QQuaternion kMinus90YawRotation = QQuaternion::fromEulerAngles(0, -90, 0);
}

#endif // QPHYSICSUTILS_P_H
