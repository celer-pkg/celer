// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3D/private/qquick3dquaternionutils_p.h>
#include "qquick3dparticlesystem_p.h"
#include "qquick3dparticleemitter_p.h"
#include "qquick3dparticletrailemitter_p.h"
#include "qquick3dparticlemodelparticle_p.h"
#include "qquick3dparticleaffector_p.h"
#include <private/qqmldelegatemodel_p.h>
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticlespriteparticle_p.h"
#include "qquick3dparticlelineparticle_p.h"
#include "qquick3dparticlemodelblendparticle_p.h"
#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>
#include <qtquick3d_tracepoints_p.h>

#include <QtGui/qquaternion.h>

#include <cmath>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleSystem3D
    \inherits Node
    \inqmlmodule QtQuick3D.Particles3D
    \brief A system which includes particle, emitter, and affector types.
    \since 6.2

    This element is the root of the particle system, which handles the system timing and groups all
    the other related elements like particles, emitters, and affectors together. To group the system
    elements, they either need to be direct children of the ParticleSystem3D like this:

    \qml,
    ParticleSystem3D {
        ParticleEmitter3D {
            ...
        }
        SpriteParticle3D {
            ...
        }
    }
    \endqml

    Or if the system elements are not direct children, they need to use \c system property to point
    which ParticleSystem3D they belong to. Like this:

    \qml
    ParticleSystem3D {
        id: psystem
    }
    ParticleEmitter3D {
        system: psystem
        ...
    }
    SpriteParticle3D {
        system: psystem
        ...
    }
    \endqml
*/

Q_TRACE_POINT(qtquick3d, QSSG_particleUpdate_entry);
Q_TRACE_POINT(qtquick3d, QSSG_particleUpdate_exit, int particleCount);

QQuick3DParticleSystem::QQuick3DParticleSystem(QQuick3DNode *parent)
    : QQuick3DNode(parent)
    , m_running(true)
    , m_paused(false)
    , m_initialized(false)
    , m_componentComplete(false)
    , m_animation(new QQuick3DParticleSystemAnimation(this))
    , m_updateAnimation(new QQuick3DParticleSystemUpdate(this))
    , m_logging(false)
    , m_loggingData(new QQuick3DParticleSystemLogging(this))
{
    connect(m_loggingData, &QQuick3DParticleSystemLogging::loggingIntervalChanged, &m_loggingTimer, [this]() {
        m_loggingTimer.setInterval(m_loggingData->m_loggingInterval);
    });
}

QQuick3DParticleSystem::~QQuick3DParticleSystem()
{
    m_animation->stop();
    m_updateAnimation->stop();

    for (const auto &connection : std::as_const(m_connections))
        QObject::disconnect(connection);
    // purposeful copy
    const auto particles = m_particles;
    const auto emitters = m_emitters;
    const auto trailEmitters = m_trailEmitters;
    const auto affectors = m_affectors;
    for (auto *particle : particles)
        particle->setSystem(nullptr);
    for (auto *emitter : emitters)
        emitter->setSystem(nullptr);
    for (auto *emitter : trailEmitters)
        emitter->setSystem(nullptr);
    for (auto *affector : affectors)
        affector->setSystem(nullptr);
}

/*!
    \qmlproperty bool ParticleSystem3D::running

    This property defines if system is currently running. If running is set to \c false,
    the particle system will stop the simulation. All particles will be destroyed when
    the system is set to running again.

    Running should be set to \c false when manually modifying/animating the \l {ParticleSystem3D::time}{time} property.

    The default value is \c true.
*/
bool QQuick3DParticleSystem::isRunning() const
{
    return m_running;
}

/*!
    \qmlproperty bool ParticleSystem3D::paused

    This property defines if system is currently paused. If paused is set to \c true, the
    particle system will not advance the simulation. When paused is set to \c false again,
    the simulation will resume from the same point where it was paused.

    The default value is \c false.
*/
bool QQuick3DParticleSystem::isPaused() const
{
    return m_paused;
}

/*!
    \qmlproperty int ParticleSystem3D::startTime

    This property defines time in milliseconds where the system starts. This can be useful
    to warm up the system so that a set of particles has already been emitted. If for example
    \l startTime is set to 2000 and system \l time is animating from 0 to 1000, actually
    animation shows particles from 2000 to 3000ms.

    The default value is \c 0.
*/
int QQuick3DParticleSystem::startTime() const
{
    return m_startTime;
}

/*!
    \qmlproperty int ParticleSystem3D::time

    This property defines time in milliseconds for the system.
    \note When modifying the time property, \l {ParticleSystem3D::running}{running}
    should usually be set to \c false.

    Here is an example how to manually animate the system for 3 seconds, in a loop, at half speed:

    \qml
    ParticleSystem3D {
        running: false
        NumberAnimation on time {
            loops: Animation.Infinite
            from: 0
            to: 3000
            duration: 6000
        }
    }
    \endqml
*/
int QQuick3DParticleSystem::time() const
{
    return m_time;
}

/*!
    \qmlproperty bool ParticleSystem3D::useRandomSeed

    This property defines if particle system seed should be random or user defined.
    When \c true, a new random value for \l {ParticleSystem3D::seed}{seed} is generated every time particle
    system is restarted.

    The default value is \c true.

    \note This property should not be modified during the particle animations.

    \sa seed
*/
bool QQuick3DParticleSystem::useRandomSeed() const
{
    return m_useRandomSeed;
}

/*!
    \qmlproperty int ParticleSystem3D::seed

    This property defines the seed value used for particles randomization. With the same seed,
    particles effect will be identical on every run. This is useful when deterministic behavior
    is desired over random behavior.

    The default value is \c 0 when \l {ParticleSystem3D::useRandomSeed}{useRandomSeed} is set to
    \c false, and something in between \c 1..INT32_MAX when \l {ParticleSystem3D::useRandomSeed}{useRandomSeed}
    is set to \c true.

    \note This property should not be modified during the particle animations.

    \sa useRandomSeed
*/
int QQuick3DParticleSystem::seed() const
{
    return m_seed;
}

/*!
    \qmlproperty bool ParticleSystem3D::logging

    Set this to true to collect \l {ParticleSystem3D::loggingData}{loggingData}.

    \note This property has some performance impact, so it should not be enabled in releases.

    The default value is \c false.

    \sa loggingData
*/
bool QQuick3DParticleSystem::logging() const
{
    return m_logging;
}

/*!
    \qmlproperty ParticleSystem3DLogging ParticleSystem3D::loggingData
    \readonly

    This property contains logging data which can be useful when developing and optimizing
    the particle effects.

    \note This property contains correct data only when \l {ParticleSystem3D::logging}{logging} is set
    to \c true and particle system is running.

    \sa logging
*/
QQuick3DParticleSystemLogging *QQuick3DParticleSystem::loggingData() const
{
    return m_loggingData;
}

/*!
    \qmlmethod void ParticleSystem3D::reset()

    This method resets the internal state of the particle system to it's initial state.
    This can be used when \l running property is \c false to reset the system.
    The \l running is \c true this method does not need to be called as the system is managing
    the internal state, but when it is \c false the system needs to be told when the system should
    be reset.
*/
void QQuick3DParticleSystem::reset()
{
    for (auto emitter : std::as_const(m_emitters))
        emitter->reset();
    for (auto emitter : std::as_const(m_trailEmitters))
        emitter->reset();
    for (auto particle : std::as_const(m_particles))
        particle->reset();
    m_particleIdIndex = 0;
}

/*!
    Returns the current time of the system (m_time + m_startTime).
    \internal
*/
int QQuick3DParticleSystem::currentTime() const
{
    return m_currentTime;
}

void QQuick3DParticleSystem::setRunning(bool running)
{
    if (m_running != running) {
        m_running = running;
        Q_EMIT runningChanged();
        setPaused(false);

        if (m_running)
            reset();

        if (m_componentComplete && !m_running && m_useRandomSeed)
            doSeedRandomization();

        (m_running && !isEditorModeOn()) ? m_animation->start() : m_animation->stop();
    }
}

void QQuick3DParticleSystem::setPaused(bool paused)
{
    if (m_paused != paused) {
        m_paused = paused;
        if (m_animation->state() != QAbstractAnimation::Stopped)
            m_paused ? m_animation->pause() : m_animation->resume();
        Q_EMIT pausedChanged();
    }
}

void QQuick3DParticleSystem::setStartTime(int startTime)
{
    if (m_startTime == startTime)
        return;

    m_startTime = startTime;
    m_updateAnimation->setDirty(true);
    Q_EMIT startTimeChanged();
}

void QQuick3DParticleSystem::setTime(int time)
{
    if (m_time == time)
        return;

    // Update the time and mark the system dirty
    m_time = time;
    m_updateAnimation->setDirty(true);

    Q_EMIT timeChanged();
}

void QQuick3DParticleSystem::setUseRandomSeed(bool randomize)
{
    if (m_useRandomSeed == randomize)
        return;

    m_useRandomSeed = randomize;
    // When set to true, random values are recalculated with a random seed
    // and random values will become independent of particle index when possible.
    if (m_useRandomSeed)
        doSeedRandomization();
    m_rand.setDeterministic(!m_useRandomSeed);
    Q_EMIT useRandomSeedChanged();
}

void QQuick3DParticleSystem::setSeed(int seed)
{
    if (m_seed == seed)
        return;

    m_seed = seed;
    m_rand.init(m_seed);
    Q_EMIT seedChanged();
}

void QQuick3DParticleSystem::setLogging(bool logging)
{
    if (m_logging == logging)
        return;

    m_logging = logging;

    resetLoggingVariables();
    m_loggingData->resetData();

    if (m_logging)
        m_loggingTimer.start();
    else
        m_loggingTimer.stop();

    Q_EMIT loggingChanged();
}

/*!
    Set editor time which in editor mode overwrites the time.
    \internal
*/
void QQuick3DParticleSystem::setEditorTime(int time)
{
    if (m_editorTime == time)
        return;

    // Update the time and mark the system dirty
    m_editorTime = time;
    m_updateAnimation->setDirty(true);
}

void QQuick3DParticleSystem::componentComplete()
{
    QQuick3DNode::componentComplete();
    m_componentComplete = true;
    m_updateAnimation->start();

    connect(&m_loggingTimer, &QTimer::timeout, this, &QQuick3DParticleSystem::updateLoggingData);
    m_loggingTimer.setInterval(m_loggingData->m_loggingInterval);

    if (m_useRandomSeed)
        doSeedRandomization();
    else
        m_rand.init(m_seed);

    m_time = 0;
    m_currentTime = 0;
    m_editorTime = 0;

    Q_EMIT timeChanged();

    // Reset restarts the animation (if running)
    if (m_animation->state() == QAbstractAnimation::Running)
        m_animation->stop();
    if (m_running && !isEditorModeOn())
        m_animation->start();
    if (m_paused)
        m_animation->pause();

    m_initialized = true;
}

void QQuick3DParticleSystem::refresh()
{
    // If the system isn't running, force refreshing by calling update
    // with the current time. QAbstractAnimation::setCurrentTime() implementation
    // always calls updateCurrentTime() even if the time would remain the same.
    if (!m_running || m_paused || isEditorModeOn())
        m_animation->setCurrentTime(isEditorModeOn() ? m_editorTime : m_time);
}

void QQuick3DParticleSystem::markDirty()
{
    // Mark the system dirty so things are updated at the next frame.
    m_updateAnimation->setDirty(true);
}

int QQuick3DParticleSystem::particleCount() const
{
    int pCount = 0;
    for (auto particle : std::as_const(m_particles))
        pCount += particle->maxAmount();
    return pCount;
}

void QQuick3DParticleSystem::registerParticle(QQuick3DParticle *particle)
{
    m_particles << particle;
}

void QQuick3DParticleSystem::unRegisterParticle(QQuick3DParticle *particle)
{
    m_particles.removeAll(particle);
}

void QQuick3DParticleSystem::registerParticleEmitter(QQuick3DParticleEmitter *e)
{
    auto te = qobject_cast<QQuick3DParticleTrailEmitter *>(e);
    if (te)
        m_trailEmitters << te;
    else
        m_emitters << e;
}

void QQuick3DParticleSystem::unRegisterParticleEmitter(QQuick3DParticleEmitter *e)
{
    auto te = qobject_cast<QQuick3DParticleTrailEmitter *>(e);
    if (te)
        m_trailEmitters.removeAll(te);
    else
        m_emitters.removeAll(e);
}

void QQuick3DParticleSystem::registerParticleAffector(QQuick3DParticleAffector *a)
{
    m_affectors << a;
    m_connections.insert(a, connect(a, &QQuick3DParticleAffector::update, this, &QQuick3DParticleSystem::markDirty));
}

void QQuick3DParticleSystem::unRegisterParticleAffector(QQuick3DParticleAffector *a)
{
    QObject::disconnect(m_connections[a]);
    m_connections.remove(a);
    m_affectors.removeAll(a);
}

void QQuick3DParticleSystem::updateCurrentTime(int currentTime)
{
    if (!m_initialized || isGloballyDisabled() || (isEditorModeOn() && !visible()))
        return;

    Q_QUICK3D_PROFILE_START(QQuick3DProfiler::Quick3DParticleUpdate);

    Q_TRACE(QSSG_particleUpdate_entry);

    m_currentTime = currentTime;
    const float timeS = float(m_currentTime / 1000.0f);

    m_particlesMax = 0;
    m_particlesUsed = 0;
    m_updates++;

    m_perfTimer.start();

    // Emit new particles
    for (auto emitter : std::as_const(m_emitters))
        emitter->emitParticles();

    // Prepare Affectors
    for (auto affector : std::as_const(m_affectors)) {
        if (affector->m_enabled)
            affector->prepareToAffect();
    }

    // Animate current particles
    for (auto particle : std::as_const(m_particles)) {

        // Collect possible trail emits
        QVector<TrailEmits> trailEmits;
        for (auto emitter : std::as_const(m_trailEmitters)) {
            if (emitter->follow() == particle) {
                int emitAmount = emitter->getEmitAmount();
                if (emitAmount > 0 || emitter->hasBursts()) {
                    TrailEmits e;
                    e.emitter = emitter;
                    e.amount = emitAmount;
                    trailEmits << e;
                }
            }
        }

        m_particlesMax += particle->maxAmount();

        QQuick3DParticleSpriteParticle *spriteParticle = qobject_cast<QQuick3DParticleSpriteParticle *>(particle);
        if (spriteParticle) {
            processSpriteParticle(spriteParticle, trailEmits, timeS);
            continue;
        }
        QQuick3DParticleModelParticle *modelParticle = qobject_cast<QQuick3DParticleModelParticle *>(particle);
        if (modelParticle) {
            processModelParticle(modelParticle, trailEmits, timeS);
            continue;
        }
        QQuick3DParticleModelBlendParticle *mbp = qobject_cast<QQuick3DParticleModelBlendParticle *>(particle);
        if (mbp) {
            processModelBlendParticle(mbp, trailEmits, timeS);
            continue;
        }
    }

    // Clear bursts from trailemitters
    for (auto emitter : std::as_const(m_trailEmitters))
        emitter->clearBursts();

    m_timeAnimation += m_perfTimer.nsecsElapsed();
    m_updateAnimation->setDirty(false);
    Q_QUICK3D_PROFILE_END_WITH_ID(QQuick3DProfiler::Quick3DParticleUpdate, m_particlesUsed, Q_QUICK3D_PROFILE_GET_ID(this));

    Q_TRACE(QSSG_particleUpdate_exit, m_particlesUsed);

}

void QQuick3DParticleSystem::processModelParticle(QQuick3DParticleModelParticle *modelParticle, const QVector<TrailEmits> &trailEmits, float timeS)
{
    modelParticle->clearInstanceTable();

    const int c = modelParticle->maxAmount();

    for (int i = 0; i < c; i++) {
        const auto d = &modelParticle->m_particleData.at(i);

        const float particleTimeEnd = d->startTime + d->lifetime;

        if (timeS < d->startTime || timeS > particleTimeEnd) {
            if (timeS > particleTimeEnd && d->lifetime > 0.0f) {
                const auto pos = d->reversed ? d->startPosition : d->startPosition + (d->startVelocity * (particleTimeEnd - d->startTime));
                for (auto trailEmit : std::as_const(trailEmits))
                    trailEmit.emitter->emitTrailParticles(pos, 0, QQuick3DParticleDynamicBurst::TriggerEnd, d->surfaceNormal, d->startVelocity.normalized());
            }
            // Particle not alive currently
            continue;
        }

        QQuick3DParticleDataCurrent currentData;
        if (timeS >= d->startTime && d->lifetime <= 0.0f) {
            for (auto trailEmit : std::as_const(trailEmits))
                trailEmit.emitter->emitTrailParticles(d->startPosition, 0, QQuick3DParticleDynamicBurst::TriggerStart, d->surfaceNormal, d->startVelocity.normalized());
        }

        // Adjust time for reversed particles
        const float particleTimeS = d->reversed ? particleTimeEnd - timeS : timeS - d->startTime;

        // Process features shared for both model & sprite particles
        processParticleCommon(currentData, d, particleTimeS);

        // Add a base rotation if alignment requested
        if (modelParticle->m_alignMode != QQuick3DParticle::AlignNone)
            processParticleAlignment(currentData, modelParticle, d);

        // 0.0 -> 1.0 during the particle lifetime
        const float timeChange = std::max(0.0f, std::min(1.0f, particleTimeS / d->lifetime));

        // Scale from initial to endScale
        currentData.scale = modelParticle->m_initialScale * (d->endSize * timeChange + d->startSize * (1.0f - timeChange));

        // Fade in & out
        const float particleTimeLeftS = d->lifetime - particleTimeS;
        processParticleFadeInOut(currentData, modelParticle, particleTimeS, particleTimeLeftS);

        // Affectors
        for (auto affector : std::as_const(m_affectors)) {
            // If affector is set to affect only particular particles, check these are included
            if (affector->m_enabled && (affector->m_particles.isEmpty() || affector->m_particles.contains(modelParticle)))
                affector->affectParticle(*d, &currentData, particleTimeS);
        }

        // Emit new particles from trails
        for (auto trailEmit : std::as_const(trailEmits))
            trailEmit.emitter->emitTrailParticles(currentData.position, trailEmit.amount, QQuick3DParticleDynamicBurst::TriggerTime, d->surfaceNormal, d->startVelocity.normalized());

        const QColor color(currentData.color.r, currentData.color.g, currentData.color.b, currentData.color.a);
        // Set current particle properties
        modelParticle->addInstance(currentData.position, currentData.scale, currentData.rotation, color, timeChange);
    }
    modelParticle->commitInstance();
}

static QVector3D mix(const QVector3D &a, const QVector3D &b, float f)
{
    return (b - a) * f + a;
}

void QQuick3DParticleSystem::processModelBlendParticle(QQuick3DParticleModelBlendParticle *particle, const QVector<TrailEmits> &trailEmits, float timeS)
{
    const int c = particle->maxAmount();

    for (int i = 0; i < c; i++) {
        const auto d = &particle->m_particleData.at(i);

        const float particleTimeEnd = d->startTime + d->lifetime;

        if (timeS < d->startTime || timeS > particleTimeEnd) {
            if (timeS > particleTimeEnd && d->lifetime > 0.0f) {
                const auto pos = d->reversed ? d->startPosition : d->startPosition + (d->startVelocity * (particleTimeEnd - d->startTime));
                for (auto trailEmit : std::as_const(trailEmits))
                    trailEmit.emitter->emitTrailParticles(pos, 0, QQuick3DParticleDynamicBurst::TriggerEnd, d->surfaceNormal, d->startVelocity.normalized());
            }
            // Particle not alive currently
            float age = 0.0f;
            float size = 0.0f;
            QVector3D pos;
            QVector3D rot;
            QVector4D color(float(d->startColor.r)/ 255.0f,
                            float(d->startColor.g)/ 255.0f,
                            float(d->startColor.b)/ 255.0f,
                            float(d->startColor.a)/ 255.0f);
            if (d->startTime > 0.0f && timeS > particleTimeEnd
                    && (particle->modelBlendMode() == QQuick3DParticleModelBlendParticle::Construct ||
                        particle->modelBlendMode() == QQuick3DParticleModelBlendParticle::Transfer)) {
                age = 1.0f;
                size = 1.0f;
                pos = particle->particleEndPosition(i);
                rot = particle->particleEndRotation(i);
                if (particle->fadeOutEffect() == QQuick3DParticle::FadeOpacity)
                    color.setW(0.0f);
            } else if (particle->modelBlendMode() == QQuick3DParticleModelBlendParticle::Explode ||
                       particle->modelBlendMode() == QQuick3DParticleModelBlendParticle::Transfer) {
                age = 0.0f;
                size = 1.0f;
                pos = particle->particleCenter(i);
                if (particle->fadeInEffect() == QQuick3DParticle::FadeOpacity)
                    color.setW(0.0f);
            }
            particle->setParticleData(i, pos, rot, color, size, age);
            continue;
        }

        QQuick3DParticleDataCurrent currentData;
        if (timeS >= d->startTime && d->lifetime <= 0.0f) {
            for (auto trailEmit : std::as_const(trailEmits))
                trailEmit.emitter->emitTrailParticles(d->startPosition, 0, QQuick3DParticleDynamicBurst::TriggerStart, d->surfaceNormal, d->startVelocity.normalized());
        }

        // Adjust time for reversed particles
        const float particleTimeS = d->reversed ? particleTimeEnd - timeS : timeS - d->startTime;

        // Process features shared for both model & sprite particles
        processParticleCommon(currentData, d, particleTimeS);

        // 0.0 -> 1.0 during the particle lifetime
        const float timeChange = std::max(0.0f, std::min(1.0f, particleTimeS / d->lifetime));

        // Scale from initial to endScale
        const float scale = d->endSize * timeChange + d->startSize * (1.0f - timeChange);
        currentData.scale = QVector3D(scale, scale, scale);

        // Fade in & out
        const float particleTimeLeftS = d->lifetime - particleTimeS;
        processParticleFadeInOut(currentData, particle, particleTimeS, particleTimeLeftS);

        // Affectors
        for (auto affector : std::as_const(m_affectors)) {
            // If affector is set to affect only particular particles, check these are included
            if (affector->m_enabled && (affector->m_particles.isEmpty() || affector->m_particles.contains(particle)))
                affector->affectParticle(*d, &currentData, particleTimeS);
        }

        // Emit new particles from trails
        for (auto trailEmit : std::as_const(trailEmits))
            trailEmit.emitter->emitTrailParticles(currentData.position, trailEmit.amount, QQuick3DParticleDynamicBurst::TriggerTime, d->surfaceNormal, d->startVelocity.normalized());

        // Set current particle properties
        const QVector4D color(float(currentData.color.r) / 255.0f,
                              float(currentData.color.g) / 255.0f,
                              float(currentData.color.b) / 255.0f,
                              float(currentData.color.a) / 255.0f);
        float endTimeS = particle->endTime() * 0.001f;
        if ((particle->modelBlendMode() == QQuick3DParticleModelBlendParticle::Construct ||
             particle->modelBlendMode() == QQuick3DParticleModelBlendParticle::Transfer)
                && particleTimeLeftS < endTimeS) {
            QVector3D endPosition = particle->particleEndPosition(i);
            QVector3D endRotation = particle->particleEndRotation(i);
            float factor = 1.0f - particleTimeLeftS / endTimeS;
            currentData.position = mix(currentData.position, endPosition, factor);
            currentData.rotation = mix(currentData.rotation, endRotation, factor);
        }
        particle->setParticleData(i, currentData.position, currentData.rotation,
                                  color, currentData.scale.x(), timeChange);
    }
    particle->commitParticles();
}

void QQuick3DParticleSystem::processSpriteParticle(QQuick3DParticleSpriteParticle *spriteParticle, const QVector<TrailEmits> &trailEmits, float timeS)
{
    const int c = spriteParticle->maxAmount();

    for (int i = 0; i < c; i++) {
        const auto d = &spriteParticle->m_particleData.at(i);

        const float particleTimeEnd = d->startTime + d->lifetime;
        auto &particleData = spriteParticle->m_spriteParticleData[i];
        if (timeS < d->startTime || timeS > particleTimeEnd) {
            if (timeS > particleTimeEnd && particleData.age > 0.0f) {
                const auto pos = d->reversed ? d->startPosition : d->startPosition + (d->startVelocity * (particleTimeEnd - d->startTime));
                for (auto trailEmit : std::as_const(trailEmits))
                    trailEmit.emitter->emitTrailParticles(pos, 0, QQuick3DParticleDynamicBurst::TriggerEnd, d->surfaceNormal, d->startVelocity.normalized());
                auto *lineParticle = qobject_cast<QQuick3DParticleLineParticle *>(spriteParticle);
                if (lineParticle)
                    lineParticle->saveLineSegment(i, timeS);
            }
            // Particle not alive currently
            spriteParticle->resetParticleData(i);
            continue;
        }

        QQuick3DParticleDataCurrent currentData;
        if (timeS >= d->startTime && timeS < particleTimeEnd && particleData.age == 0.0f) {
            for (auto trailEmit : std::as_const(trailEmits))
                trailEmit.emitter->emitTrailParticles(d->startPosition, 0, QQuick3DParticleDynamicBurst::TriggerStart, d->surfaceNormal, d->startVelocity.normalized());
        }

        // Adjust time for reversed particles
        const float particleTimeS = d->reversed ? particleTimeEnd - timeS : timeS - d->startTime;

        // Process features shared for both model & sprite particles
        processParticleCommon(currentData, d, particleTimeS);

        // Add a base rotation if alignment requested
        if (!spriteParticle->m_billboard && spriteParticle->m_alignMode != QQuick3DParticle::AlignNone)
            processParticleAlignment(currentData, spriteParticle, d);

        // 0.0 -> 1.0 during the particle lifetime
        const float timeChange = std::max(0.0f, std::min(1.0f, particleTimeS / d->lifetime));

        // Scale from initial to endScale
        const float scale = d->endSize * timeChange + d->startSize * (1.0f - timeChange);
        currentData.scale = QVector3D(scale, scale, scale);

        // Fade in & out
        const float particleTimeLeftS = d->lifetime - particleTimeS;
        processParticleFadeInOut(currentData, spriteParticle, particleTimeS, particleTimeLeftS);

        float animationFrame = 0.0f;
        if (auto sequence = spriteParticle->m_spriteSequence) {
            // animationFrame range is [0..1) where 0.0 is the beginning of the first frame
            // and 0.9999 is the end of the last frame.
            const bool isSingleFrame = (sequence->animationDirection() == QQuick3DParticleSpriteSequence::SingleFrame);
            float startFrame = sequence->firstFrame(d->index, isSingleFrame);
            if (sequence->animationDirection() == QQuick3DParticleSpriteSequence::Normal) {
                animationFrame = fmodf(startFrame + particleTimeS / d->animationTime, 1.0f);
            } else if (sequence->animationDirection() == QQuick3DParticleSpriteSequence::Reverse) {
                animationFrame = fmodf(startFrame + 0.9999f - fmodf(particleTimeS / d->animationTime, 1.0f), 1.0f);
            } else if (sequence->animationDirection() == QQuick3DParticleSpriteSequence::Alternate) {
                animationFrame = startFrame + particleTimeS / d->animationTime;
                animationFrame = fabsf(fmodf(1.0f + animationFrame, 2.0f) - 1.0f);
            } else if (sequence->animationDirection() == QQuick3DParticleSpriteSequence::AlternateReverse) {
                animationFrame = fmodf(startFrame + 0.9999f, 1.0f) - particleTimeS / d->animationTime;
                animationFrame = fabsf(fmodf(fabsf(1.0f + animationFrame), 2.0f) - 1.0f);
            } else {
                // SingleFrame
                animationFrame = startFrame;
            }
            animationFrame = std::clamp(animationFrame, 0.0f, 0.9999f);
        }

        // Affectors
        for (auto affector : std::as_const(m_affectors)) {
            // If affector is set to affect only particular particles, check these are included
            if (affector->m_enabled && (affector->m_particles.isEmpty() || affector->m_particles.contains(spriteParticle)))
                affector->affectParticle(*d, &currentData, particleTimeS);
        }

        // Emit new particles from trails
        for (auto trailEmit : std::as_const(trailEmits))
            trailEmit.emitter->emitTrailParticles(currentData.position, trailEmit.amount, QQuick3DParticleDynamicBurst::TriggerTime, d->surfaceNormal, d->startVelocity.normalized());


        // Set current particle properties
        const QVector4D color(float(currentData.color.r) / 255.0f,
                              float(currentData.color.g) / 255.0f,
                              float(currentData.color.b) / 255.0f,
                              float(currentData.color.a) / 255.0f);
        const QVector3D offset(spriteParticle->offsetX(), spriteParticle->offsetY(), 0);
        spriteParticle->setParticleData(i, currentData.position + (offset * currentData.scale.x()),
                                        currentData.rotation, color, currentData.scale.x(), timeChange,
                                        animationFrame);
    }
    spriteParticle->commitParticles(timeS);
}

void QQuick3DParticleSystem::processParticleCommon(QQuick3DParticleDataCurrent &currentData, const QQuick3DParticleData *d, float particleTimeS)
{
    m_particlesUsed++;

    currentData.position = d->startPosition;

    // Initial color from start color
    currentData.color = d->startColor;

    // Initial position from start velocity
    currentData.position += d->startVelocity * particleTimeS;

    // Initial rotation from start velocity
    constexpr float step = 360.0f / 127.0f;
    currentData.rotation = QVector3D(
                d->startRotation.x * step + abs(d->startRotationVelocity.x) * d->startRotationVelocity.x * particleTimeS,
                d->startRotation.y * step + abs(d->startRotationVelocity.y) * d->startRotationVelocity.y * particleTimeS,
                d->startRotation.z * step + abs(d->startRotationVelocity.z) * d->startRotationVelocity.z * particleTimeS);
}

void QQuick3DParticleSystem::processParticleFadeInOut(QQuick3DParticleDataCurrent &currentData, const QQuick3DParticle *particle, float particleTimeS, float particleTimeLeftS)
{
    const float fadeInS = particle->m_fadeInDuration / 1000.0f;
    const float fadeOutS = particle->m_fadeOutDuration / 1000.0f;
    if (particleTimeS < fadeInS) {
        // 0.0 -> 1.0 during the particle fadein
        const float fadeIn = particleTimeS / fadeInS;
        if (particle->m_fadeInEffect == QQuick3DParticleModelParticle::FadeOpacity)
            currentData.color.a *= fadeIn;
        else if (particle->m_fadeInEffect == QQuick3DParticleModelParticle::FadeScale)
            currentData.scale *= fadeIn;
    }
    if (particleTimeLeftS < fadeOutS) {
        // 1.0 -> 0.0 during the particle fadeout
        const float fadeOut = particleTimeLeftS / fadeOutS;
        if (particle->m_fadeOutEffect == QQuick3DParticleModelParticle::FadeOpacity)
            currentData.color.a *= fadeOut;
        else if (particle->m_fadeOutEffect == QQuick3DParticleModelParticle::FadeScale)
            currentData.scale *= fadeOut;
    }
}

void QQuick3DParticleSystem::processParticleAlignment(QQuick3DParticleDataCurrent &currentData, const QQuick3DParticle *particle, const QQuick3DParticleData *d)
{
    if (particle->m_alignMode == QQuick3DParticle::AlignTowardsTarget) {
        QQuaternion alignQuat = QQuick3DQuaternionUtils::lookAt(particle->alignTargetPosition(), currentData.position);
        currentData.rotation = (alignQuat * QQuaternion::fromEulerAngles(currentData.rotation)).toEulerAngles();
    } else if (particle->m_alignMode == QQuick3DParticle::AlignTowardsStartVelocity) {
        QQuaternion alignQuat = QQuick3DQuaternionUtils::lookAt(d->startVelocity, QVector3D());
        currentData.rotation = (alignQuat * QQuaternion::fromEulerAngles(currentData.rotation)).toEulerAngles();
    }
}

bool QQuick3DParticleSystem::isGloballyDisabled()
{
    static const bool disabled = qEnvironmentVariableIntValue("QT_QUICK3D_DISABLE_PARTICLE_SYSTEMS");
    return disabled;
}

bool QQuick3DParticleSystem::isEditorModeOn()
{
    static const bool editorMode = qEnvironmentVariableIntValue("QT_QUICK3D_EDITOR_PARTICLE_SYSTEMS");
    return editorMode;
}

void QQuick3DParticleSystem::updateLoggingData()
{
    if (m_updates == 0)
        return;

    if (m_loggingData->m_particlesMax != m_particlesMax) {
        m_loggingData->m_particlesMax = m_particlesMax;
        Q_EMIT m_loggingData->particlesMaxChanged();
    }
    if (m_loggingData->m_particlesUsed != m_particlesUsed) {
        m_loggingData->m_particlesUsed = m_particlesUsed;
        Q_EMIT m_loggingData->particlesUsedChanged();
    }
    if (m_loggingData->m_updates != m_updates) {
        m_loggingData->m_updates = m_updates;
        Q_EMIT m_loggingData->updatesChanged();
    }

    m_loggingData->updateTimes(m_timeAnimation);

    Q_EMIT loggingDataChanged();
    resetLoggingVariables();
}

void QQuick3DParticleSystem::resetLoggingVariables()
{
    m_particlesMax = 0;
    m_particlesUsed = 0;
    m_updates = 0;
    m_timeAnimation = 0;
}

QPRand *QQuick3DParticleSystem::rand()
{
    return &m_rand;
}

void QQuick3DParticleSystem::doSeedRandomization()
{
    // Random 1..INT32_MAX, making sure seed changes from the initial 0.
    setSeed(QRandomGenerator::global()->bounded(1 + (INT32_MAX - 1)));
}

bool QQuick3DParticleSystem::isShared(const QQuick3DParticle *particle) const
{
    int count = 0;
    for (auto emitter : std::as_const(m_emitters)) {
        count += emitter->particle() == particle;
        if (count > 1)
            return true;
    }
    for (auto emitter : std::as_const(m_trailEmitters)) {
        count += emitter->particle() == particle;
        if (count > 1)
            return true;
    }
    return false;
}

QT_END_NAMESPACE
