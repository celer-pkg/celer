// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquicktimelineanimation_p.h"

#include "qquicktimeline_p.h"

#include <private/qobject_p.h>
#include <private/qquickanimation_p_p.h>

#include <QTimer>

QT_BEGIN_NAMESPACE

/*!
    \qmltype TimelineAnimation
    \inherits NumberAnimation
    \nativetype QQuickTimelineAnimation
    \inqmlmodule QtQuick.Timeline
    \ingroup qtqmltypes

    \brief A number animation attached to a timeline.

    Specifies how the current frame property of a timeline is animated. This
    animates the properties of the objects targeted by the timeline.
*/

/*!
    \qmlproperty bool TimelineAnimation::pingPong

    Whether the animation is played backwards after it finishes. This is an easy
    way to create circular animations.
*/

/*!
    \qmlsignal TimelineAnimation::finished

    This signal is emitted when the timeline animation finishes.
*/

QQuickTimelineAnimation::QQuickTimelineAnimation(QObject *parent) : QQuickNumberAnimation(parent)
{
    setProperty(QLatin1String("currentFrame"));
    connect(this, &QQuickAbstractAnimation::started, this, &QQuickTimelineAnimation::handleStarted);
    connect(this, &QQuickAbstractAnimation::stopped, this, &QQuickTimelineAnimation::handleStopped);
}

void QQuickTimelineAnimation::setPingPong(bool b)
{
    if (b == m_pinpong)
        return;

    m_pinpong = b;
    emit pingPongChanged();
}

bool QQuickTimelineAnimation::pingPong() const
{
    return m_pinpong;
}

void QQuickTimelineAnimation::handleStarted()
{
    auto timeline = qobject_cast<QQuickTimeline*>(parent());

    if (!timeline)
        return;

    for (QQuickTimelineAnimation *other : timeline->getAnimations()) {
        if (other != this)
            other->stop();
    }

    auto *privateObject = static_cast<QQuickPropertyAnimationPrivate*>(QObjectPrivate::get(this));

    if (m_pinpong && m_originalStart) {
        m_originalLoop = privateObject->loopCount;
        m_currentLoop = 0;
        privateObject->loopCount = 1;
        privateObject->animationInstance->setLoopCount(1);
        m_originalStart = false;
        m_reversed = false;
    }
}

static void swapStartEnd(QQuickPropertyAnimationPrivate *privateObject)
{
    std::swap(privateObject->to, privateObject->from);
}

void QQuickTimelineAnimation::handleStopped()
{
    if (!m_pinpong) {
        emit finished();
        return;
    }

    auto *privateObject = static_cast<QQuickPropertyAnimationPrivate*>(QObjectPrivate::get(this));

    if (m_reversed)
        m_currentLoop++;

    if (!(privateObject->animationInstance->currentTime() < privateObject->duration)
            && (m_currentLoop < m_originalLoop
                || m_originalLoop == -1)) {
        swapStartEnd(privateObject);

        m_reversed = !m_reversed;
        QQuickTimelineAnimation::start();

    } else {
        if (m_reversed)
            swapStartEnd(privateObject);

        m_originalStart = true;
        m_reversed = false;
        privateObject->loopCount = m_originalLoop;
        emit finished();
    }
}

QT_END_NAMESPACE
