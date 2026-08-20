// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlescaleaffector_p.h"
#include <qmath.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ScaleAffector3D
    \inherits Affector3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Particle scale affector.
    \since 6.4

    Scale affector scales the particle size based on its lifetime and parameters.
*/

QQuick3DParticleScaleAffector::QQuick3DParticleScaleAffector(QQuick3DNode *parent)
    : QQuick3DParticleAffector(parent)
{

}

/*!
    \qmlproperty real ScaleAffector3D::minSize

    This property holds the minimum size the affector can scale the particle.
    The default is 1.0.
*/
float QQuick3DParticleScaleAffector::minSize() const
{
    return m_minSize;
}

/*!
    \qmlproperty real ScaleAffector3D::maxSize

    This property holds the maximum size the affector can scale the particle.
    The default is 1.0.
*/
float QQuick3DParticleScaleAffector::maxSize() const
{
    return m_maxSize;
}

/*!
    \qmlproperty int ScaleAffector3D::duration

    This property holds the duration of scaling cycle in milliseconds.
    The default is 1000.
*/
int QQuick3DParticleScaleAffector::duration() const
{
    return m_duration;
}

/*!
    \qmlproperty enumeration ScaleAffector3D::ScalingType

    Defines the scaling type of the affector.

    \value ScaleAffector3D.Linear
        The scale is calculated using the easing curve to interpolate between minimum and maximum
        scale size between duration milliseconds and then continues from the minimum size.
    \value ScaleAffector3D.SewSaw
        The scale is calculated using the easing curve to interpolate between minimum and maximum
        scale size between duration milliseconds on a rising edge then continues from maximum to minimum
        on a falling edge.
    \value ScaleAffector3D.SineWave
        The scale follows the sine wave. Easing curve is not used.
    \value ScaleAffector3D.AbsSineWave
        The scale follows the sine wave except negative values are inverted. Easing curve is not used.
    \value ScaleAffector3D.Step
        The scale stays at minimum size until half of the duration milliseconds have passed then steps directly
        to the maximum size. Easing curve is not used.
    \value ScaleAffector3D.SmoothStep
        The scale smootly transitions from minimum to maximum size. Easing curve is not used.
*/

/*!
    \qmlproperty ScalingType ScaleAffector3D::type

    This property holds the scaling type of the affector. The default value is \c Linear.
*/
QQuick3DParticleScaleAffector::ScalingType QQuick3DParticleScaleAffector::type() const
{
    return m_type;
}

/*!
    \qmlproperty EasingCurve ScaleAffector3D::easingCurve

    This property holds the \l {QtQuick::PropertyAnimation::easing}{easing curve} providing
    more fine tuned control on how the scaling occurs. The easing curve is used with \c Linear
    and \c SewSaw scaling types. The default easing curve provides linear value between [0, 1].
*/
QEasingCurve QQuick3DParticleScaleAffector::easingCurve() const
{
    return m_easing;
}

void QQuick3DParticleScaleAffector::setMinSize(float size)
{
    if (qFuzzyCompare(size, m_minSize))
        return;
    m_minSize = size;
    Q_EMIT minSizeChanged();
}

void QQuick3DParticleScaleAffector::setMaxSize(float size)
{
    if (qFuzzyCompare(size, m_maxSize))
        return;
    m_maxSize = size;
    Q_EMIT maxSizeChanged();
}

void QQuick3DParticleScaleAffector::setDuration(int duration)
{
    duration = qMax(0, duration);
    if (duration == m_duration)
        return;
    m_duration = duration;
    Q_EMIT durationChanged();
}

void QQuick3DParticleScaleAffector::setType(ScalingType type)
{
    if (m_type == type)
        return;
    m_type = type;
    Q_EMIT typeChanged();
}

void QQuick3DParticleScaleAffector::setEasingCurve(const QEasingCurve &curve)
{
    if (m_easing == curve)
        return;
    m_easing = curve;
    Q_EMIT easingCurveChanged();
}

void QQuick3DParticleScaleAffector::prepareToAffect()
{

}

void QQuick3DParticleScaleAffector::affectParticle(const QQuick3DParticleData &, QQuick3DParticleDataCurrent *d, float time)
{
    float scale = 1.0f;

    const auto fract = [](const float v) -> float {
        return v - qFloor(v);
    };
    const auto lerp = [](const float a, const float b, const float f) -> float {
        return a + (b - a) * f;
    };
    const auto smoothstep = [](const float a, const float b, const float f) -> float {
        return a + (b - a) * f * f * (3.0f - 2.0f * f);
    };

    float pos = fract(time / float(m_duration * 0.001f));
    switch (m_type) {
    case Linear:
        scale = lerp(m_minSize, m_maxSize, m_easing.valueForProgress(pos));
        scale = qMax(scale, 0.0f);
        break;
    case SewSaw:
        if (pos < 0.5f)
            scale = lerp(m_minSize, m_maxSize, m_easing.valueForProgress(pos * 2.0f));
        else
            scale = lerp(m_maxSize, m_minSize, m_easing.valueForProgress((pos - 0.5) * 2.0f));
        scale = qMax(scale, 0.0f);
        break;
    case SineWave:
        scale = m_minSize + (m_maxSize - m_minSize) * (1.0f + qSin(2.0f * M_PI * pos)) * 0.5f;
        break;
    case AbsSineWave:
        scale = m_minSize + (m_maxSize - m_minSize) * qAbs(qSin(2.0f * M_PI * pos));
        break;
    case Step:
        if (pos < 0.5f)
            scale = m_minSize;
        else
            scale = m_maxSize;
        break;
    case SmoothStep:
        scale = smoothstep(m_minSize, m_maxSize, pos);
        break;
    }

    d->scale *= scale;
}

QT_END_NAMESPACE
