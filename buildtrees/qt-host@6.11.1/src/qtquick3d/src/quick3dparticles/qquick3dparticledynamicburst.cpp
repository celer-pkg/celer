// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticledynamicburst_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype DynamicBurst3D
    \inherits EmitBurst3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Dynamic emitter bursts.
    \since 6.3

    This element defines particle bursts in the \l ParticleEmitter3D. These bursts are
    dynamic, meaning that they are evaluated while the particlesystem runs. Use these
    instead of \l EmitBurst3D for example when the emitter moves, so that emitting
    happens at the correct position.

    For example, to emit 100 particles at 1 second time and 200 particles at 2 second:

    \qml
    ParticleEmitter3D {
        ...
        emitBursts: [
            DynamicBurst3D {
                time: 1000
                amount: 100
            },
            DynamicBurst3D {
                time: 2000
                amount: 200
            }
        ]
    }
    \endqml
*/

QQuick3DParticleDynamicBurst::QQuick3DParticleDynamicBurst(QObject *parent)
    : QQuick3DParticleEmitBurst(parent)
{

}

/*!
    \qmlproperty bool DynamicBurst3D::enabled

    If enabled is set to \c false, this burst will not emit any particles.

    The default value is \c true.
*/
bool QQuick3DParticleDynamicBurst::enabled() const
{
    return m_enabled;
}

/*!
    \qmlproperty int DynamicBurst3D::amountVariation

    This property defines the random variation in particle emit amount.

    For example, to have a random range between 0 and 10
    \qml
    DynamicBurst3D {
        time: 1000
        amount: 5
        amountVariation: 5
    }
    \endqml

    The default value is \c 0.
*/
int QQuick3DParticleDynamicBurst::amountVariation() const
{
    return m_amountVariation;
}

/*!
    \qmlproperty ShapeType DynamicBurst3D::triggerMode

    This property defines the emitting mode.

    The default value is \c TriggerMode.TriggerTime.
*/

/*!
    \qmlproperty enumeration DynamicBurst3D::TriggerMode

    Defines the mode of the bursting.

    \value DynamicBurst3D.TriggerTime
        The particles are emitted when the burst \l {EmitBurst3D::time}{time} is due.
    \value DynamicBurst3D.TriggerStart
        The particles are emitted when the followed particle is emitted.
        \note This property is restricted to only work with trail emitters.
        \note In this mode, \c time and \c duration properties don't have an effect.
    \value DynamicBurst3D.TriggerEnd
        The particles are emitted when the followed particle \c lifeSpan ends.
        \note This property is restricted to only work with trail emitters.
        \note In this mode, \c time and \c duration properties don't have an effect.
*/

QQuick3DParticleDynamicBurst::TriggerMode QQuick3DParticleDynamicBurst::triggerMode() const
{
    return m_triggerMode;
}

void QQuick3DParticleDynamicBurst::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    Q_EMIT enabledChanged();
}

void QQuick3DParticleDynamicBurst::setAmountVariation(int value)
{
    if (m_amountVariation == value)
        return;

    if (value < 0) {
        qWarning () << "DynamicBurst3D: Amount variation must be positive.";
        return;
    }
    m_amountVariation = value;
    Q_EMIT amountVariationChanged();
}

void QQuick3DParticleDynamicBurst::setTriggerMode(TriggerMode mode)
{
    if (m_triggerMode == mode)
        return;

    m_triggerMode = mode;
    Q_EMIT triggerModeChanged();
}

QT_END_NAMESPACE
