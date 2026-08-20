// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qeventtransition.h"
#include "qeventtransition_p.h"
#include "qstate.h"
#include "qstate_p.h"
#include "qstatemachine.h"
#include "qstatemachine_p.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

/*!
  \class QEventTransition
  \inmodule QtStateMachine

  \brief The QEventTransition class provides a QObject-specific transition for Qt events.

  \since 4.6
  \ingroup statemachine

  A QEventTransition object binds an event to a particular QObject.
  QEventTransition is part of \l{Qt State Machine Overview}{Qt State Machine Framework}.

  Example:

  \code
  QPushButton *button = ...;
  QState *s1 = ...;
  QState *s2 = ...;
  // If in s1 and the button receives an Enter event, transition to s2
  QEventTransition *enterTransition = new QEventTransition(button, QEvent::Enter);
  enterTransition->setTargetState(s2);
  s1->addTransition(enterTransition);
  // If in s2 and the button receives an Exit event, transition back to s1
  QEventTransition *leaveTransition = new QEventTransition(button, QEvent::Leave);
  leaveTransition->setTargetState(s1);
  s2->addTransition(leaveTransition);
  \endcode

  \section1 Subclassing

  When reimplementing the eventTest() function, you should first call the base
  implementation to verify that the event is a QStateMachine::WrappedEvent for
  the proper object and event type. You may then cast the event to a
  QStateMachine::WrappedEvent and get the original event by calling
  QStateMachine::WrappedEvent::event(), and perform additional checks on that
  object.

  \sa QState::addTransition()
*/

/*!
    \property QEventTransition::eventSource

    \brief the event source that this event transition is associated with
*/

/*!
    \property QEventTransition::eventType

    \brief the type of event that this event transition is associated with
*/

QEventTransitionPrivate::~QEventTransitionPrivate()
{
}

void QEventTransitionPrivate::unregister()
{
    Q_Q(QEventTransition);
    if (!registered || !machine())
        return;
    QStateMachinePrivate::get(machine())->unregisterEventTransition(q);
}

void QEventTransitionPrivate::maybeRegister()
{
    Q_Q(QEventTransition);
    if (QStateMachine *mach = machine())
        QStateMachinePrivate::get(mach)->maybeRegisterEventTransition(q);
}

/*!
  Constructs a new QEventTransition object with the given \a sourceState.
*/
QEventTransition::QEventTransition(QState *sourceState)
    : QAbstractTransition(*new QEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new QEventTransition object associated with events of the given
  \a type for the given \a object, and with the given \a sourceState.
*/
QEventTransition::QEventTransition(QObject *object, QEvent::Type type,
                                   QState *sourceState)
    : QAbstractTransition(*new QEventTransitionPrivate, sourceState)
{
    Q_D(QEventTransition);
    d->registered = false;
    d->object.setValueBypassingBindings(object);
    d->eventType.setValueBypassingBindings(type);
    d->maybeRegister();
}

/*!
  \internal
*/
QEventTransition::QEventTransition(QEventTransitionPrivate &dd, QState *parent)
    : QAbstractTransition(dd, parent)
{
}

/*!
  \internal
*/
QEventTransition::QEventTransition(QEventTransitionPrivate &dd, QObject *object,
                                   QEvent::Type type, QState *parent)
    : QAbstractTransition(dd, parent)
{
    Q_D(QEventTransition);
    d->registered = false;
    d->object.setValueBypassingBindings(object);
    d->eventType.setValueBypassingBindings(type);
    d->maybeRegister();
}

/*!
  Destroys this QObject event transition.
*/
QEventTransition::~QEventTransition()
{
}

/*!
  Returns the event type that this event transition is associated with.
*/
QEvent::Type QEventTransition::eventType() const
{
    Q_D(const QEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this event transition is associated with.
*/
void QEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QEventTransition);
    d->eventType.removeBindingUnlessInWrapper();
    if (d->eventType.valueBypassingBindings() == type)
        return;
    d->unregister();
    d->eventType.setValueBypassingBindings(type);
    d->maybeRegister();
    d->eventType.notify();
}

QBindable<QEvent::Type> QEventTransition::bindableEventType()
{
    Q_D(QEventTransition);
    return &d->eventType;
}

/*!
  Returns the event source associated with this event transition.
*/
QObject *QEventTransition::eventSource() const
{
    Q_D(const QEventTransition);
    return d->object;
}

/*!
  Sets the event source associated with this event transition to be the given
  \a object.
*/
void QEventTransition::setEventSource(QObject *object)
{
    Q_D(QEventTransition);
    d->object.removeBindingUnlessInWrapper();
    if (d->object.valueBypassingBindings() == object)
        return;
    d->unregister();
    d->object.setValueBypassingBindings(object);
    d->maybeRegister();
    d->object.notify();
}

QBindable<QObject*> QEventTransition::bindableEventSource()
{
    Q_D(QEventTransition);
    return &d->object;
}

/*!
  \reimp
*/
bool QEventTransition::eventTest(QEvent *event)
{
    Q_D(const QEventTransition);
    if (event->type() == QEvent::StateMachineWrapped) {
        QStateMachine::WrappedEvent *we = static_cast<QStateMachine::WrappedEvent*>(event);
        return (we->object() == d->object)
            && (we->event()->type() == d->eventType.value());
    }
    return false;
}

/*!
  \reimp
*/
void QEventTransition::onTransition(QEvent *event)
{
    Q_UNUSED(event);
}

/*!
  \reimp
*/
bool QEventTransition::event(QEvent *e)
{
    return QAbstractTransition::event(e);
}

QT_END_NAMESPACE

#include "moc_qeventtransition.cpp"
