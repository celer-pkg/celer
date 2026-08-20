// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QEVENTTRANSITION_H
#define QEVENTTRANSITION_H

#include <QtCore/qcoreevent.h>
#include <QtStateMachine/qabstracttransition.h>

QT_BEGIN_NAMESPACE

class QEventTransitionPrivate;
class Q_STATEMACHINE_EXPORT QEventTransition : public QAbstractTransition
{
    Q_OBJECT
    Q_PROPERTY(QObject* eventSource READ eventSource WRITE setEventSource
               BINDABLE bindableEventSource)
    Q_PROPERTY(QEvent::Type eventType READ eventType WRITE setEventType
               BINDABLE bindableEventType)
public:
    QEventTransition(QState *sourceState = nullptr);
    QEventTransition(QObject *object, QEvent::Type type, QState *sourceState = nullptr);
    ~QEventTransition();

    QObject *eventSource() const;
    void setEventSource(QObject *object);
    QBindable<QObject*> bindableEventSource();

    QEvent::Type eventType() const;
    void setEventType(QEvent::Type type);
    QBindable<QEvent::Type> bindableEventType();

protected:
    bool eventTest(QEvent *event) override;
    void onTransition(QEvent *event) override;

    bool event(QEvent *e) override;

protected:
    QEventTransition(QEventTransitionPrivate &dd, QState *parent);
    QEventTransition(QEventTransitionPrivate &dd, QObject *object,
                     QEvent::Type type, QState *parent);

private:
    Q_DISABLE_COPY(QEventTransition)
    Q_DECLARE_PRIVATE(QEventTransition)
};

QT_END_NAMESPACE

#endif
