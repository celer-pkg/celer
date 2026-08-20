// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QABSTRACTTRANSITION_P_H
#define QABSTRACTTRANSITION_P_H

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

#include <QtCore/qlist.h>
#include <QtCore/qpointer.h>
#include <QtCore/qsharedpointer.h>
#include <private/qstatemachineglobal_p.h>

QT_REQUIRE_CONFIG(statemachine);

QT_BEGIN_NAMESPACE

class QAbstractState;
class QState;
class QStateMachine;

class QAbstractTransition;
class Q_STATEMACHINE_EXPORT QAbstractTransitionPrivate
    : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractTransition)
public:
    QAbstractTransitionPrivate() = default;

    static QAbstractTransitionPrivate *get(QAbstractTransition *q)
    { return q->d_func(); }

    bool callEventTest(QEvent *e);
    virtual void callOnTransition(QEvent *e);
    QState *sourceState() const;
    QStateMachine *machine() const;
    void emitTriggered();

    QList<QPointer<QAbstractState>> targetStates;

    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QAbstractTransitionPrivate,
                                         QAbstractTransition::TransitionType, transitionType,
                                         QAbstractTransition::ExternalTransition);

#if QT_CONFIG(animation)
    QList<QAbstractAnimation*> animations;
#endif
};

QT_END_NAMESPACE

#endif
