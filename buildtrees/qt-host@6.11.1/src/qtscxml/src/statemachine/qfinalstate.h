// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QFINALSTATE_H
#define QFINALSTATE_H

#include <QtStateMachine/qabstractstate.h>

QT_BEGIN_NAMESPACE

class QFinalStatePrivate;
class Q_STATEMACHINE_EXPORT QFinalState : public QAbstractState
{
    Q_OBJECT
public:
    QFinalState(QState *parent = nullptr);
    ~QFinalState();

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

    bool event(QEvent *e) override;

protected:
    explicit QFinalState(QFinalStatePrivate &dd, QState *parent);

private:
    Q_DISABLE_COPY(QFinalState)
    Q_DECLARE_PRIVATE(QFinalState)
};

QT_END_NAMESPACE

#endif
