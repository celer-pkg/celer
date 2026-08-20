// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qabstractstate.h"
#include "qabstractstate_p.h"
#include "qstate.h"
#include "qstate_p.h"
#include "qstatemachine.h"
#include "qstatemachine_p.h"

QT_BEGIN_NAMESPACE

/*!
  \class QAbstractState
  \inmodule QtStateMachine

  \brief The QAbstractState class is the base class of states of a QStateMachine.

  \since 4.6
  \ingroup statemachine

  The QAbstractState class is the abstract base class of states that are part
  of a QStateMachine. It defines the interface that all state objects have in
  common. QAbstractState is part of \l{Qt State Machine Overview}{Qt State Machine Framework}.

  The entered() signal is emitted when the state has been entered. The
  exited() signal is emitted when the state has been exited.

  The parentState() function returns the state's parent state. The machine()
  function returns the state machine that the state is part of.

  \section1 Subclassing

  The onEntry() function is called when the state is entered; reimplement this
  function to perform custom processing when the state is entered.

  The onExit() function is called when the state is exited; reimplement this
  function to perform custom processing when the state is exited.
*/

/*!
    \property QAbstractState::active
    \since 5.4

    \brief the active property of this state. A state is active between
    entered() and exited() signals.
*/


QAbstractStatePrivate::QAbstractStatePrivate(StateType type)
    : stateType(type), isMachine(false), active(false), parentState(nullptr)
{
}

QStateMachine *QAbstractStatePrivate::machine() const
{
    QObject *par = parent;
    while (par != nullptr) {
        if (QStateMachine *mach = qobject_cast<QStateMachine*>(par))
            return mach;
        par = par->parent();
    }
    return nullptr;
}

void QAbstractStatePrivate::callOnEntry(QEvent *e)
{
    Q_Q(QAbstractState);
    q->onEntry(e);
}

void QAbstractStatePrivate::callOnExit(QEvent *e)
{
    Q_Q(QAbstractState);
    q->onExit(e);
}

void QAbstractStatePrivate::emitEntered()
{
    Q_Q(QAbstractState);
    emit q->entered(QAbstractState::QPrivateSignal());
    active.setValue(true);
}

void QAbstractStatePrivate::emitExited()
{
    Q_Q(QAbstractState);
    active.setValue(false);
    emit q->exited(QAbstractState::QPrivateSignal());
}

/*!
  Constructs a new state with the given \a parent state.
*/
QAbstractState::QAbstractState(QState *parent)
    : QObject(*new QAbstractStatePrivate(QAbstractStatePrivate::AbstractState), parent)
{
}

/*!
  \internal
*/
QAbstractState::QAbstractState(QAbstractStatePrivate &dd, QState *parent)
    : QObject(dd, parent)
{
}

/*!
  Destroys this state.
*/
QAbstractState::~QAbstractState()
{
}

/*!
  Returns this state's parent state, or \nullptr if the state has no
  parent state.
*/
QState *QAbstractState::parentState() const
{
    Q_D(const QAbstractState);
    if (d->parentState != parent())
        d->parentState = qobject_cast<QState*>(parent());
    return d->parentState;
}

/*!
  Returns the state machine that this state is part of, or \nullptr if
  the state is not part of a state machine.
*/
QStateMachine *QAbstractState::machine() const
{
    Q_D(const QAbstractState);
    return d->machine();
}

/*!
  Returns whether this state is active.

  \sa activeChanged(bool), entered(), exited()
*/
bool QAbstractState::active() const
{
    Q_D(const QAbstractState);
    return d->active;
}

QBindable<bool> QAbstractState::bindableActive()
{
    Q_D(QAbstractState);
    return &d->active;
}

/*!
  \fn QAbstractState::onExit(QEvent *event)

  This function is called when the state is exited. The given \a event is what
  caused the state to be exited. Reimplement this function to perform custom
  processing when the state is exited.
*/

/*!
  \fn QAbstractState::onEntry(QEvent *event)

  This function is called when the state is entered. The given \a event is
  what caused the state to be entered. Reimplement this function to perform
  custom processing when the state is entered.
*/

/*!
  \fn QAbstractState::entered()

  This signal is emitted when the state has been entered (after onEntry() has
  been called).
*/

/*!
  \fn QAbstractState::exited()

  This signal is emitted when the state has been exited (after onExit() has
  been called).
*/

/*!
  \fn QAbstractState::activeChanged(bool active)
  \since 5.4

  This signal is emitted when the active property is changed with \a active as argument.

  \sa QAbstractState::active, entered(), exited()
*/

/*!
  \reimp
*/
bool QAbstractState::event(QEvent *e)
{
    return QObject::event(e);
}

QT_END_NAMESPACE

#include "moc_qabstractstate.cpp"
