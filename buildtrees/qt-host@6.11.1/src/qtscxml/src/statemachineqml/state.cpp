// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "state_p.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlInfo>

State::State(QState *parent)
    : QState(parent)
{
}

QQmlListProperty<QObject> State::childrenActualCalculation() const
{
    // Mutating accesses to m_children only happen in the QML thread,
    // so there are no thread-safety issues.
    // The engine only creates non-const instances of the class anyway
    return QQmlListProperty<QObject>(const_cast<State*>(this), &m_children,
                                     m_children.append, m_children.count, m_children.at,
                                     m_children.clear, m_children.replace, m_children.removeLast);
}

void State::componentComplete()
{
    if (this->machine() == nullptr) {
        static bool once = false;
        if (!once) {
            once = true;
            qmlWarning(this) << "No top level StateMachine found.  Nothing will run without a StateMachine.";
        }
    }
}

QQmlListProperty<QObject> State::children()
{
    return m_childrenComputedProperty;
}

void State::childrenContentChanged()
{
    m_childrenComputedProperty.notify();
    emit childrenChanged();
}

QBindable<QQmlListProperty<QObject>> State::bindableChildren() const
{
    return &m_childrenComputedProperty;
}

/*!
    \qmltype QAbstractState
    \inqmlmodule QtQml.StateMachine
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief The QAbstractState type is the base type of States of a StateMachine.

    Do not use QAbstractState directly; use State, FinalState or
    StateMachine instead.

    \sa StateMachine, State
*/

/*!
    \qmlproperty bool QAbstractState::active
    \readonly active

    The active property of this state. A state is active between
    entered() and exited() signals. This property is readonly.

    \sa entered, exited
*/

/*!
    \qmlsignal QAbstractState::entered()

    This signal is emitted when the State becomes active.

    \sa active, exited
*/

/*!
    \qmlsignal QAbstractState::exited()

    This signal is emitted when the State becomes inactive.

    \sa active, entered
*/

/*!
    \qmltype State
    \inqmlmodule QtQml.StateMachine
    \inherits QAbstractState
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief Provides a general-purpose state for StateMachine.

    State objects can have child states as well as transitions to other
    states. State is part of \l{Qt State Machine QML Guide}{Qt State Machine QML API}

    \section1 States with Child States

    The childMode property determines how child states are treated.  For
    non-parallel state groups, the initialState property must be used to
    set the initial state.  The child states are mutually exclusive states,
    and the state machine needs to know which child state to enter when the
    parent state is the target of a transition.

    The state emits the State::finished() signal when a final child state
    (FinalState) is entered.

    The errorState sets the state's error state.  The error state is the state
    that the state machine will transition to if an error is detected when
    attempting to enter the state (e.g. because no initial state has been set).

    \section1 Example Usage

    \snippet qml/statemachine/basicstate.qml document

    \clearfloat

    \sa StateMachine, FinalState
*/

/*!
    \qmlproperty enumeration State::childMode

    \brief The child mode of this state

    The default value of this property is QState.ExclusiveStates.

    This enum specifies how a state's child states are treated:
    \list
    \li QState.ExclusiveStates The child states are mutually exclusive and an initial state must be set by setting initialState property.
    \li QState.ParallelStates The child states are parallel. When the parent state is entered, all its child states are entered in parallel.
    \endlist
*/

/*!
    \qmlproperty QAbstractState State::errorState

    \brief The error state of this state.
*/

/*!
    \qmlproperty QAbstractState State::initialState

    \brief The initial state of this state (one of its child states).
*/

/*!
    \qmlsignal State::finished()

    This signal is emitted when a final child state of this state is entered.

    \sa QAbstractState::active, QAbstractState::entered, QAbstractState::exited
*/

/*!
    \qmltype HistoryState
    \inqmlmodule QtQml.StateMachine
    \inherits QAbstractState
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief The HistoryState type provides a means of returning to a previously active substate.

    A history state is a pseudo-state that represents the child state that the
    parent state was in the last time the parent state was exited.  A transition
    with a history state as its target is in fact a transition to one of the
    other child states of the parent state.
    HistoryState is part of \l{Qt State Machine QML Guide}{Qt State Machine QML API}.

    Use the defaultState property to set the state that should be entered
    if the parent state has never been entered.

    \section1 Example Usage

    \snippet qml/statemachine/historystate.qml document

    \clearfloat

    By default, a history state is shallow, meaning that it will not remember
    nested states.  This can be configured through the historyType property.

    \sa StateMachine, State
*/

/*!
    \qmlproperty QAbstractState HistoryState::defaultState

    \brief The default state of this history state.

    The default state indicates the state to transition to if the parent
    state has never been entered before.
*/

/*!
    \qmlproperty enumeration HistoryState::historyType

    \brief The type of history that this history state records.

    The default value of this property is HistoryState.ShallowHistory.

    This enum specifies the type of history that a HistoryState records.
    \list
    \li HistoryState.ShallowHistory Only the immediate child states of the
        parent state are recorded.  In this case, a transition with the history
        state as its target will end up in the immediate child state that the
        parent was in the last time it was exited.  This is the default.
    \li HistoryState.DeepHistory Nested states are recorded.  In this case
        a transition with the history state as its target will end up in the
        most deeply nested descendant state the parent was in the last time
        it was exited.
    \endlist
*/

#include "moc_state_p.cpp"
