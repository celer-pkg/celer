// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticleemitburst_p.h"
#include "qquick3dparticleemitter_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype EmitBurst3D
    \inherits QtObject
    \inqmlmodule QtQuick3D.Particles3D
    \brief Declarative emitter bursts.
    \since 6.2

    This element defines particle bursts in the \l ParticleEmitter3D. These bursts are
    static, meaning that they are evaluated when the particlesystem starts. This allows
    better performance than \l DynamicBurst3D and bursting outside of the particlesystem
    time (so e.g. burst at 1000ms while system time starts from 2000ms).
    \note EmitBurst3D uses emitter properties (position, rotation etc.) at the
    particlesystem start. For dynamic emitters, use \l DynamicBurst3D instead.

    For example, to emit 100 particles at the beginning, and 50 particles at 2 seconds,
    so that both bursts take 200 milliseconds:

    \qml
    ParticleEmitter3D {
        ...
        emitBursts: [
            EmitBurst3D {
                time: 0
                amount: 100
                duration: 200
            },
            EmitBurst3D {
                time: 2000
                amount: 50
                duration: 200
            }
        ]
    }
    \endqml
*/

QQuick3DParticleEmitBurst::QQuick3DParticleEmitBurst(QObject *parent)
    : QObject(parent)
{
    m_parentEmitter = qobject_cast<QQuick3DParticleEmitter *>(parent);
}

QQuick3DParticleEmitBurst::~QQuick3DParticleEmitBurst()
{
    if (m_parentEmitter)
        m_parentEmitter->unRegisterEmitBurst(this);
}

/*!
    \qmlproperty int EmitBurst3D::time

    This property defines the time in milliseconds when emitting the burst starts.

    The default value is \c 0.
*/
int QQuick3DParticleEmitBurst::time() const
{
    return m_time;
}

/*!
    \qmlproperty int EmitBurst3D::amount

    This property defines the amount of particles emitted during the burst.

    The default value is \c 0.
*/
int QQuick3DParticleEmitBurst::amount() const
{
    return m_amount;
}

/*!
    \qmlproperty int EmitBurst3D::duration

    This property defines the duration of the burst. The default value is 0,
    meaning all particles will burst at the beginning of \l time.
    If the duration is set, particles emitting is distributed between \c time
    and \c time + \c duration.

    For example, to have emit rate of 400 between 1000 and 1200 milliseconds:
    \qml
    EmitBurst3D {
        time: 1000
        amount: 80
        duration: 1200
    }
    \endqml

    The default value is \c 0.
*/
int QQuick3DParticleEmitBurst::duration() const
{
    return m_duration;
}

void QQuick3DParticleEmitBurst::setTime(int time)
{
    if (m_time == time)
        return;

    m_time = time;
    Q_EMIT timeChanged();
}

void QQuick3DParticleEmitBurst::setAmount(int amount)
{
    if (m_amount == amount)
        return;
    if (amount < 0) {
        qWarning () << "EmitBurst3D: Amount must be positive.";
        return;
    }
    m_amount = amount;
    Q_EMIT amountChanged();
}

void QQuick3DParticleEmitBurst::setDuration(int duration)
{
    if (m_duration == duration)
        return;
    if (duration < 0) {
        qWarning () << "EmitBurst3D: Duration must be positive.";
        return;
    }
    m_duration = duration;
    Q_EMIT durationChanged();
}

void QQuick3DParticleEmitBurst::componentComplete()
{
    m_parentEmitter = qobject_cast<QQuick3DParticleEmitter *>(parent());
    if (m_parentEmitter)
        m_parentEmitter->registerEmitBurst(this);
    else
        qWarning() << "EmitBurst requires parent Emitter to function correctly!";
}

QT_END_NAMESPACE
