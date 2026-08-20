// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticle_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Particle3D
    \inherits Object3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Abstract logical particle.
    \since 6.2

    This element defines the common properties of the logical particles.
    Particle3D is an abstract base class of particles, use \l ModelParticle3D or \l SpriteParticle3D instead.

    \note Unlike the materials used with the models, particles default to being rendered with assuming
    semi-transparency, and so with blending enabled. This is the desired behavior most of the time due
    to particle textures, color (alpha) variations, fadings etc. If you don't need the blending,
    set the \l hasTransparency to \c false for possible performance gains.
*/

QQuick3DParticle::QQuick3DParticle(QQuick3DObject *parent)
    : QQuick3DObject(parent)
    , m_color(255, 255, 255, 255)
    , m_colorVariation(0, 0, 0, 0)
{
}

QQuick3DParticle::QQuick3DParticle(QQuick3DObjectPrivate &dd, QQuick3DNode *parent)
    : QQuick3DObject(dd, parent)
    , m_color(255, 255, 255, 255)
    , m_colorVariation(0, 0, 0, 0)
{

}

QQuick3DParticle::~QQuick3DParticle()
{
    if (m_system)
        m_system->unRegisterParticle(this);
}

QQuick3DParticleSystem *QQuick3DParticle::system() const
{
    return m_system;
}

void QQuick3DParticle::setSystem(QQuick3DParticleSystem *system)
{
    if (m_system == system)
        return;

    if (m_system)
        m_system->unRegisterParticle(this);

    m_system = system;
    if (m_system)
        m_system->registerParticle(this);
    Q_EMIT systemChanged();
}

/*!
    \qmlproperty int Particle3D::maxAmount

    This property defines the maximum amount of particles that can exist at the same time.
    You can use \l {ParticleSystem3DLogging::particlesUsed}{particlesUsed} for debugging how
    efficiently the allocated particles are used. If the maxAmount is too small, particles
    are reused before they reach the end of their \l {ParticleEmitter3D::lifeSpan}{lifeSpan}.
    If the maxAmount is too big, unnecessary memory is allocated for the particles.

    \note Changing the maxAmount resets all the particles in the particle system.

    The default value is \c 100.
*/
int QQuick3DParticle::maxAmount() const
{
    return m_maxAmount;
}

void QQuick3DParticle::setMaxAmount(int maxAmount)
{
    if (m_maxAmount == maxAmount)
        return;

    doSetMaxAmount(maxAmount);
}

void QQuick3DParticle::doSetMaxAmount(int amount)
{
    m_maxAmount = amount;
    Q_EMIT maxAmountChanged();
}

/*!
    \qmlproperty color Particle3D::color

    This property defines the base color that is used for colorizing the particles.

    The default value is \c "#FFFFFF" (white).
*/
QColor QQuick3DParticle::color() const
{
    return m_color;
}

float QQuick3DParticle::opacity() const
{
    return m_color.alphaF();
}


void QQuick3DParticle::setColor(QColor color)
{
    if (m_color == color)
        return;

    m_color = color;
    Q_EMIT colorChanged();
}

// When setting color to undefined, reset particle
// to use its own color instead
void QQuick3DParticle::resetColor()
{
    m_color = QColor(255, 255, 255, 255);
    m_colorVariation = QVector4D(0, 0, 0, 0);
}

/*!
    \qmlproperty vector4d Particle3D::colorVariation

    This property defines the color variation that is used for colorizing the particles.
    The values are in RGBA order and each value should be between 0.0 (no variation) and 1.0
    (full variation).

    For example, to create particles which will have translucent red colors between
    \c #ff0000 and \c #e50000, with 40% to 60% opacity:

    \qml
    ModelParticle3D {
        ...
        color: "#7fff0000"
        colorVariation: Qt.vector4d(0.1, 0.0, 0.0, 0.2)
    }
    \endqml

    The default value is \c (0, 0, 0, 0) (no variation).

    \sa unifiedColorVariation
*/
QVector4D QQuick3DParticle::colorVariation() const
{
    return m_colorVariation;
}

void QQuick3DParticle::setColorVariation(QVector4D colorVariation)
{
    if (m_colorVariation == colorVariation)
        return;

    m_colorVariation = colorVariation;
    Q_EMIT colorVariationChanged();
}

/*!
    \qmlproperty bool Particle3D::unifiedColorVariation

    This property defines if the \l colorVariation should be applied uniformly for all
    the color channels. This means that all variations are applied with the same
    random amount.

    For example, to create particles which will have yellow colors between
    \c #ffff00 and \c #7f7f00, so that the values of \c R and \c G color channels are
    always the same:

    \qml
    ModelParticle3D {
        ...
        color: "#ffff00"
        colorVariation: Qt.vector4d(0.5, 0.5, 0.0, 0.0)
        unifiedColorVariation: true
    }
    \endqml

    The default value is \c false.

    \sa colorVariation
*/
bool QQuick3DParticle::unifiedColorVariation() const
{
    return m_unifiedColorVariation;
}

void QQuick3DParticle::setUnifiedColorVariation(bool unified)
{
    if (m_unifiedColorVariation == unified)
        return;

    m_unifiedColorVariation = unified;
    Q_EMIT unifiedColorVariationChanged();
}

/*!
    \qmlproperty enumeration Particle3D::FadeType

    Defines the type of the fading effect.

    \value Particle3D.FadeNone
        No fading.
    \value Particle3D.FadeOpacity
        Fade the particle opacity from/to 0.0.
    \value Particle3D.FadeScale
        Fade the particle scale from/to 0.0.
*/

/*!
    \qmlproperty FadeType Particle3D::fadeInEffect

    This property defines the fading effect used when the particles appear.

    The default value is \c Particle3D.FadeOpacity.

    \sa fadeInDuration, fadeOutEffect
*/
QQuick3DParticle::FadeType QQuick3DParticle::fadeInEffect() const
{
    return m_fadeInEffect;
}

void QQuick3DParticle::setFadeInEffect(FadeType fadeInEffect)
{
    if (m_fadeInEffect == fadeInEffect)
        return;

    m_fadeInEffect = fadeInEffect;
    Q_EMIT fadeInEffectChanged();
}

/*!
    \qmlproperty FadeType Particle3D::fadeOutEffect

    This property defines the fading effect used when the particles reach their
    \l {ParticleEmitter3D::lifeSpan}{lifeSpan} and disappear.

    The default value is \c Particle3D.FadeOpacity.

    \sa fadeOutDuration, fadeInEffect
*/
QQuick3DParticle::FadeType QQuick3DParticle::fadeOutEffect() const
{
    return m_fadeOutEffect;
}

void QQuick3DParticle::setFadeOutEffect(FadeType fadeOutEffect)
{
    if (m_fadeOutEffect == fadeOutEffect)
        return;

    m_fadeOutEffect = fadeOutEffect;
    Q_EMIT fadeOutEffectChanged();
}

/*!
    \qmlproperty int Particle3D::fadeInDuration

    This property defines the duration in milliseconds for the fading in effect.

    \note The fading durations are part of the particles
    \l {ParticleEmitter3D::lifeSpan}{lifeSpan}. So e.g. if \c lifeSpan is 3000,
    \c fadeInDuration is 500 and \c fadeOutDuration is 500, the fully visible
    time of the particle is 2000ms.

    The default value is \c 250.

    \sa fadeInEffect, fadeOutDuration
*/
int QQuick3DParticle::fadeInDuration() const
{
    return m_fadeInDuration;
}

void QQuick3DParticle::setFadeInDuration(int fadeInDuration)
{
    if (m_fadeInDuration == fadeInDuration)
        return;

    m_fadeInDuration = fadeInDuration;
    Q_EMIT fadeInDurationChanged();
}

/*!
    \qmlproperty int Particle3D::fadeOutDuration

    This property defines the duration in milliseconds for the fading out effect.

    The default value is \c 250.

    \sa fadeOutEffect, fadeInDuration
*/
int QQuick3DParticle::fadeOutDuration() const
{
    return m_fadeOutDuration;
}

void QQuick3DParticle::setFadeOutDuration(int fadeOutDuration)
{
    if (m_fadeOutDuration == fadeOutDuration)
        return;

    m_fadeOutDuration = fadeOutDuration;
    Q_EMIT fadeOutDurationChanged();
}

/*!
    \qmlproperty enumeration Particle3D::AlignMode

    Defines the type of the alignment.

    \value Particle3D.AlignNone
        No alignment. Particles rotation can be defined with \l {ParticleEmitter3D::particleRotation}{particleRotation}.
    \value Particle3D.AlignTowardsTarget
        Align the particles towards \l alignTargetPosition direction.
    \value Particle3D.AlignTowardsStartVelocity
        Align the particles towards their starting \l {ParticleEmitter3D::velocity}{velocity}
        direction.
*/

/*!
    \qmlproperty AlignMode Particle3D::alignMode

    This property defines the align mode used for the particles.
    Particle alignment means the direction that particles face.

    \note When the \l SpriteParticle3D \l {SpriteParticle3D::billboard}{billboard}
    property is set to \c true, alignMode does not have an effect.

    The default value is \c Particle3D.AlignNone.

    \sa alignTargetPosition
*/
QQuick3DParticle::AlignMode QQuick3DParticle::alignMode() const
{
    return m_alignMode;
}

/*!
    \qmlproperty vector3d Particle3D::alignTargetPosition

    This property defines the position particles are aligned to.
    This property has effect only when the \l alignMode is set to
    \c Particle3D.AlignTowardsTarget.

    \sa alignMode
*/
QVector3D QQuick3DParticle::alignTargetPosition() const
{
    return m_alignTarget;
}

/*!
    \qmlproperty bool Particle3D::hasTransparency

    This property defines if the particle has any transparency and should
    be blended with the background. Usually this should be true, like when
    the particle color doesn't have full alpha, texture contains semi-transparent
    pixels or particles opacity is faded in or out. Setting this to false can be
    an optimization in specific cases.

    The default value is \c true.

    \sa color, fadeInEffect, fadeOutEffect
*/
bool QQuick3DParticle::hasTransparency() const
{
    return m_hasTransparency;
}

void QQuick3DParticle::setHasTransparency(bool transparency)
{
    if (m_hasTransparency == transparency)
        return;

    m_hasTransparency = transparency;
    Q_EMIT hasTransparencyChanged();
}

void QQuick3DParticle::setAlignMode(AlignMode alignMode)
{
    if (m_alignMode == alignMode)
        return;

    m_alignMode = alignMode;
    Q_EMIT alignModeChanged();
}

void QQuick3DParticle::setAlignTargetPosition(const QVector3D &alignPosition)
{
    if (m_alignTarget == alignPosition)
        return;

    m_alignTarget = alignPosition;
    Q_EMIT alignTargetPositionChanged();
}

/*!
    \qmlproperty enumeration Particle3D::SortMode

    Defines the sorting mode of the particle. The sorting mode determines
    the order in which the particles are drawn.

    \value Particle3D.SortNone
        No sorting.
    \value Particle3D.SortNewest
        Sort based on particle lifetime, newest first.
    \value Particle3D.SortOldest
        Sort based on particle lifetime, oldest first.
    \value Particle3D.SortDistance
        Sort based on distance to the camera, farthest first.
*/


/*!
    \qmlproperty SortMode Particle3D::sortMode

    This property defines the sort mode used for the particles.

    The default value is \c Particle3D.SortNone.
*/
QQuick3DParticle::SortMode QQuick3DParticle::sortMode() const
{
    return m_sortMode;
}

void QQuick3DParticle::setSortMode(QQuick3DParticle::SortMode mode)
{
    if (m_sortMode == mode)
        return;
    m_sortMode = mode;
    Q_EMIT sortModeChanged();
}

void QQuick3DParticle::updateBurstIndex(int amount)
{
    m_lastBurstIndex += amount;
}

int QQuick3DParticle::nextCurrentIndex(const QQuick3DParticleEmitter *)
{
    m_currentIndex = (m_currentIndex < m_maxAmount - 1) ? m_currentIndex + 1 : m_lastBurstIndex;
    return m_currentIndex;
}

void QQuick3DParticle::componentComplete()
{
    QQuick3DObject::componentComplete();
    // Make sure the default amount gets initialized, even if user doesn't set it
    Q_EMIT maxAmountChanged();
}

void QQuick3DParticle::reset() {
    m_currentIndex = -1;
    m_lastBurstIndex = 0;

    // Reset all particles data
    m_particleData.fill({});
}

QT_END_NAMESPACE
