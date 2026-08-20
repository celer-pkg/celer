// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrabstracthapticeffect_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype XrHapticEffect
    \qmlabstract
    \inherits QtObject
    \inqmlmodule QtQuick3D.Xr
    \brief Represents a haptic effect.
    \since 6.9

    XrHapticEffect defines the characteristics of the haptic effect.

    This type is abstract. Use a subtype, such as \l XrSimpleHapticEffect.

    \sa XrHapticFeedback XrSimpleHapticEffect
 */

/*!
    \qmltype XrSimpleHapticEffect
    \inherits XrHapticEffect
    \inqmlmodule QtQuick3D.Xr
    \brief Allows setting controller haptics using amplitude, duration and frequency.
    \since 6.9

    \qml
    XrSimpleHapticEffect {
        amplitude: 0.5
        duration: 300
        frequency: 3000
    }
    \endqml

    \sa XrHapticFeedback
*/

/*!
    \qmlproperty bool XrSimpleHapticEffect::amplitude()
    \brief Defines the amplitude of the effect's vibration.
    Acceptable values are from 0.0 to 1.0
    \default 0.5
 */
float QQuick3DXrSimpleHapticEffect::amplitude()
{
    return m_amplitude;
}

void QQuick3DXrSimpleHapticEffect::setAmplitude(float newAmplitude)
{
    if (m_amplitude == newAmplitude)
        return;
    m_amplitude = newAmplitude;
    emit amplitudeChanged();
}

/*!
    \qmlproperty bool XrSimpleHapticEffect::duration()
    \brief Defines the duration of the haptic effect in milliseconds.
    \default 30
 */
float QQuick3DXrSimpleHapticEffect::duration()
{
    return m_duration;
}

void QQuick3DXrSimpleHapticEffect::setDuration(float newDuration)
{
    if (m_duration == newDuration)
        return;
    m_duration = newDuration;
    emit durationChanged();
}

/*!
    \qmlproperty bool XrSimpleHapticEffect::frequency()
    \brief Defines the frequency of the haptic effect in Hz
    \default 3000
 */

float QQuick3DXrSimpleHapticEffect::frequency()
{
    return m_frequency;
}

void QQuick3DXrSimpleHapticEffect::setFrequency(float newFrequency)
{
    if (m_frequency == newFrequency)
        return;
    m_frequency = newFrequency;
    emit frequencyChanged();
}

QT_END_NAMESPACE
