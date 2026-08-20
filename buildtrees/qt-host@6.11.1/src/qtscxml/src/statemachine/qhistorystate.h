// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QHISTORYSTATE_H
#define QHISTORYSTATE_H

#include <QtStateMachine/qabstractstate.h>

QT_BEGIN_NAMESPACE

class QAbstractTransition;
class QHistoryStatePrivate;
class Q_STATEMACHINE_EXPORT QHistoryState : public QAbstractState
{
    Q_OBJECT
    Q_PROPERTY(QAbstractState* defaultState READ defaultState WRITE setDefaultState NOTIFY defaultStateChanged)
    Q_PROPERTY(QAbstractTransition* defaultTransition READ defaultTransition
               WRITE setDefaultTransition NOTIFY defaultTransitionChanged
               BINDABLE bindableDefaultTransition)
    Q_PROPERTY(HistoryType historyType READ historyType WRITE setHistoryType
               NOTIFY historyTypeChanged BINDABLE bindableHistoryType)
public:
    enum HistoryType {
        ShallowHistory,
        DeepHistory
    };
    Q_ENUM(HistoryType)

    QHistoryState(QState *parent = nullptr);
    QHistoryState(HistoryType type, QState *parent = nullptr);
    ~QHistoryState();

    QAbstractTransition *defaultTransition() const;
    void setDefaultTransition(QAbstractTransition *transition);
    QBindable<QAbstractTransition*> bindableDefaultTransition();

    QAbstractState *defaultState() const;
    void setDefaultState(QAbstractState *state);

    HistoryType historyType() const;
    void setHistoryType(HistoryType type);
    QBindable<QHistoryState::HistoryType> bindableHistoryType();

Q_SIGNALS:
    void defaultTransitionChanged(QPrivateSignal);
    void defaultStateChanged(QPrivateSignal);
    void historyTypeChanged(QPrivateSignal);

protected:
    void onEntry(QEvent *event) override;
    void onExit(QEvent *event) override;

    bool event(QEvent *e) override;

private:
    Q_DISABLE_COPY(QHistoryState)
    Q_DECLARE_PRIVATE(QHistoryState)
};

QT_END_NAMESPACE

#endif
