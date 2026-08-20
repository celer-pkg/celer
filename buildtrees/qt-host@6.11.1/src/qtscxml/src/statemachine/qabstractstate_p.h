// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QABSTRACTSTATE_P_H
#define QABSTRACTSTATE_P_H

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

#include <private/qobject_p.h>
#include <QtStateMachine/qabstractstate.h>
#include <private/qstatemachineglobal_p.h>

QT_REQUIRE_CONFIG(statemachine);

QT_BEGIN_NAMESPACE

class QStateMachine;

class QState;
class QAbstractStatePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractState)

public:
    enum StateType {
        AbstractState,
        StandardState,
        FinalState,
        HistoryState
    };

    QAbstractStatePrivate(StateType type);

    static QAbstractStatePrivate *get(QAbstractState *q)
    { return q->d_func(); }
    static const QAbstractStatePrivate *get(const QAbstractState *q)
    { return q->d_func(); }

    QStateMachine *machine() const;

    void callOnEntry(QEvent *e);
    void callOnExit(QEvent *e);

    void emitEntered();
    void emitExited();

    quint16 stateType;
    bool isMachine;

    void activeChanged()
    {
        emit q_func()->activeChanged(active.value());
    }
    Q_OBJECT_BINDABLE_PROPERTY(QAbstractStatePrivate, bool, active,
                               &QAbstractStatePrivate::activeChanged);

    mutable QState *parentState;
};

QT_END_NAMESPACE

#endif // QABSTRACTSTATE_P_H
