// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticletrailemitter_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype TrailEmitter3D
    \inherits ParticleEmitter3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Emitter for logical particles from other particles.
    \since 6.2

    TrailEmitter3D is a special emitter for emitting particles with starting positions inherited from
    other logical particles.
*/

QQuick3DParticleTrailEmitter::QQuick3DParticleTrailEmitter(QQuick3DNode *parent)
    : QQuick3DParticleEmitter(parent)
{
}

/*!
    \qmlproperty Particle3D TrailEmitter3D::follow

    This property defines the logical particle which this emitter follows.
    When the TrailEmitter3D emits particles, center position of those particles
    will become from the \l Particle3D the emitter follows.
*/
QQuick3DParticle *QQuick3DParticleTrailEmitter::follow() const
{
    return m_follow;
}

void QQuick3DParticleTrailEmitter::setFollow(QQuick3DParticle *follow)
{
    if (m_follow == follow)
        return;

    m_follow = follow;
    Q_EMIT followChanged();
}

/*!
    \qmlmethod vector3d TrailEmitter3D::burst(int count)

    This method emits \a count amount of particles from this emitter immediately.

    \note TrailEmitter3D doesn't support other bursting methods. Position always comes
    from the particle defined with the \l follow property.
*/
void QQuick3DParticleTrailEmitter::burst(int count)
{
    if (!system())
        return;
    QQuick3DParticleEmitBurstData burst;
    burst.time = system()->currentTime();
    burst.amount = count;
    m_bursts << burst;
}

// Returns true if there are any dynamic bursts
bool QQuick3DParticleTrailEmitter::hasBursts() const
{
    bool dynamicBursts = false;
    for (auto *burst : std::as_const(m_emitBursts)) {
        if (qobject_cast<QQuick3DParticleDynamicBurst *>(burst)) {
            dynamicBursts = true;
            break;
        }
    }
    return !m_bursts.empty() || dynamicBursts;
}

// Called to emit set of particles
void QQuick3DParticleTrailEmitter::emitTrailParticles(const QVector3D &centerPos, int emitAmount, int triggerType, const QVector3D &normal, const QVector3D &velocity)
{
    if (!system())
        return;

    if (!enabled())
        return;

    const int systemTime = system()->currentTime();
    for (auto particle : std::as_const(m_system->m_particles)) {
        if (particle == m_particle) {
            emitAmount += getEmitAmountFromDynamicBursts(triggerType);
            emitAmount = std::min(emitAmount, int(particle->maxAmount()));
            float addTime = ((systemTime - m_prevEmitTime) / 1000.0f) / emitAmount;
            for (int i = 0; i < emitAmount; i++) {
                // Distribute evenly between previous and current time, important especially
                // when time has jumped a lot (like a starttime).
                float startTime = (m_prevEmitTime / 1000.0f) + addTime * float(i);
                emitParticle(particle, startTime, QMatrix4x4(), QQuaternion(), centerPos, -1, velocity, normal);
            }
            // Emit bursts, if any
            for (auto burst : std::as_const(m_bursts)) {
                int burstAmount = std::min(burst.amount, int(particle->maxAmount()));
                float burstTime = float(burst.time / 1000.0f);
                for (int i = 0; i < burstAmount; i++)
                    emitParticle(particle, burstTime, QMatrix4x4(), QQuaternion(), centerPos, -1, velocity, normal);
            }
        }
    }

    m_prevEmitTime = systemTime;
}

void QQuick3DParticleTrailEmitter::clearBursts()
{
    // After bursts have been emitted, clear the list
    m_bursts.clear();
}

QT_END_NAMESPACE
