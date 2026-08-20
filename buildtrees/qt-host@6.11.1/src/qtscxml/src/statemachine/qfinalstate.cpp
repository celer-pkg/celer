// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qfinalstate_p.h"

QT_BEGIN_NAMESPACE

/*!
  \class QFinalState
  \inmodule QtStateMachine

  \brief The QFinalState class provides a final state.

  \since 4.6
  \ingroup statemachine

  A final state is used to communicate that (part of) a QStateMachine has
  finished its work. When a final top-level state is entered, the state
  machine's \l{QStateMachine::finished()}{finished}() signal is emitted. In
  general, when a final substate (a child of a QState) is entered, the parent
  state's \l{QState::finished()}{finished}() signal is emitted.  QFinalState
  is part of \l{Qt State Machine Overview}{Qt State Machine Framework}.

  To use a final state, you create a QFinalState object and add a transition
  to it from another state. Example:

  \code
  QPushButton button;

  QStateMachine machine;
  QState *s1 = new QState();
  QFinalState *s2 = new QFinalState();
  s1->addTransition(&button, SIGNAL(clicked()), s2);
  machine.addState(s1);
  machine.addState(s2);

  QObject::connect(&machine, SIGNAL(finished()), QApplication::instance(), SLOT(quit()));
  machine.setInitialState(s1);
  machine.start();
  \endcode

  \sa QState::finished()
*/

QFinalStatePrivate::QFinalStatePrivate()
    : QAbstractStatePrivate(FinalState)
{
}

QFinalStatePrivate::~QFinalStatePrivate()
{
    // to prevent vtables being generated in every file that includes the private header
}

/*!
  Constructs a new QFinalState object with the given \a parent state.
*/
QFinalState::QFinalState(QState *parent)
    : QAbstractState(*new QFinalStatePrivate, parent)
{
}

/*!
  \internal
 */
QFinalState::QFinalState(QFinalStatePrivate &dd, QState *parent)
    : QAbstractState(dd, parent)
{
}


/*!
  Destroys this final state.
*/
QFinalState::~QFinalState()
{
}

/*!
  \reimp
*/
void QFinalState::onEntry(QEvent *event)
{
    Q_UNUSED(event);
}

/*!
  \reimp
*/
void QFinalState::onExit(QEvent *event)
{
    Q_UNUSED(event);
}

/*!
  \reimp
*/
bool QFinalState::event(QEvent *e)
{
    return QAbstractState::event(e);
}

QT_END_NAMESPACE

#include "moc_qfinalstate.cpp"
