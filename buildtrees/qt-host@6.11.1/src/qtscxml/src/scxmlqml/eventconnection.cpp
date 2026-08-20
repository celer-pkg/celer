// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "eventconnection_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype EventConnection
//!    \nativetype QScxmlEventConnection
    \inqmlmodule QtScxml
    \since QtScxml 5.8

    \brief Connects to events sent out by state machines.

    To receive a notification when a state machine sends out an event, a
    connection can be created to the corresponding signal.
*/

/*!
    \qmlproperty stringlist EventConnection::events

    The list of SCXML event specifiers that describe the events to listen for.

    Even though spaces are allowed in event specifications in SCXML documents,
    they are not allowed in this list. However, the list can contain multiple
    specifiers, to the same effect.
*/

/*!
    \qmlproperty ScxmlStateMachine EventConnection::stateMachine

    The state machine that sends out the event.
*/

/*!
    \qmlsignal EventConnection::occurred(event)

    This signal is emitted when the SCXML event \a event occurs.

    \sa QScxmlEvent
*/


QScxmlEventConnection::QScxmlEventConnection(QObject *parent) :
    QObject(parent)
{
}

QScxmlEventConnection::~QScxmlEventConnection()
    = default;

QStringList QScxmlEventConnection::events() const
{
    return m_events;
}

void QScxmlEventConnection::setEvents(const QStringList &events)
{
    m_events.removeBindingUnlessInWrapper();
    if (events == m_events.valueBypassingBindings()) {
        return;
    }
    m_events.setValueBypassingBindings(events);
    doConnect();
    m_events.notify();
}

QBindable<QStringList> QScxmlEventConnection::bindableEvents()
{
    return &m_events;
}

QScxmlStateMachine *QScxmlEventConnection::stateMachine() const
{
    return m_stateMachine;
}

void QScxmlEventConnection::setStateMachine(QScxmlStateMachine *stateMachine)
{
    m_stateMachine.removeBindingUnlessInWrapper();
    if (stateMachine == m_stateMachine.valueBypassingBindings())
        return;
    m_stateMachine.setValueBypassingBindings(stateMachine);
    doConnect();
    m_stateMachine.notify();
}

QBindable<QScxmlStateMachine*> QScxmlEventConnection::bindableStateMachine()
{
    return &m_stateMachine;
}

void QScxmlEventConnection::doConnect()
{
    for (QMetaObject::Connection &connection : m_connections)
        disconnect(connection);
    m_connections.clear();
    const auto stateMachine = m_stateMachine.valueBypassingBindings();
    if (stateMachine) {
        const auto events = m_events.valueBypassingBindings();
        for (const QString &event : events) {
            m_connections.append(stateMachine->connectToEvent(event, this,
                                                              &QScxmlEventConnection::occurred));
        }
    }
}

void QScxmlEventConnection::classBegin()
{
}

void QScxmlEventConnection::componentComplete()
{
    auto parentStateMachine = qobject_cast<QScxmlStateMachine *>(parent());
    if (!m_stateMachine.value() && parentStateMachine)
        setStateMachine(parentStateMachine);
}

QT_END_NAMESPACE
