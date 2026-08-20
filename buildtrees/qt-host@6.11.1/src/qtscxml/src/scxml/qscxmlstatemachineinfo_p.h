// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLSTATEMACHINEINFO_H
#define QSCXMLSTATEMACHINEINFO_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtScxml/qscxmlglobals.h>
#include <QtCore/qobject.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QScxmlStateMachine;
class QScxmlStateMachineInfoPrivate;

class Q_SCXML_EXPORT QScxmlStateMachineInfo: public QObject
{
    Q_OBJECT

public: // types
    typedef int StateId;
    typedef int TransitionId;

    static const StateId InvalidStateId = -1;
    static const TransitionId InvalidTransitionId = -1;

    enum StateType : int {
        InvalidState = -1,
        NormalState = 0,
        ParallelState = 1,
        FinalState = 2,
        ShallowHistoryState = 3,
        DeepHistoryState = 4
    };

    enum TransitionType : int {
        InvalidTransition = -1,
        InternalTransition = 0,
        ExternalTransition = 1,
        SyntheticTransition = 2
    };

public: // methods
    QScxmlStateMachineInfo(QScxmlStateMachine *stateMachine);

    QScxmlStateMachine *stateMachine() const;

    QList<StateId> allStates() const;
    QList<TransitionId> allTransitions() const;
    QString stateName(int stateId) const;
    StateId stateParent(StateId stateId) const;
    StateType stateType(int stateId) const;
    QList<StateId> stateChildren(StateId stateId) const;
    TransitionId initialTransition(StateId stateId) const;
    TransitionType transitionType(TransitionId transitionId) const;
    StateId transitionSource(TransitionId transitionId) const;
    QList<StateId> transitionTargets(TransitionId transitionId) const;
    QList<QString> transitionEvents(TransitionId transitionId) const;
    QList<StateId> configuration() const;

Q_SIGNALS:
    void statesEntered(const QList<QScxmlStateMachineInfo::StateId> &states);
    void statesExited(const QList<QScxmlStateMachineInfo::StateId> &states);
    void transitionsTriggered(const QList<QScxmlStateMachineInfo::TransitionId> &transitions);

private:
    Q_DECLARE_PRIVATE(QScxmlStateMachineInfo)
};

QT_END_NAMESPACE

#endif // QSCXMLSTATEMACHINEINFO_H
