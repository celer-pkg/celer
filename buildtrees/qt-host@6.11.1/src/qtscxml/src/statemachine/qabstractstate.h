// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QABSTRACTSTATE_H
#define QABSTRACTSTATE_H

#include <QtCore/qobject.h>
#include <QtStateMachine/qstatemachineglobal.h>

QT_BEGIN_NAMESPACE

class QState;
class QStateMachine;

class QAbstractStatePrivate;
class Q_STATEMACHINE_EXPORT QAbstractState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY activeChanged BINDABLE bindableActive)
public:
    ~QAbstractState();

    QState *parentState() const;
    QStateMachine *machine() const;

    bool active() const;
    QBindable<bool> bindableActive();

Q_SIGNALS:
    void entered(QPrivateSignal);
    void exited(QPrivateSignal);
    void activeChanged(bool active);

protected:
    QAbstractState(QState *parent = nullptr);

    virtual void onEntry(QEvent *event) = 0;
    virtual void onExit(QEvent *event) = 0;

    bool event(QEvent *e) override;

protected:
    QAbstractState(QAbstractStatePrivate &dd, QState *parent);

private:
    Q_DISABLE_COPY(QAbstractState)
    Q_DECLARE_PRIVATE(QAbstractState)
};

QT_END_NAMESPACE

#endif
