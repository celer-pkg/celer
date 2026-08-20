// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlegravity_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Gravity3D
    \inherits Affector3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Accelerates particles to a vector of the specified magnitude in the specified direction.
    \since 6.2

    This element models the gravity of a massive object whose center of gravity is far away (and thus the
    gravitational pull is effectively constant across the scene). To model the gravity of an object near
    or inside the scene, use \l Attractor3D.
*/

QQuick3DParticleGravity::QQuick3DParticleGravity(QQuick3DNode *parent)
    : QQuick3DParticleAffector(parent)
{
}

/*!
    \qmlproperty real Gravity3D::magnitude

    This property defines the magnitude in position change per second. Negative magnitude
    accelerates the opposite way from the \l {Gravity3D::direction}{direction}.

    The default value is \c 100.0.
*/
float QQuick3DParticleGravity::magnitude() const
{
    return m_magnitude;
}

void QQuick3DParticleGravity::setMagnitude(float magnitude)
{
    if (qFuzzyCompare(m_magnitude, magnitude))
        return;

    m_magnitude = magnitude;
    Q_EMIT magnitudeChanged();
    Q_EMIT update();
}

/*!
    \qmlproperty vector3d Gravity3D::direction

    This property defines the direction the gravity will affect toward. Values will be
    automatically normalized to a unit vector.

    The default value is \c (0.0, -1.0, 0.0) (downwards).
*/
const QVector3D &QQuick3DParticleGravity::direction() const
{
    return m_direction;
}

void QQuick3DParticleGravity::setDirection(const QVector3D &direction)
{
    if (m_direction == direction)
        return;

    m_direction = direction;
    m_directionNormalized = m_direction.normalized();
    Q_EMIT directionChanged();
    Q_EMIT update();
}

void QQuick3DParticleGravity::affectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time)
{
    Q_UNUSED(sd);
    float velocity = 0.5f * m_magnitude * (time * time);
    d->position += velocity * m_directionNormalized;
}

QT_END_NAMESPACE
