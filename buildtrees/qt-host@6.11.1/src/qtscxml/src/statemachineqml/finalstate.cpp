// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "finalstate_p.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlInfo>

FinalState::FinalState(QState *parent)
    : QFinalState(parent)
{
}

QQmlListProperty<QObject> FinalState::childrenActualCalculation() const
{
    // Mutating accesses to m_children only happen in the QML thread,
    // so there are no thread-safety issues.
    // The engine only creates non-const instances of the class anyway
    return QQmlListProperty<QObject>(const_cast<FinalState*>(this), &m_children,
                                     m_children.append, m_children.count, m_children.at,
                                     m_children.clear, m_children.replace, m_children.removeLast);
}

QQmlListProperty<QObject> FinalState::children()
{
    return m_childrenComputedProperty;
}

void FinalState::childrenContentChanged()
{
    m_childrenComputedProperty.notify();
    emit childrenChanged();
}

QBindable<QQmlListProperty<QObject>> FinalState::bindableChildren() const
{
    return &m_childrenComputedProperty;
}

/*!
    \qmltype FinalState
    \inqmlmodule QtQml.StateMachine
    \inherits QAbstractState
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief Provides a final state.


    A final state is used to communicate that (part of) a StateMachine has
    finished its work.  When a final top-level state is entered, the state
    machine's \l{State::finished}{finished}() signal is emitted. In
    general, when a final substate (a child of a State) is entered, the parent
    state's \l{State::finished}{finished}() signal is emitted.  FinalState
    is part of \l{Qt State Machine QML Guide}{Qt State Machine QML API}

    To use a final state, you create a FinalState object and add a transition
    to it from another state.

    \section1 Example Usage

    \snippet qml/statemachine/finalstate.qml document

    \clearfloat

    \sa StateMachine, State
*/
