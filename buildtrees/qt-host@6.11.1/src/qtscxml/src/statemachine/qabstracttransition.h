// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QABSTRACTTRANSITION_H
#define QABSTRACTTRANSITION_H

#include <QtCore/qlist.h>
#include <QtCore/qobject.h>

#include <QtStateMachine/qstatemachineglobal.h>

QT_BEGIN_NAMESPACE

class QEvent;
class QAbstractState;
class QState;
class QStateMachine;

#if QT_CONFIG(animation)
class QAbstractAnimation;
#endif

class QAbstractTransitionPrivate;
class Q_STATEMACHINE_EXPORT QAbstractTransition : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QState* sourceState READ sourceState)
    Q_PROPERTY(QAbstractState* targetState READ targetState WRITE setTargetState NOTIFY targetStateChanged)
    Q_PROPERTY(QList<QAbstractState*> targetStates READ targetStates WRITE setTargetStates NOTIFY targetStatesChanged)
    Q_PROPERTY(TransitionType transitionType READ transitionType WRITE setTransitionType
               BINDABLE bindableTransitionType REVISION(1, 1))
public:
    enum TransitionType {
        ExternalTransition,
        InternalTransition
    };
    Q_ENUM(TransitionType)

    QAbstractTransition(QState *sourceState = nullptr);
    virtual ~QAbstractTransition();

    QState *sourceState() const;
    QAbstractState *targetState() const;
    void setTargetState(QAbstractState* target);
    QList<QAbstractState*> targetStates() const;
    void setTargetStates(const QList<QAbstractState*> &targets);

    TransitionType transitionType() const;
    void setTransitionType(TransitionType type);
    QBindable<QAbstractTransition::TransitionType> bindableTransitionType();

    QStateMachine *machine() const;

#if QT_CONFIG(animation)
    void addAnimation(QAbstractAnimation *animation);
    void removeAnimation(QAbstractAnimation *animation);
    QList<QAbstractAnimation*> animations() const;
#endif

Q_SIGNALS:
    void triggered(QPrivateSignal);
    void targetStateChanged(QPrivateSignal);
    void targetStatesChanged(QPrivateSignal);

protected:
    virtual bool eventTest(QEvent *event) = 0;

    virtual void onTransition(QEvent *event) = 0;

    bool event(QEvent *e) override;

protected:
    QAbstractTransition(QAbstractTransitionPrivate &dd, QState *parent);

private:
    Q_DISABLE_COPY(QAbstractTransition)
    Q_DECLARE_PRIVATE(QAbstractTransition)
};

QT_END_NAMESPACE

#endif
