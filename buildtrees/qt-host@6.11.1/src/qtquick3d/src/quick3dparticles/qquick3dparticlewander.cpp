// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/qmath.h>
#include "qquick3dparticlewander_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticleutils_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Wander3D
    \inherits Affector3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Applies random wave curves to particles.
    \since 6.2

    This element applies random wave curves to particles. Curves can combine
    global values which are the same for all particles and unique values which
    differ randomly.
*/

QQuick3DParticleWander::QQuick3DParticleWander(QQuick3DNode *parent)
    : QQuick3DParticleAffector(parent)
{
}

/*!
    \qmlproperty vector3d Wander3D::globalAmount

    This property defines how long distance each particle moves at the ends of curves.
    So if the value is for example (100, 10, 0), all particles wander between (100, 10, 0)
    and (-100, -10, 0).

    The default value is \c (0.0, 0.0, 0.0).
*/
const QVector3D &QQuick3DParticleWander::globalAmount() const
{
    return m_globalAmount;
}

/*!
    \qmlproperty vector3d Wander3D::globalPace

    This property defines the pace (frequency) each particle wanders in curves per second.

    The default value is \c (0.0, 0.0, 0.0).
*/
const QVector3D &QQuick3DParticleWander::globalPace() const
{
    return m_globalPace;
}

/*!
    \qmlproperty vector3d Wander3D::globalPaceStart

    This property defines the starting point for the pace (frequency). The meaningful range
    is between 0 .. 2 * PI. For example, to animate the x-coordinate of the pace start:

    \qml
    PropertyAnimation on globalPaceStart {
        loops: Animation.Infinite
        duration: 2000
        from: Qt.vector3d(0, 0, 0)
        to: Qt.vector3d(Math.PI * 2, 0, 0)
    }
    \endqml

    The default value is \c (0.0, 0.0, 0.0).
*/
const QVector3D &QQuick3DParticleWander::globalPaceStart() const
{
    return m_globalPaceStart;
}

/*!
    \qmlproperty vector3d Wander3D::uniqueAmount

    This property defines how long distance each particle moves at the ends of curves at maximum.

    The default value is \c (0.0, 0.0, 0.0).
*/
const QVector3D &QQuick3DParticleWander::uniqueAmount() const
{
    return m_uniqueAmount;
}

/*!
    \qmlproperty vector3d Wander3D::uniquePace

    This property defines the unique pace (frequency) each particle wanders in curves per second.

    The default value is \c (0.0, 0.0, 0.0).
*/
const QVector3D &QQuick3DParticleWander::uniquePace() const
{
    return m_uniquePace;
}

/*!
    \qmlproperty real Wander3D::uniqueAmountVariation

    This property defines variation for \l uniqueAmount between 0.0 and 1.0.
    When the amount variation is 0.0, every particle reaches maximum amount. When it's 0.5,
    every particle reaches between 0.5 - 1.5 of the amount.
    For example if \l uniqueAmount is (100, 50, 20) and \l uniqueAmountVariation is 0.1,
    the particles maximum wave distances are something random between (110, 55, 22)
    and (90, 45, 18).

    The default value is \c 0.0.
*/
float QQuick3DParticleWander::uniqueAmountVariation() const
{
    return m_uniqueAmountVariation;
}

/*!
    \qmlproperty real Wander3D::uniquePaceVariation

    This property defines the unique pace (frequency) variation for each particle
    between 0.0 and 1.0. When the variation is 0.0, every particle wander at the same
    frequency. For example if \l uniquePace is (1.0, 2.0, 4.0) and \l uniquePaceVariation
    is 0.5, the particles wave paces are something random between (2.0, 4.0, 8.0)
    and (0.5, 1.0, 2.0).

    The default value is \c 0.0.
*/
float QQuick3DParticleWander::uniquePaceVariation() const
{
    return m_uniquePaceVariation;
}

/*!
    \qmlproperty int Wander3D::fadeInDuration

    This property defines the duration in milliseconds for fading in the affector.
    After this duration, the wandering will be in full effect.
    Setting this can be useful to emit from a specific position or shape,
    otherwise wander will affect position also at the beginning.

    The default value is \c 0.
*/
int QQuick3DParticleWander::fadeInDuration() const
{
    return m_fadeInDuration;
}

/*!
    \qmlproperty int Wander3D::fadeOutDuration

    This property defines the duration in milliseconds for fading out the affector.
    Setting this can be useful to reduce the wander when the particle life
    time ends, for example when combined with \l Attractor3D so end positions will
    match the \l{Attractor3D::shape}{shape}.

    The default value is \c 0.
*/
int QQuick3DParticleWander::fadeOutDuration() const
{
    return m_fadeOutDuration;
}

void QQuick3DParticleWander::setGlobalAmount(const QVector3D &globalAmount)
{
    if (m_globalAmount == globalAmount)
        return;

    m_globalAmount = globalAmount;
    Q_EMIT globalAmountChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setGlobalPace(const QVector3D &globalPace)
{
    if (m_globalPace == globalPace)
        return;

    m_globalPace = globalPace;
    Q_EMIT globalPaceChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setGlobalPaceStart(const QVector3D &globalPaceStart)
{
    if (m_globalPaceStart == globalPaceStart)
        return;

    m_globalPaceStart = globalPaceStart;
    Q_EMIT globalPaceStartChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setUniqueAmount(const QVector3D &uniqueAmount)
{
    if (m_uniqueAmount == uniqueAmount)
        return;

    m_uniqueAmount = uniqueAmount;
    Q_EMIT uniqueAmountChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setUniquePace(const QVector3D &uniquePace)
{
    if (m_uniquePace == uniquePace)
        return;

    m_uniquePace = uniquePace;
    Q_EMIT uniquePaceChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setUniqueAmountVariation(float uniqueAmountVariation)
{
    if (qFuzzyCompare(m_uniqueAmountVariation, uniqueAmountVariation))
        return;

    uniqueAmountVariation = std::max(0.0f, std::min(1.0f, uniqueAmountVariation));
    m_uniqueAmountVariation = uniqueAmountVariation;
    Q_EMIT uniqueAmountVariationChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setUniquePaceVariation(float uniquePaceVariation)
{
    if (qFuzzyCompare(m_uniquePaceVariation, uniquePaceVariation))
        return;

    uniquePaceVariation = std::max(0.0f, std::min(1.0f, uniquePaceVariation));
    m_uniquePaceVariation = uniquePaceVariation;
    Q_EMIT uniquePaceVariationChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setFadeInDuration(int fadeInDuration)
{
    if (m_fadeInDuration == fadeInDuration)
        return;

    m_fadeInDuration = std::max(0, fadeInDuration);
    Q_EMIT fadeInDurationChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::setFadeOutDuration(int fadeOutDuration)
{
    if (m_fadeOutDuration == fadeOutDuration)
        return;

    m_fadeOutDuration = std::max(0, fadeOutDuration);
    Q_EMIT fadeOutDurationChanged();
    Q_EMIT update();
}

void QQuick3DParticleWander::affectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time)
{
    if (!system())
        return;
    auto rand = system()->rand();

    // Optionally smoothen the beginning & end of wander
    float smooth = 1.0f;
    if (m_fadeInDuration > 0) {
        smooth = time / (float(m_fadeInDuration) / 1000.0f);
        smooth = std::min(1.0f, smooth);
    }
    if (m_fadeOutDuration > 0) {
        float timeLeft = (sd.lifetime - time);
        float smoothOut = timeLeft / (float(m_fadeOutDuration) / 1000.0f);
        // When fading both in & out, select smaller (which is always max 1.0)
        smooth = std::min(smoothOut, smooth);
    }

    const float pi2 = float(M_PI * 2);
    // Global
    if (!qFuzzyIsNull(m_globalAmount.x()) && !qFuzzyIsNull(m_globalPace.x()))
        d->position.setX(d->position.x() + smooth * QPSIN(m_globalPaceStart.x() + time * pi2 * m_globalPace.x()) * m_globalAmount.x());
    if (!qFuzzyIsNull(m_globalAmount.y()) && !qFuzzyIsNull(m_globalPace.y()))
        d->position.setY(d->position.y() + smooth * QPSIN(m_globalPaceStart.y() + time * pi2 * m_globalPace.y()) * m_globalAmount.y());
    if (!qFuzzyIsNull(m_globalAmount.z()) && !qFuzzyIsNull(m_globalPace.z()))
        d->position.setZ(d->position.z() + smooth * QPSIN(m_globalPaceStart.z() + time * pi2 * m_globalPace.z()) * m_globalAmount.z());

    // Unique
    // Rather simple to only use a single sin operation per direction
    if (!qFuzzyIsNull(m_uniqueAmount.x()) && !qFuzzyIsNull(m_uniquePace.x())) {
        // Values between  1.0 +/- variation
        float paceVariation = 1.0f + m_uniquePaceVariation - 2.0f * rand->get(sd.index, QPRand::WanderXPV) * m_uniquePaceVariation;
        float amountVariation = 1.0f + m_uniqueAmountVariation - 2.0f * rand->get(sd.index, QPRand::WanderXAV) * m_uniqueAmountVariation;
        float startPace = rand->get(sd.index, QPRand::WanderXPS) * pi2;
        float pace = startPace + paceVariation * time * pi2 * m_uniquePace.x();
        float amount = amountVariation * m_uniqueAmount.x();
        d->position.setX(d->position.x() + smooth * QPSIN(pace) * amount);
    }
    if (!qFuzzyIsNull(m_uniqueAmount.y()) && !qFuzzyIsNull(m_uniquePace.y())) {
        // Values between  1.0 +/- variation
        float paceVariation = 1.0f + m_uniquePaceVariation - 2.0f * rand->get(sd.index, QPRand::WanderYPV) * m_uniquePaceVariation;
        float amountVariation = 1.0f + m_uniqueAmountVariation - 2.0f * rand->get(sd.index, QPRand::WanderYAV) * m_uniqueAmountVariation;
        float startPace = rand->get(sd.index, QPRand::WanderYPS) * pi2;
        float pace = startPace + paceVariation * time * pi2 * m_uniquePace.y();
        float amount = amountVariation * m_uniqueAmount.y();
        d->position.setY(d->position.y() + smooth * QPSIN(pace) * amount);
    }
    if (!qFuzzyIsNull(m_uniqueAmount.z()) && !qFuzzyIsNull(m_uniquePace.z())) {
        // Values between  1.0 +/- variation
        float paceVariation = 1.0f + m_uniquePaceVariation - 2.0f * rand->get(sd.index, QPRand::WanderZPV) * m_uniquePaceVariation;
        float amountVariation = 1.0f + m_uniqueAmountVariation - 2.0f * rand->get(sd.index, QPRand::WanderZAV) * m_uniqueAmountVariation;
        float startPace = rand->get(sd.index, QPRand::WanderZPS) * pi2;
        float pace = startPace + paceVariation * time * pi2 * m_uniquePace.z();
        float amount = amountVariation * m_uniqueAmount.z();
        d->position.setZ(d->position.z() + smooth * QPSIN(pace) * amount);
    }
}

QT_END_NAMESPACE
