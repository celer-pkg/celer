// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticleemitter_p.h"
#include "qquick3dparticlemodelparticle_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticleutils_p.h"
#include "qquick3dparticlespritesequence_p.h"
#include "qquick3dparticlemodelblendparticle_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ParticleEmitter3D
    \inherits Node
    \inqmlmodule QtQuick3D.Particles3D
    \brief Emitter for logical particles.
    \since 6.2

    This element emits logical particles into the \l ParticleSystem3D, with the given starting attributes.

    At least one emitter is required to have particles in the \l ParticleSystem3D. There are a few different
    ways to control the emitting amount:
    \list
        \li Set the \l emitRate which controls how many particles per second get emitted continuously.
        \li Add \l EmitBurst3D elements into emitBursts property to emit bursts declaratively.
        \li Call any of the \l burst() methods to emit bursts immediately.
    \endlist
*/

template <typename T,
         typename std::enable_if_t<std::is_signed_v<T>, bool> = true,
         typename std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
static constexpr T qSign(T val)
{
    return val < T(0) ? T(-1) : T(1);
}

QQuick3DParticleEmitter::QQuick3DParticleEmitter(QQuick3DNode *parent)
    : QQuick3DNode(parent)
{
}

QQuick3DParticleEmitter::~QQuick3DParticleEmitter()
{
    qDeleteAll(m_emitBursts);
    m_emitBursts.clear();
    if (m_system)
        m_system->unRegisterParticleEmitter(this);
}

/*!
    \qmlproperty bool ParticleEmitter3D::enabled

    If enabled is set to \c false, this emitter will not emit any particles.
    Usually this is used to conditionally turn an emitter on or off.
    If you want to continue emitting burst, keep \l emitRate at 0 instead of
    toggling this to \c false.

    The default value is \c true.
*/
bool QQuick3DParticleEmitter::enabled() const
{
    return m_enabled;
}
void QQuick3DParticleEmitter::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    if (enabled && m_system) {
        // When enabling, we need to reset the
        // previous emit time as it might be a long time ago.
        m_prevEmitTime = m_system->currentTime();
        m_prevBurstTime = m_prevEmitTime;
    }

    m_enabled = enabled;
    Q_EMIT enabledChanged();
}

/*!
    \qmlproperty Direction3D ParticleEmitter3D::velocity

    This property can be used to set a starting velocity for emitted particles.
    If velocity is not set, particles start motionless and velocity comes from
    \l {Affector3D}{affectors} if they are used.
*/
QQuick3DParticleDirection *QQuick3DParticleEmitter::velocity() const
{
    return m_velocity;
}

void QQuick3DParticleEmitter::setVelocity(QQuick3DParticleDirection *velocity)
{
    if (m_velocity == velocity)
        return;

    m_velocity = velocity;
    if (m_velocity && m_system)
        m_velocity->m_system = m_system;

    Q_EMIT velocityChanged();
}

/*!
    \qmlproperty ParticleSystem3D ParticleEmitter3D::system

    This property defines the \l ParticleSystem3D for the emitter. If system is direct parent of the emitter,
    this property does not need to be defined.
*/
QQuick3DParticleSystem *QQuick3DParticleEmitter::system() const
{
    return m_system;
}

void QQuick3DParticleEmitter::setSystem(QQuick3DParticleSystem *system)
{
    if (m_system == system)
        return;

    if (m_system)
        m_system->unRegisterParticleEmitter(this);

    m_system = system;
    if (m_system) {
        m_system->registerParticleEmitter(this);
        // Reset prev emit time to time of the new system
        m_prevEmitTime = m_system->currentTime();
        m_prevBurstTime = m_prevEmitTime;
    }

    if (m_particle)
        m_particle->setSystem(m_system);

    if (m_shape)
        m_shape->m_system = m_system;

    if (m_velocity)
        m_velocity->m_system = m_system;

    m_systemSharedParent = getSharedParentNode(this, m_system);

    Q_EMIT systemChanged();
}

/*!
    \qmlproperty real ParticleEmitter3D::emitRate

    This property defines the constant emitting rate in particles per second.
    For example, if the emitRate is 120 and system animates at 60 frames per
    second, 2 new particles are emitted at every frame.

    The default value is \c 0.
*/
float QQuick3DParticleEmitter::emitRate() const
{
    return m_emitRate;
}

void QQuick3DParticleEmitter::setEmitRate(float emitRate)
{
    if (qFuzzyCompare(m_emitRate, emitRate))
        return;

    if (m_emitRate == 0 && m_system) {
        // When changing emit rate from 0 we need to reset
        // previous emit time as it may be long time ago
        m_prevEmitTime = m_system->currentTime();
    }
    m_emitRate = emitRate;
    Q_EMIT emitRateChanged();
}


/*!
    \qmlproperty real ParticleEmitter3D::particleScale

    This property defines the scale multiplier of the particles at the beginning.
    To have variation in the particle sizes, use \l particleScaleVariation.

    The default value is \c 1.0.

    \sa particleEndScale, particleScaleVariation
*/
float QQuick3DParticleEmitter::particleScale() const
{
    return m_particleScale;
}

void QQuick3DParticleEmitter::setParticleScale(float particleScale)
{
    if (qFuzzyCompare(m_particleScale, particleScale))
        return;

    m_particleScale = particleScale;
    Q_EMIT particleScaleChanged();
}

/*!
    \qmlproperty real ParticleEmitter3D::particleEndScale

    This property defines the scale multiplier of the particles at the end
    of particle \l lifeSpan. To have variation in the particle end sizes, use
    \l particleEndScaleVariation. When the value is negative, end scale is the
    same as the \l particleScale, so scale doesn't change during the particle
    \l lifeSpan.

    The default value is \c -1.0.

    \sa particleScale, particleScaleVariation
*/
float QQuick3DParticleEmitter::particleEndScale() const
{
    return m_particleEndScale;
}

void QQuick3DParticleEmitter::setParticleEndScale(float particleEndScale)
{
    if (qFuzzyCompare(m_particleEndScale, particleEndScale))
        return;

    m_particleEndScale = particleEndScale;
    Q_EMIT particleEndScaleChanged();
}

/*!
    \qmlproperty real ParticleEmitter3D::particleScaleVariation

    This property defines the scale variation of the particles. For example, to
    emit particles at scale 0.5 - 1.5:

    \qml
    ParticleEmitter3D {
        ...
        particleScale: 1.0
        particleScaleVariation: 0.5
    }
    \endqml

    The default value is \c 0.0.

    \sa particleScale, particleEndScaleVariation
*/
float QQuick3DParticleEmitter::particleScaleVariation() const
{
    return m_particleScaleVariation;
}

void QQuick3DParticleEmitter::setParticleScaleVariation(float particleScaleVariation)
{
    if (qFuzzyCompare(m_particleScaleVariation, particleScaleVariation))
        return;

    m_particleScaleVariation = particleScaleVariation;
    Q_EMIT particleScaleVariationChanged();
}

/*!
    \qmlproperty real ParticleEmitter3D::particleEndScaleVariation

    This property defines the scale variation of the particles in the end.
    When the value is negative, \l particleScaleVariation is used also for the
    end scale. For example, to emit particles which start at scale 0.5 - 1.5 and end
    at scale 1.0 - 5.0:

    \qml
    ParticleEmitter3D {
        ...
        particleScale: 1.0
        particleScaleVariation: 0.5
        particleEndScale: 3.0
        particleEndScaleVariation: 2.0
    }
    \endqml

    The default value is \c -1.0.

    \sa particleEndScale
*/
float QQuick3DParticleEmitter::particleEndScaleVariation() const
{
    return m_particleEndScaleVariation;
}

void QQuick3DParticleEmitter::setParticleEndScaleVariation(float particleEndScaleVariation)
{
    if (qFuzzyCompare(m_particleEndScaleVariation, particleEndScaleVariation))
        return;

    m_particleEndScaleVariation = particleEndScaleVariation;
    Q_EMIT particleEndScaleVariationChanged();
}

/*!
    \qmlproperty int ParticleEmitter3D::lifeSpan

    This property defines the lifespan of a single particle in milliseconds.

    The default value is \c 1000.

    \sa lifeSpanVariation
*/
int QQuick3DParticleEmitter::lifeSpan() const
{
    return m_lifeSpan;
}

void QQuick3DParticleEmitter::setLifeSpan(int lifeSpan)
{
    if (m_lifeSpan == lifeSpan)
        return;

    m_lifeSpan = lifeSpan;
    Q_EMIT lifeSpanChanged();
}

/*!
    \qmlproperty int ParticleEmitter3D::lifeSpanVariation

    This property defines the lifespan variation of a single particle in milliseconds.

    For example, to emit particles which will exist between 3 and 4 seconds:

    \qml
    ParticleEmitter3D {
        ...
        lifeSpan: 3500
        lifeSpanVariation: 500
    }
    \endqml

    The default value is \c 0.

    \sa lifeSpan
*/
int QQuick3DParticleEmitter::lifeSpanVariation() const
{
    return m_lifeSpanVariation;
}

void QQuick3DParticleEmitter::setLifeSpanVariation(int lifeSpanVariation)
{
    if (m_lifeSpanVariation == lifeSpanVariation)
        return;

    m_lifeSpanVariation = lifeSpanVariation;
    Q_EMIT lifeSpanVariationChanged();
}

/*!
    \qmlproperty Particle3D ParticleEmitter3D::particle

    This property defines the logical particle which this emitter emits.
    Emitter must have a particle defined, or it doesn't emit anything.
    Particle can be either \l SpriteParticle3D or \l ModelParticle3D.
*/
QQuick3DParticle *QQuick3DParticleEmitter::particle() const
{
    return m_particle;
}

void QQuick3DParticleEmitter::setParticle(QQuick3DParticle *particle)
{
    if (m_particle == particle)
        return;
    if (particle && particle->system() != nullptr && m_system && particle->system() != m_system) {
        qWarning("ParticleEmitter3D: Emitter and Particle must be in the same system.");
        return;
    }

    if (m_particle && m_system && !m_system->isShared(m_particle))
        m_particle->setSystem(nullptr);
    m_particle = particle;
    if (particle) {
        particle->setDepthBias(m_depthBias);
        particle->setSystem(system());
        QObject::connect(this, &QQuick3DParticleEmitter::depthBiasChanged, m_particle, [this]() {
            m_particle->setDepthBias(m_depthBias);
        });
    }
    Q_EMIT particleChanged();
}

/*!
    \qmlproperty ParticleAbstractShape3D ParticleEmitter3D::shape

    This property defines optional shape for the emitting area. It can be either
    \l ParticleShape3D or \l ParticleModelShape3D. Shape is scaled,
    positioned and rotated based on the emitter node properties. When the Shape
    \l {ParticleShape3D::fill}{fill} property is set to false, emitting happens
    only from the surface of the shape.

    When the shape is not defined, emitting is done from the center point of the
    emitter node.
*/
QQuick3DParticleAbstractShape *QQuick3DParticleEmitter::shape() const
{
    return m_shape;
}

void QQuick3DParticleEmitter::setShape(QQuick3DParticleAbstractShape *shape)
{
    if (m_shape == shape)
        return;

    m_shape = shape;
    if (m_shape && m_system)
        m_shape->m_system = m_system;
    Q_EMIT shapeChanged();
}

/*!
    \qmlproperty vector3d ParticleEmitter3D::particleRotation

    This property defines the rotation of the particles in the beginning.
    Rotation is defined as degrees in euler angles.

    \sa particleRotationVariation
*/
QVector3D QQuick3DParticleEmitter::particleRotation() const
{
    return m_particleRotation;
}

void QQuick3DParticleEmitter::setParticleRotation(const QVector3D &particleRotation)
{
    if (m_particleRotation == particleRotation)
        return;

    m_particleRotation = particleRotation;
    Q_EMIT particleRotationChanged();
}

/*!
    \qmlproperty vector3d ParticleEmitter3D::particleRotationVariation

    This property defines the rotation variation of the particles in the beginning.
    Rotation variation is defined as degrees in euler angles.

    For example, to emit particles in fully random rotations:

    \qml
    ParticleEmitter3D {
        ...
        particleRotationVariation: Qt.vector3d(180, 180, 180)
    }
    \endqml

    \sa particleRotation
*/
QVector3D QQuick3DParticleEmitter::particleRotationVariation() const
{
    return m_particleRotationVariation;
}

void QQuick3DParticleEmitter::setParticleRotationVariation(const QVector3D &particleRotationVariation)
{
    if (m_particleRotationVariation == particleRotationVariation)
        return;

    m_particleRotationVariation = particleRotationVariation;
    Q_EMIT particleRotationVariationChanged();
}

/*!
    \qmlproperty vector3d ParticleEmitter3D::particleRotationVelocity

    This property defines the rotation velocity of the particles in the beginning.
    Rotation velocity is defined as degrees per second in euler angles.

    \sa particleRotationVelocityVariation
*/
QVector3D QQuick3DParticleEmitter::particleRotationVelocity() const
{
    return m_particleRotationVelocity;
}

void QQuick3DParticleEmitter::setParticleRotationVelocity(const QVector3D &particleRotationVelocity)
{
    if (m_particleRotationVelocity == particleRotationVelocity)
        return;

    m_particleRotationVelocity = particleRotationVelocity;
    Q_EMIT particleRotationVelocityChanged();
}

/*!
    \qmlproperty vector3d ParticleEmitter3D::particleRotationVelocityVariation

    This property defines the rotation velocity variation of the particles.
    Rotation velocity variation is defined as degrees per second in euler angles.

    For example, to emit particles in random rotations which have random rotation
    velocity between -100 and 100 degrees per second into any directions:

    \qml
    ParticleEmitter3D {
        ...
        particleRotationVariation: Qt.vector3d(180, 180, 180)
        particleRotationVelocityVariation: Qt.vector3d(100, 100, 100)
    }
    \endqml

    \sa particleRotationVelocity
*/
QVector3D QQuick3DParticleEmitter::particleRotationVelocityVariation() const
{
    return m_particleRotationVelocityVariation;
}

void QQuick3DParticleEmitter::setParticleRotationVelocityVariation(const QVector3D &particleRotationVelocityVariation)
{
    if (m_particleRotationVelocityVariation == particleRotationVelocityVariation)
        return;

    m_particleRotationVelocityVariation = particleRotationVelocityVariation;
    Q_EMIT particleRotationVariationVelocityChanged();
}

/*!
    \qmlproperty real ParticleEmitter3D::depthBias

    Holds the depth bias of the emitter. Depth bias is added to the object distance from camera when sorting
    objects. This can be used to force rendering order between objects close to each other, that
    might otherwise be rendered in different order in different frames. Negative values cause the
    sorting value to move closer to the camera while positive values move it further from the camera.
*/
float QQuick3DParticleEmitter::depthBias() const
{
    return m_depthBias;
}

void QQuick3DParticleEmitter::setDepthBias(float bias)
{
    if (qFuzzyCompare(bias, m_depthBias))
        return;

    m_depthBias = bias;
    emit depthBiasChanged();
}

/*!
    \qmlproperty bool ParticleEmitter3D::reversed
    \since 6.10

    This property reverses the particle time i.e. emitted particles are run from
    the end time to the start time.
    \default false
*/
bool QQuick3DParticleEmitter::reversed() const
{
    return m_reversed;
}

void QQuick3DParticleEmitter::setReversed(bool reversed)
{
    if (m_reversed == reversed)
        return;
    m_reversed = reversed;
    emit reversedChanged();
}

/*!
    \qmlproperty EmitMode ParticleEmitter3D::emitMode
    \since 6.10

    This property defines the emit mode of the emitter with shape.

    \default ParticleEmitter3D.Default
*/

/*!
    \qmlproperty enumeration ParticleEmitter3D::EmitMode
    \since 6.10

    Defines the emit mode of the emitter with shape.

    \value ParticleEmitter3D.Default
        Default emit behavior.
    \value ParticleEmitter3D.SurfaceNormal
        The particles are emitted along the surface normal. Requires a particle shape.
        If the emitter is a trail emitter, the surface normal is inherited from the followed particle.
    \value ParticleEmitter3D.SurfaceReflected
        The particles are emitted along the reflection of the velocity vector and the surface normal.
        If the emitter is a trail emitter, the surface normal and the velocity are inherited from the followed particle.
        Requires particle shape.
*/

QQuick3DParticleEmitter::EmitMode QQuick3DParticleEmitter::emitMode() const
{
    return m_emitMode;
}

void QQuick3DParticleEmitter::setEmitMode(EmitMode mode)
{
    if (m_emitMode == mode)
        return;
    m_emitMode = mode;
    emit emitModeChanged();
}

// Called to reset when system stop/continue
void QQuick3DParticleEmitter::reset()
{
    m_prevEmitTime = 0;
    m_unemittedF = 0.0f;
    m_prevBurstTime = 0;
    m_burstEmitData.clear();
}

/*!
    \qmlmethod vector3d ParticleEmitter3D::burst(int count)

    This method emits \a count amount of particles from this emitter immediately.
*/
void QQuick3DParticleEmitter::burst(int count)
{
    burst(count, 0, QVector3D());
}

/*!
    \qmlmethod vector3d ParticleEmitter3D::burst(int count, int duration)

    This method emits \a count amount of particles from this emitter during the
    next \a duration milliseconds.
*/
void QQuick3DParticleEmitter::burst(int count, int duration)
{
    burst(count, duration, QVector3D());
}

/*!
    \qmlmethod vector3d ParticleEmitter3D::burst(int count, int duration, vector3d position)

    This method emits \a count amount of particles from this emitter during the
    next \a duration milliseconds. The particles are emitted as if the emitter was
    at \a position but all other properties are the same.
*/
void QQuick3DParticleEmitter::burst(int count, int duration, const QVector3D &position)
{
    if (!m_system)
        return;
    QQuick3DParticleEmitBurstData burst;
    burst.time = m_system->currentTime();
    burst.amount = count;
    burst.duration = duration;
    burst.position = position;
    emitParticlesBurst(burst);
}

void QQuick3DParticleEmitter::generateEmitBursts()
{
    if (!m_system)
        return;

    if (!m_particle)
        return;

    if (m_emitBursts.isEmpty()) {
        m_burstGenerated = true;
        return;
    }

    // Generating burst causes all particle data reseting
    // as bursts take first particles in the list.
    m_particle->reset();

    // TODO: In trail emitter case centerPos should be calculated
    // taking into account each particle position at emitburst time
    QMatrix4x4 transform = calculateParticleTransform(parentNode(), m_systemSharedParent);
    QQuaternion rotation = calculateParticleRotation(parentNode(), m_systemSharedParent);
    QVector3D centerPos = position();

    for (auto emitBurst : std::as_const(m_emitBursts)) {
        // Ignore all dynamic bursts here
        if (qobject_cast<QQuick3DParticleDynamicBurst *>(emitBurst))
            continue;
        int emitAmount = emitBurst->amount();
        if (emitAmount <= 0)
            return;
        // Distribute start times between burst time and time+duration.
        float startTime = float(emitBurst->time() / 1000.0f);
        float timeStep = float(emitBurst->duration() / 1000.0f) / emitAmount;
        for (int i = 0; i < emitAmount; i++) {
            emitParticle(m_particle, startTime, transform, rotation, centerPos);
            startTime += timeStep;
        }
        // Increase burst index (for statically allocated particles)
        m_particle->updateBurstIndex(emitBurst->amount());
    }
    m_burstGenerated = true;
}

void QQuick3DParticleEmitter::registerEmitBurst(QQuick3DParticleEmitBurst* emitBurst)
{
    m_emitBursts.removeAll(emitBurst);
    m_emitBursts << emitBurst;
    m_burstGenerated = false;
}

void QQuick3DParticleEmitter::unRegisterEmitBurst(QQuick3DParticleEmitBurst* emitBurst)
{
    m_emitBursts.removeAll(emitBurst);
    m_burstGenerated = false;
}

static QMatrix4x4 rotationFromNormal(const QVector3D &n)
{
    static QMatrix4x4 s_cached;
    static QVector3D s_cachedN;
    if (qFuzzyCompare(s_cachedN.x(), n.x()) && qFuzzyCompare(s_cachedN.y(), n.y()) && qFuzzyCompare(s_cachedN.z(), n.z()))
        return s_cached;
    QVector3D b;
    QVector3D t;
    float values[16];

    if (qAbs(n.y()) < 1.0f - 0.0001f) {
        t = QVector3D::crossProduct(n, QVector3D(0, qSign(n.y()), 0));
        b = QVector3D::crossProduct(n, t);
    } else {
        b = QVector3D(1, 0, 0);
        t = QVector3D(0, 0, 1);
    }
    values[0] = b.x();
    values[1] = b.y();
    values[2] = b.z();
    values[3] = 0.0;
    values[4] = t.x();
    values[5] = t.y();
    values[6] = t.z();
    values[7] = 0.0;
    values[8] = n.x();
    values[9] = n.y();
    values[10] = n.z();
    values[11] = 0.0;
    values[12] = 0.0;
    values[13] = 0.0;
    values[14] = 0.0;
    values[15] = 1.0;
    QMatrix4x4 ret = QMatrix4x4(values);
    s_cached = ret.transposed();
    s_cachedN = n;
    return s_cached;
}

static QVector3D reflect(const QVector3D &I, QVector3D &N)
{
    return I - 2.0 * QVector3D::dotProduct(N, I) * N;
}

void QQuick3DParticleEmitter::emitParticle(QQuick3DParticle *particle, float startTime, const QMatrix4x4 &transform,
                                           const QQuaternion &parentRotation, const QVector3D &centerPos, int index,
                                           const QVector3D &velocity, const QVector3D &normal)
{
    if (!m_system)
        return;
    auto rand = m_system->rand();

    QQuick3DParticleModelBlendParticle *mbp = qobject_cast<QQuick3DParticleModelBlendParticle *>(particle);
    if (mbp && mbp->lastParticle())
        return;

    int particleDataIndex = index == -1 ? particle->nextCurrentIndex(this) : index;
    if (index == -1 && mbp && mbp->emitMode() == QQuick3DParticleModelBlendParticle::Random)
        particleDataIndex = mbp->randomIndex(particleDataIndex);

    auto d = &particle->m_particleData[particleDataIndex];
    int particleIdIndex = m_system->m_particleIdIndex++;
    if (m_system->m_particleIdIndex == INT_MAX)
        m_system->m_particleIdIndex = 0;

    *d = m_clearData; // Reset the data as it might be reused
    d->index = particleIdIndex;
    d->startTime = startTime;
    d->surfaceNormal = normal;

    // Life time in seconds
    float lifeSpanMs = m_lifeSpanVariation / 1000.0f;
    float lifeSpanVariationMs = lifeSpanMs - 2.0f * rand->get(particleIdIndex, QPRand::LifeSpanV) * lifeSpanMs;
    d->lifetime = (m_lifeSpan / 1000.0f) + lifeSpanVariationMs;

    // Size
    float sVar = m_particleScaleVariation - 2.0f * rand->get(particleIdIndex, QPRand::ScaleV) * m_particleScaleVariation;
    float endScale = (m_particleEndScale < 0.0f) ? m_particleScale : m_particleEndScale;
    float sEndVar = (m_particleEndScaleVariation < 0.0f)
            ? sVar
            : m_particleEndScaleVariation - 2.0f * rand->get(particleIdIndex, QPRand::ScaleEV) * m_particleEndScaleVariation;
    d->startSize = std::max(0.0f, float(m_particleScale + sVar));
    d->endSize = std::max(0.0f, float(endScale + sEndVar));

    // Emiting area/shape
    bool normalBasedVelocity = false;
    if (mbp && mbp->modelBlendMode() != QQuick3DParticleModelBlendParticle::Construct) {
        // We emit from model position unless in construct mode
        d->startPosition = mbp->particleCenter(particleDataIndex);
    } else {
        // When shape is not set, default to node center point.
        QVector3D pos = centerPos;
        if (m_shape) {
            pos += m_shape->getPosition(particleIdIndex);
            const auto n = m_shape->getSurfaceNormal(particleIdIndex);
            if (!n.isNull()) {
                d->surfaceNormal = n;
                if (m_emitMode != EmitMode::Default)
                    normalBasedVelocity = true;
            }
        } else if (!normal.isNull() && m_emitMode != EmitMode::Default) {
            normalBasedVelocity = true;
        }
        d->startPosition = transform.map(pos);
    }

    // Velocity
    // Rotate velocity based on parent node rotation and emitter rotation
    if (m_velocity)
        d->startVelocity = parentRotation * rotation() * m_velocity->sample(*d);
    if (normalBasedVelocity) {
        if (m_emitMode == EmitMode::SurfaceNormal)
            d->startVelocity = rotationFromNormal(d->surfaceNormal).mapVector(d->startVelocity);
        else if (!velocity.isNull()) // EmitMode::SurfaceReflected
            d->startVelocity = rotationFromNormal(-reflect(velocity, d->surfaceNormal)).mapVector(d->startVelocity);
        else
            d->startVelocity = rotationFromNormal(-reflect(d->startVelocity.normalized(), d->surfaceNormal)).mapVector(d->startVelocity);
    }

    // Rotation
    if (!m_particleRotation.isNull() || !m_particleRotationVariation.isNull()) {
        Vector3b rot;
        constexpr float step = 127.0f / 360.0f; // +/- 360-degrees as qint8 (-127..127)
        rot.x = m_particleRotation.x() * step;
        rot.y = m_particleRotation.y() * step;
        rot.z = m_particleRotation.z() * step;
        rot.x += (m_particleRotationVariation.x() - 2.0f * rand->get(particleIdIndex, QPRand::RotXV) * m_particleRotationVariation.x()) * step;
        rot.y += (m_particleRotationVariation.y() - 2.0f * rand->get(particleIdIndex, QPRand::RotYV) * m_particleRotationVariation.y()) * step;
        rot.z += (m_particleRotationVariation.z() - 2.0f * rand->get(particleIdIndex, QPRand::RotZV) * m_particleRotationVariation.z()) * step;
        d->startRotation = rot;
    }
    // Rotation velocity
    if (!m_particleRotationVelocity.isNull() || !m_particleRotationVelocityVariation.isNull()) {
        float rotVelX = m_particleRotationVelocity.x();
        float rotVelY = m_particleRotationVelocity.y();
        float rotVelZ = m_particleRotationVelocity.z();
        rotVelX += (m_particleRotationVelocityVariation.x() - 2.0f * rand->get(particleIdIndex, QPRand::RotXVV) * m_particleRotationVelocityVariation.x());
        rotVelY += (m_particleRotationVelocityVariation.y() - 2.0f * rand->get(particleIdIndex, QPRand::RotYVV) * m_particleRotationVelocityVariation.y());
        rotVelZ += (m_particleRotationVelocityVariation.z() - 2.0f * rand->get(particleIdIndex, QPRand::RotZVV) * m_particleRotationVelocityVariation.z());
        // Particle data rotations are in qint8 vec3 to save memory.
        // max value 127*127 = 16129 degrees/second
        float sign;
        sign = rotVelX < 0.0f ? -1.0f : 1.0f;
        rotVelX = std::max(-127.0f, std::min<float>(127.0f, sign * std::sqrt(abs(rotVelX))));
        sign = rotVelY < 0.0f ? -1.0f : 1.0f;
        rotVelY = std::max(-127.0f, std::min<float>(127.0f, sign * std::sqrt(abs(rotVelY))));
        sign = rotVelZ < 0.0f ? -1.0f : 1.0f;
        rotVelZ = std::max(-127.0f, std::min<float>(127.0f, sign * std::sqrt(abs(rotVelZ))));
        d->startRotationVelocity = { qint8(rotVelX), qint8(rotVelY), qint8(rotVelZ) };
    }

    // Colors
    QColor pc = particle->color();
    QVector4D pcv = particle->colorVariation();
    uchar r, g, b, a;
    if (particle->unifiedColorVariation()) {
        // Vary all color channels using the same random amount
        const int randVar = int(rand->get(particleIdIndex, QPRand::ColorAV) * 256);
        r = pc.red() * (1.0f - pcv.x()) + randVar * pcv.x();
        g = pc.green() * (1.0f - pcv.y()) + randVar * pcv.y();
        b = pc.blue() * (1.0f - pcv.z()) + randVar * pcv.z();
        a = pc.alpha() * (1.0f - pcv.w()) + randVar * pcv.w();
    } else {
        r = pc.red() * (1.0f - pcv.x()) + int(rand->get(particleIdIndex, QPRand::ColorRV) * 256) * pcv.x();
        g = pc.green() * (1.0f - pcv.y()) + int(rand->get(particleIdIndex, QPRand::ColorGV) * 256) * pcv.y();
        b = pc.blue() * (1.0f - pcv.z()) + int(rand->get(particleIdIndex, QPRand::ColorBV) * 256) * pcv.z();
        a = pc.alpha() * (1.0f - pcv.w()) + int(rand->get(particleIdIndex, QPRand::ColorAV) * 256) * pcv.w();
    }
    d->startColor = {r, g, b, a};

    // Sprite sequence animation
    if (auto sequence = particle->m_spriteSequence) {
        if (sequence->duration() > 0) {
            float animationTimeMs = float(sequence->duration()) / 1000.0f;
            float animationTimeVarMs = float(sequence->durationVariation()) / 1000.0f;
            animationTimeVarMs = animationTimeVarMs - 2.0f * rand->get(particleIdIndex, QPRand::SpriteAnimationV) * animationTimeVarMs;
            // Sequence duration to be at least 1ms
            const float MIN_DURATION = 0.001f;
            d->animationTime = std::max(MIN_DURATION, animationTimeMs + animationTimeVarMs);
        } else {
            // Duration not set, so use the lifetime of the particle
            d->animationTime = d->lifetime;
        }
    }
    d->reversed = m_reversed;
}

int QQuick3DParticleEmitter::getEmitAmountFromDynamicBursts(int triggerType)
{
    int amount = 0;
    const int currentTime = m_system->time();
    const int prevTime = m_prevBurstTime;
    // First go through dynamic bursts and see if any of them tiggers
    for (auto *burst : std::as_const(m_emitBursts)) {
        auto *burstPtr = qobject_cast<QQuick3DParticleDynamicBurst *>(burst);
        if (!burstPtr)
            continue;
        if (!burstPtr->m_enabled)
            continue;
        // Trigering on trail emitter start / end
        const bool trailTriggering = triggerType && (burstPtr->m_triggerMode) == triggerType;
        // Triggering on time for the first time
        const bool timeTriggeringStart = !triggerType && currentTime >= burstPtr->m_time && prevTime <= burstPtr->m_time;
        if (trailTriggering || timeTriggeringStart) {
            int burstAmount = burstPtr->m_amount;
            if (burstPtr->m_amountVariation > 0) {
                auto rand = m_system->rand();
                int randAmount = 2 * rand->get() * burstPtr->m_amountVariation;
                burstAmount += burstPtr->m_amountVariation - randAmount;
            }
            if (burstAmount > 0) {
                if (timeTriggeringStart && burstPtr->m_duration > 0) {
                    // Burst with duration, so generate burst data
                    BurstEmitData emitData;
                    emitData.startTime = currentTime;
                    emitData.endTime = currentTime + burstPtr->m_duration;
                    emitData.emitAmount = burstAmount;
                    emitData.prevBurstTime = prevTime;
                    m_burstEmitData << emitData;
                } else {
                    // Directly trigger the amount
                    amount += burstAmount;
                }
            }
        }
    }
    // Then go through the triggered emit bursts list
    for (int burstIndex = 0; burstIndex < m_burstEmitData.size(); ++burstIndex) {
        auto &burstData = m_burstEmitData[burstIndex];
        const int amountLeft = burstData.emitAmount - burstData.emitCounter;
        if (currentTime >= burstData.endTime) {
            // Burst time has ended, emit all rest of the particles and remove the burst
            amount += amountLeft;
            m_burstEmitData.removeAt(burstIndex);
        } else {
            // Otherwise burst correct amount depending on burst duration
            const int durationTime = currentTime - burstData.prevBurstTime;
            const int burstDurationTime = burstData.endTime - burstData.startTime;
            int burstAmount = burstData.emitAmount * (float(durationTime) / float(burstDurationTime));
            burstAmount = std::min(amountLeft, burstAmount);
            if (burstAmount > 0) {
                amount += burstAmount;
                burstData.emitCounter += burstAmount;
                burstData.prevBurstTime = currentTime;
            }
        }
    }
    // Reset the prev burst time
    m_prevBurstTime = currentTime;
    return amount;
}

int QQuick3DParticleEmitter::getEmitAmount()
{
    if (!m_system)
        return 0;

    if (!m_enabled)
        return 0;

    if (m_emitRate <= 0.0f)
        return 0;

    float timeChange = m_system->currentTime() - m_prevEmitTime;
    float emitAmountF = timeChange / (1000.0f / m_emitRate);
    int emitAmount = floorf(emitAmountF);
    // Store the partly unemitted particles
    // When emitAmount = 0, we just let the timeChange grow.
    if (emitAmount > 0) {
        m_unemittedF += (emitAmountF - emitAmount);
        // When unemitted grow to a full particle, emit it
        // This way if emit rate is 140 emitAmounts can be e.g. 2,2,3,2,2,3 etc.
        if (m_unemittedF >= 1.0f) {
            emitAmount++;
            m_unemittedF--;
        }
    }
    return emitAmount;
}

void QQuick3DParticleEmitter::emitParticlesBurst(const QQuick3DParticleEmitBurstData &burst)
{
    if (!m_system)
        return;

    if (!m_enabled)
        return;

    if (!m_particle)
        return;

    QMatrix4x4 transform = calculateParticleTransform(parentNode(), m_systemSharedParent);
    QQuaternion rotation = calculateParticleRotation(parentNode(), m_systemSharedParent);
    QVector3D centerPos = position() + burst.position;

    int emitAmount = std::min(burst.amount, int(m_particle->maxAmount()));
    for (int i = 0; i < emitAmount; i++) {
        // Distribute evenly between time and time+duration.
        float startTime = (burst.time / 1000.0f) + (float(1 + i) / emitAmount) * ((burst.duration) / 1000.0f);
        emitParticle(m_particle, startTime, transform, rotation, centerPos);
    }
}

// Called to emit set of particles
void QQuick3DParticleEmitter::emitParticles()
{
    if (!m_system)
        return;

    if (!m_enabled)
        return;

    if (!m_particle)
        return;

    auto *mbp = qobject_cast<QQuick3DParticleModelBlendParticle *>(m_particle);
    if (mbp && mbp->activationNode() && mbp->emitMode() == QQuick3DParticleModelBlendParticle::Activation) {
        // The particles are emitted using the activationNode instead of regular emit
        emitActivationNodeParticles(mbp);
        return;
    }

    const int systemTime = m_system->currentTime();

    if (systemTime < m_prevEmitTime) {
        // If we are goint backwards, reset previous emit time to current time.
        m_prevEmitTime = systemTime;
    } else {
        // Keep previous emitting time within max the life span.
        // This way emitting is reasonable also with big time jumps.
        const int maxLifeSpan = m_lifeSpan + m_lifeSpanVariation;
        m_prevEmitTime = std::max(m_prevEmitTime, systemTime - maxLifeSpan);
    }

    // If bursts have changed, generate them first in the beginning
    if (!m_burstGenerated)
       generateEmitBursts();

    int emitAmount = getEmitAmount() + getEmitAmountFromDynamicBursts();

    // With lower emitRates, let timeChange grow until at least 1 particle is emitted
    if (emitAmount < 1)
        return;

    QMatrix4x4 transform = calculateParticleTransform(parentNode(), m_systemSharedParent);
    QQuaternion rotation = calculateParticleRotation(parentNode(), m_systemSharedParent);
    QVector3D centerPos = position();

    emitAmount = std::min(emitAmount, int(m_particle->maxAmount()));
    for (int i = 0; i < emitAmount; i++) {
        // Distribute evenly between previous and current time, important especially
        // when time has jumped a lot (like a starttime).
        float startTime = (m_prevEmitTime / 1000.0f) + (float(1+i) / emitAmount) * ((systemTime - m_prevEmitTime) / 1000.0f);
        emitParticle(m_particle, startTime, transform, rotation, centerPos);
    }

    m_prevEmitTime = systemTime;
}

void QQuick3DParticleEmitter::emitActivationNodeParticles(QQuick3DParticleModelBlendParticle *particle)
{
    QMatrix4x4 matrix = particle->activationNode()->sceneTransform();
    QMatrix4x4 actTransform = sceneTransform().inverted() * matrix;
    QVector3D front = actTransform.column(2).toVector3D();
    QVector3D pos = actTransform.column(3).toVector3D();
    float d = QVector3D::dotProduct(pos, front);

    const int systemTime = m_system->currentTime();

    // Keep previous emitting time within max the life span.
    // This way emitting is reasonable also with big time jumps.
    const int maxLifeSpan = m_lifeSpan + m_lifeSpanVariation;
    m_prevEmitTime = std::max(m_prevEmitTime, systemTime - maxLifeSpan);

    float startTime = systemTime / 1000.0f;

    QMatrix4x4 transform = calculateParticleTransform(parentNode(), m_systemSharedParent);
    QQuaternion rotation = calculateParticleRotation(parentNode(), m_systemSharedParent);
    QVector3D centerPos = position();

    for (int i = 0; i < particle->maxAmount(); i++) {
        if (particle->m_particleData[i].startTime >= 0)
            continue;
        const QVector3D pc = particle->particleCenter(i);
        if (QVector3D::dotProduct(front, pc) - d > 0.0f)
            emitParticle(particle, startTime, transform, rotation, centerPos, i);
    }

    m_prevEmitTime = systemTime;
}

void QQuick3DParticleEmitter::componentComplete()
{
    if (!m_system && qobject_cast<QQuick3DParticleSystem *>(parentItem()))
        setSystem(qobject_cast<QQuick3DParticleSystem *>(parentItem()));

    // When dynamically creating emitters, start from the current time.
    if (m_system)
        m_prevEmitTime = m_system->currentTime();

    QQuick3DNode::componentComplete();
}

// EmitBursts - list handling

/*!
    \qmlproperty List<EmitBurst3D> ParticleEmitter3D::emitBursts

    This property takes a list of \l EmitBurst3D elements, to declaratively define bursts.
    If the burst starting time, amount, and duration are known beforehand, it is better to
    use this property than e.g. calling \l burst() with a \l Timer.

    For example, to emit 100 particles at the beginning, and 50 particles at 2 seconds:

    \qml
    ParticleEmitter3D {
        emitBursts: [
            EmitBurst3D {
                time: 0
                amount: 100
            },
            EmitBurst3D {
                time: 2000
                amount: 50
            }
        ]
    }
    \endqml

    \sa burst()
*/
QQmlListProperty<QQuick3DParticleEmitBurst> QQuick3DParticleEmitter::emitBursts()
{
    return {this, this,
             &QQuick3DParticleEmitter::appendEmitBurst,
             &QQuick3DParticleEmitter::emitBurstCount,
             &QQuick3DParticleEmitter::emitBurst,
             &QQuick3DParticleEmitter::clearEmitBursts,
             &QQuick3DParticleEmitter::replaceEmitBurst,
             &QQuick3DParticleEmitter::removeLastEmitBurst};
}

void QQuick3DParticleEmitter::appendEmitBurst(QQuick3DParticleEmitBurst* n) {
    m_emitBursts.append(n);
}

qsizetype QQuick3DParticleEmitter::emitBurstCount() const
{
    return m_emitBursts.size();
}

QQuick3DParticleEmitBurst *QQuick3DParticleEmitter::emitBurst(qsizetype index) const
{
    return m_emitBursts.at(index);
}

void QQuick3DParticleEmitter::clearEmitBursts() {
    m_emitBursts.clear();
}

void QQuick3DParticleEmitter::replaceEmitBurst(qsizetype index, QQuick3DParticleEmitBurst *n)
{
    m_emitBursts[index] = n;
}

void QQuick3DParticleEmitter::removeLastEmitBurst()
{
    m_emitBursts.removeLast();
}

// EmitBursts - static
void QQuick3DParticleEmitter::appendEmitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *list, QQuick3DParticleEmitBurst *p) {
    reinterpret_cast< QQuick3DParticleEmitter *>(list->data)->appendEmitBurst(p);
}

void QQuick3DParticleEmitter::clearEmitBursts(QQmlListProperty<QQuick3DParticleEmitBurst> *list) {
    reinterpret_cast< QQuick3DParticleEmitter *>(list->data)->clearEmitBursts();
}

void QQuick3DParticleEmitter::replaceEmitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *list, qsizetype i, QQuick3DParticleEmitBurst *p)
{
    reinterpret_cast< QQuick3DParticleEmitter *>(list->data)->replaceEmitBurst(i, p);
}

void QQuick3DParticleEmitter::removeLastEmitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *list)
{
    reinterpret_cast< QQuick3DParticleEmitter *>(list->data)->removeLastEmitBurst();
}

QQuick3DParticleEmitBurst* QQuick3DParticleEmitter::emitBurst(QQmlListProperty<QQuick3DParticleEmitBurst> *list, qsizetype i) {
    return reinterpret_cast< QQuick3DParticleEmitter *>(list->data)->emitBurst(i);
}

qsizetype QQuick3DParticleEmitter::emitBurstCount(QQmlListProperty<QQuick3DParticleEmitBurst> *list) {
    return reinterpret_cast< QQuick3DParticleEmitter *>(list->data)->emitBurstCount();
}

QT_END_NAMESPACE
