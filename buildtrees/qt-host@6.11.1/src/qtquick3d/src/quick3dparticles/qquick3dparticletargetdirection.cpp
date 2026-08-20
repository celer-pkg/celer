// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticletargetdirection_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticlesystem_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype TargetDirection3D
    \inherits Direction3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief For specifying a direction towards the target position.
    \since 6.2

    This element sets emitted particle velocity towards the target position.

    For example, to emit particles towards position (100, 0, 0) with
    random magnitude between 10..20:

    \qml
    ParticleEmitter3D {
        ...
        velocity: TargetDirection3D {
            position: Qt.vector3d(100, 0, 0)
            normalized: true
            magnitude: 15.0
            magnitudeVariation: 5.0
        }
    }
    \endqml
*/

QQuick3DParticleTargetDirection::QQuick3DParticleTargetDirection(QObject *parent)
    : QQuick3DParticleDirection(parent)
{
}

/*!
    \qmlproperty vector3d TargetDirection3D::position

    This property defines the position for particles target.

    The default value is \c (0, 0, 0) (the center of the emitter).

    \sa positionVariation
*/
QVector3D QQuick3DParticleTargetDirection::position() const
{
    return m_position;
}

/*!
    \qmlproperty vector3d TargetDirection3D::positionVariation

    This property defines the position variation for particles target.

    The default value is \c (0, 0, 0) (no variation).

    \sa position
*/
QVector3D QQuick3DParticleTargetDirection::positionVariation() const
{
    return m_positionVariation;
}

void QQuick3DParticleTargetDirection::setPosition(const QVector3D &position)
{
    if (m_position == position)
        return;

    m_position = position;
    Q_EMIT positionChanged();
}

void QQuick3DParticleTargetDirection::setPositionVariation(const QVector3D &positionVariation)
{
    if (m_positionVariation == positionVariation)
        return;

    m_positionVariation = positionVariation;
    Q_EMIT positionVariationChanged();
}

/*!
    \qmlproperty bool TargetDirection3D::normalized

    This property defines if the distance to \l position should be considered as normalized or not.
    When this is false, distance to the \l position affects the magnitude of the particles velocity.
    When set to true, distance is normalized and velocity amount comes only from \l magnitude and
    \l magnitudeVariation.

    The default value is \c false.

    \sa magnitude, magnitudeVariation
*/
bool QQuick3DParticleTargetDirection::normalized() const
{
    return m_normalized;
}

void QQuick3DParticleTargetDirection::setNormalized(bool normalized)
{
    if (m_normalized == normalized)
        return;

    m_normalized = normalized;
    Q_EMIT normalizedChanged();
}

/*!
    \qmlproperty real TargetDirection3D::magnitude

    This property defines the magnitude in position change per second. Negative magnitude
    accelerates the opposite way from the \l {TargetDirection3D::position}{position}.
    When the \l normalized is false, this is multiplied with the distance to the target position.

    The default value is \c 1.0.

    \sa magnitudeVariation
*/
float QQuick3DParticleTargetDirection::magnitude() const
{
    return m_magnitude;
}

void QQuick3DParticleTargetDirection::setMagnitude(float magnitude)
{
    if (qFuzzyCompare(m_magnitude, magnitude))
        return;

    m_magnitude = magnitude;
    Q_EMIT magnitudeChanged();
}

/*!
    \qmlproperty real TargetDirection3D::magnitudeVariation

    This property defines the magnitude variation in position change per second.
    When the \l normalized is false, this is multiplied with the distance to the target position.

    The default value is \c 0.0.

    \sa magnitude
*/
float QQuick3DParticleTargetDirection::magnitudeVariation() const
{
    return m_magnitudeVariation;
}

void QQuick3DParticleTargetDirection::setMagnitudeVariation(float magnitudeVariation)
{
    if (qFuzzyCompare(m_magnitudeVariation, magnitudeVariation))
        return;

    m_magnitudeVariation = magnitudeVariation;
    Q_EMIT magnitudeChangedVariation();
}

QVector3D QQuick3DParticleTargetDirection::sample(const QQuick3DParticleData &d)
{
    QVector3D ret = m_position - d.startPosition;
    if (!m_system)
        return ret;
    auto rand = m_system->rand();

    ret.setX(ret.x() - m_positionVariation.x() + rand->get(d.index, QPRand::TDirPosXV) * m_positionVariation.x() * 2.0f);
    ret.setY(ret.y() - m_positionVariation.y() + rand->get(d.index, QPRand::TDirPosYV) * m_positionVariation.y() * 2.0f);
    ret.setZ(ret.z() - m_positionVariation.z() + rand->get(d.index, QPRand::TDirPosZV) * m_positionVariation.z() * 2.0f);
    if (m_normalized)
        ret.normalize();
    ret *= (m_magnitude - m_magnitudeVariation + rand->get(d.index, QPRand::TDirMagV) * m_magnitudeVariation * 2.0f);
    return ret;
}

QT_END_NAMESPACE
