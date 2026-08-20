// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "timeouttransition_p.h"

#include <QQmlInfo>
#include <QTimer>
#include <QState>

TimeoutTransition::TimeoutTransition(QState* parent)
    : QSignalTransition((m_timer = new QTimer), SIGNAL(timeout()), parent)
{
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);
}

TimeoutTransition::~TimeoutTransition()
{
    delete m_timer;
}

int TimeoutTransition::timeout() const
{
    return m_timer->interval();
}

void TimeoutTransition::setTimeout(int timeout)
{
    m_timer->setInterval(timeout);
}

QBindable<int> TimeoutTransition::bindableTimeout()
{
    return m_timer->bindableInterval();
}

void TimeoutTransition::componentComplete()
{
    QState *state = qobject_cast<QState*>(parent());
    if (!state) {
        qmlWarning(this) << "Parent needs to be a State";
        return;
    }

    connect(state, SIGNAL(entered()), m_timer, SLOT(start()));
    connect(state, SIGNAL(exited()), m_timer, SLOT(stop()));
    if (state->active())
        m_timer->start();
}

/*!
    \qmltype TimeoutTransition
    \inqmlmodule QtQml.StateMachine
    \inherits QSignalTransition
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief The TimeoutTransition type provides a transition based on a timer.

    \l {QtQml::Timer}{Timer} type can be combined with SignalTransition to enact more complex
    timeout based transitions.

    TimeoutTransition is part of \l{Qt State Machine QML Guide}{Qt State Machine QML API}

    \section1 Example Usage

    \snippet qml/statemachine/timeouttransition.qml document

    \clearfloat

    \sa StateMachine, SignalTransition, FinalState, HistoryState
*/

/*!
    \qmlproperty int TimeoutTransition::timeout

    \brief The timeout interval in milliseconds.
*/

#include "moc_timeouttransition_p.cpp"
