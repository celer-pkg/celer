// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlespritesequence_p.h"
#include "qquick3dparticlespriteparticle_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype SpriteSequence3D
    \inherits QtObject
    \inqmlmodule QtQuick3D.Particles3D
    \brief Provides image sequence features for the Sprite particles.
    \since 6.2

    The SpriteSequence3D element provides support for animated images with multiple frames. The frames
    should be aligned horizontally in the image, first frame being on the left and last on the right.

    To make a \l SpriteParticle3D an animated sequence, set its \l {SpriteParticle3D::spriteSequence}{spriteSequence} property.
*/

QQuick3DParticleSpriteSequence::QQuick3DParticleSpriteSequence(QObject *parent)
    : QObject(parent)
{

}

QQuick3DParticleSpriteSequence::~QQuick3DParticleSpriteSequence()
{
   if (m_parentParticle)
        m_parentParticle->setSpriteSequence(nullptr);
}

/*!
    \qmlproperty int SpriteSequence3D::frameCount

    This property defines the amount of image frames in \l {SpriteParticle3D::}{sprite}.
    Particle animates through these frames during its \l duration.
    The frames should be laid out horizontally in the same image file. For example,
    \e sprite could be a \c {512x64} image, with \c frameCount of \c 8. This would make
    each particle frame size \c {64x64} pixels.

    The default value is \c 1.

    \note If your image only has a single sprite frame, don't define the
    \l {SpriteParticle3D::}{spriteSequence} property at all.

    \sa interpolate
*/
int QQuick3DParticleSpriteSequence::frameCount() const
{
    return m_frameCount;
}

/*!
    \qmlproperty int SpriteSequence3D::frameIndex

    This property defines the initial index of the frame. This is the position in between frames
    where the animation is started. For example when the \c frameIndex is 5 and the \l animationDirection
    is \c Normal, the first rendered frame is 5. If the \l animationDirection is \c Reverse, the
    first rendered frame is 4.

    The value of frameIndex must be between 0 and \l{frameCount} - \c 1. When the \l animationDirection
    is \c SingleFrame and \l randomStart is \c false, all the particles will render sprites with the
    \c frameIndex.

    The default value is \c 0.

    \sa randomStart, animationDirection
*/
int QQuick3DParticleSpriteSequence::frameIndex() const
{
    return m_frameIndex;
}

/*!
    \qmlproperty bool SpriteSequence3D::interpolate

    This property defines if the sprites are interpolated (blended) between frames
    to make the animation appear smoother.

    The default value is \c true.

    \sa frameCount
*/
bool QQuick3DParticleSpriteSequence::interpolate() const
{
    return m_interpolate;
}

/*!
    \qmlproperty int SpriteSequence3D::duration

    This property defines the duration in milliseconds how long it takes for the
    sprite sequence to animate. For example, if the \l duration is \c 400 and the
    \l frameCount is 8, each frame will be shown for 50 milliseconds. When the
    value is -1, the particle lifeSpan is used as the duration.

    The default value is \c -1.
*/
int QQuick3DParticleSpriteSequence::duration() const
{
    return m_duration;
}

/*!
    \qmlproperty int SpriteSequence3D::durationVariation

    This property defines the duration variation in milliseconds. The actual duration
    of the animation is between \c duration - \c durationVariation and \c duration +
    \c durationVariation.

    The default value is \c 0 (no variation).
*/
int QQuick3DParticleSpriteSequence::durationVariation() const
{
    return m_durationVariation;
}

/*!
    \qmlproperty bool SpriteSequence3D::randomStart

    This property defines if the animation should start from a random frame between \c 0 and \l frameCount - \c 1.
    This allows animations to not look like they all just started when the animation begins.

    The default value is \c false.

    \sa animationDirection
*/
bool QQuick3DParticleSpriteSequence::randomStart() const
{
    return m_randomStart;
}

/*!
    \qmlproperty AnimationDirection SpriteSequence3D::animationDirection

    This property defines the animation direction of the sequence.

    The default value is \c SpriteSequence3D.Normal.

    \sa randomStart
*/

/*!
    \qmlproperty enumeration SpriteSequence3D::AnimationDirection

    Defines the animation playback direction of the sequence.

    \value SpriteSequence3D.Normal
        Animate from the first frame to the last frame. When the last frame is reached, jump back to the first frame.
    \value SpriteSequence3D.Reverse
        Animate from the last frame to the first frame. When the first frame is reached, jump back to the last frame.
    \value SpriteSequence3D.Alternate
        Animate from the first frame to the last frame. When the last or first frame is reached, switch the animation direction.
        This makes the sequence animation smooth even when the first and the last frames don't match.
    \value SpriteSequence3D.AlternateReverse
        Animate from the last frame to the first frame. When the last or first frame is reached, switch the animation direction.
        This makes the sequence animation smooth even when the first and the last frames don't match.
    \value SpriteSequence3D.SingleFrame
        Don't animate the frame. When the \l randomStart is false, \l frameIndex frame is rendered.
        When the \l randomStart is true, each particle renders a random frame.
*/
QQuick3DParticleSpriteSequence::AnimationDirection QQuick3DParticleSpriteSequence::animationDirection() const
{
    return m_animationDirection;
}

void QQuick3DParticleSpriteSequence::setFrameCount(int frameCount)
{
    if (m_frameCount == frameCount)
        return;
    m_frameCount = std::max(1, frameCount);
    markNodesDirty();
    Q_EMIT frameCountChanged();
}

void QQuick3DParticleSpriteSequence::setFrameIndex(int frameIndex)
{
    if (m_frameIndex == frameIndex)
        return;
    m_frameIndex = std::max(0, frameIndex);
    markNodesDirty();
    Q_EMIT frameIndexChanged();
}

void QQuick3DParticleSpriteSequence::setInterpolate(bool interpolate)
{
    if (m_interpolate == interpolate)
        return;
    m_interpolate = interpolate;
    markNodesDirty();
    Q_EMIT interpolateChanged();
}

void QQuick3DParticleSpriteSequence::setDuration(int duration)
{
    if (m_duration == duration)
        return;

    m_duration = duration;
    markNodesDirty();
    Q_EMIT durationChanged();
}

void QQuick3DParticleSpriteSequence::setDurationVariation(int durationVariation)
{
    if (m_durationVariation == durationVariation)
        return;

    m_durationVariation = durationVariation;
    markNodesDirty();
    Q_EMIT durationVariationChanged();
}

void QQuick3DParticleSpriteSequence::setRandomStart(bool randomStart)
{
    if (m_randomStart == randomStart)
        return;
    m_randomStart = randomStart;
    markNodesDirty();
    Q_EMIT randomStartChanged();
}

void QQuick3DParticleSpriteSequence::setAnimationDirection(QQuick3DParticleSpriteSequence::AnimationDirection animationDirection)
{
    if (m_animationDirection == animationDirection)
        return;
    m_animationDirection = animationDirection;
    markNodesDirty();
    Q_EMIT animationDirectionChanged();
}

void QQuick3DParticleSpriteSequence::componentComplete()
{
    m_parentParticle = qobject_cast<QQuick3DParticleSpriteParticle *>(parent());
    if (!m_parentParticle)
        qWarning() << "SpriteSequence3D requires parent SpriteParticle3D to function correctly!";
}

void QQuick3DParticleSpriteSequence::markNodesDirty()
{
    if (m_parentParticle)
        m_parentParticle->markNodesDirty();
}

// Returns the first frame of the sequence.
// Return range [0..1) where 0.0 is the first frame and 0.9999 is the last.
float QQuick3DParticleSpriteSequence::firstFrame(int index, bool singleFrame)
{
    float firstFrame = 0.0f;
    if (m_randomStart) {
        if (!m_parentParticle || !m_parentParticle->m_system)
            return firstFrame;
        auto rand = m_parentParticle->m_system->rand();
        firstFrame = rand->get(index, QPRand::SpriteAnimationI);
    } else if (m_frameCount > 1 && m_frameIndex > 0) {
        int frameIndex = std::min(m_frameIndex, m_frameCount - 1);
        if (singleFrame)
            firstFrame = float(frameIndex) / (float(m_frameCount - 1) + 0.0001f);
        else
            firstFrame = float(frameIndex) / float(m_frameCount);
    }
    return firstFrame;
}

QT_END_NAMESPACE
