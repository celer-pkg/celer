// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlevectordirection_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticlesystem_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype VectorDirection3D
    \inherits Direction3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief For specifying a direction towards the target direction.
    \since 6.2

    This element sets emitted particle velocity towards the target direction vector.
    The length of the direction vector is used as the velocity magnitude.

    For example, to emit particles towards some random direction within
    x: 50..150, y: -20..20, z: 0:

    \qml
    ParticleEmitter3D {
        ...
        velocity: VectorDirection3D {
            direction: Qt.vector3d(100, 0, 0)
            directionVariation: Qt.vector3d(50, 20, 0)
        }
    }
    \endqml

*/

QQuick3DParticleVectorDirection::QQuick3DParticleVectorDirection(QObject *parent)
    : QQuick3DParticleDirection(parent)
{
}

/*!
    \qmlproperty vector3d VectorDirection3D::direction

    This property defines the direction for particles target.

    The default value is \c (0, 100, 0) (upwards on the y-axis).

    \sa directionVariation
*/
QVector3D QQuick3DParticleVectorDirection::direction() const
{
    return m_direction;
}

/*!
    \qmlproperty vector3d VectorDirection3D::directionVariation

    This property defines the direction variation for particles target.

    The default value is \c (0, 0, 0) (no variation).

    \sa direction
*/
QVector3D QQuick3DParticleVectorDirection::directionVariation() const
{
    return m_directionVariation;
}

/*!
    \qmlproperty bool VectorDirection3D::normalized

    This property defines if the direction should be normalized after applying the variation.
    When this is false, variation affects the magnitude of the particles velocity.
    When set to true, variation affects the direction, but the magnitude is determined
    by the original direction length.

    The default value is \c false.
*/
bool QQuick3DParticleVectorDirection::normalized() const
{
    return m_normalized;
}

void QQuick3DParticleVectorDirection::setDirection(const QVector3D &direction)
{
    if (m_direction == direction)
        return;

    m_direction = direction;
    Q_EMIT directionChanged();
}

void QQuick3DParticleVectorDirection::setDirectionVariation(const QVector3D &directionVariation)
{
    if (m_directionVariation == directionVariation)
        return;

    m_directionVariation = directionVariation;
    Q_EMIT directionVariationChanged();
}

void QQuick3DParticleVectorDirection::setNormalized(bool normalized)
{
    if (m_normalized == normalized)
        return;

    m_normalized = normalized;
    Q_EMIT normalizedChanged();
}

QVector3D QQuick3DParticleVectorDirection::sample(const QQuick3DParticleData &d)
{
    QVector3D ret;
    if (!m_system)
        return ret;
    auto rand = m_system->rand();
    ret.setX(m_direction.x() - m_directionVariation.x() + rand->get(d.index, QPRand::VDirXV) * m_directionVariation.x() * 2.0f);
    ret.setY(m_direction.y() - m_directionVariation.y() + rand->get(d.index, QPRand::VDirYV) * m_directionVariation.y() * 2.0f);
    ret.setZ(m_direction.z() - m_directionVariation.z() + rand->get(d.index, QPRand::VDirZV) * m_directionVariation.z() * 2.0f);
    if (m_normalized)
        ret = m_direction.length() * ret.normalized();
    return ret;
}

QT_END_NAMESPACE
