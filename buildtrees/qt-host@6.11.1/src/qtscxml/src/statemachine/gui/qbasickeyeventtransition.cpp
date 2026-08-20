// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qbasickeyeventtransition_p.h"

#include <QtGui/qevent.h>

#include <private/qabstracttransition_p.h>

QT_BEGIN_NAMESPACE

/*!
  \internal
  \class QBasicKeyEventTransition
  \since 4.6
  \ingroup statemachine

  \brief The QBasicKeyEventTransition class provides a transition for Qt key events.
*/

class QBasicKeyEventTransitionPrivate : public QAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QBasicKeyEventTransition)
public:
    QBasicKeyEventTransitionPrivate() = default;

    static QBasicKeyEventTransitionPrivate *get(QBasicKeyEventTransition *q);

    QEvent::Type eventType = QEvent::None;

    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QBasicKeyEventTransitionPrivate, int, key, 0);
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QBasicKeyEventTransitionPrivate, Qt::KeyboardModifiers,
                                         modifierMask, Qt::NoModifier);
};

QBasicKeyEventTransitionPrivate *QBasicKeyEventTransitionPrivate::get(QBasicKeyEventTransition *q)
{
    return q->d_func();
}

/*!
  Constructs a new key event transition with the given \a sourceState.
*/
QBasicKeyEventTransition::QBasicKeyEventTransition(QState *sourceState)
    : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new event transition for events of the given \a type for the
  given \a key, with the given \a sourceState.
*/
QBasicKeyEventTransition::QBasicKeyEventTransition(QEvent::Type type, int key,
                                                   QState *sourceState)
    : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
    Q_D(QBasicKeyEventTransition);
    d->eventType = type;
    d->key = key;
}

/*!
  Constructs a new event transition for events of the given \a type for the
  given \a key, with the given \a modifierMask and \a sourceState.
*/
QBasicKeyEventTransition::QBasicKeyEventTransition(QEvent::Type type, int key,
                                                   Qt::KeyboardModifiers modifierMask,
                                                   QState *sourceState)
    : QAbstractTransition(*new QBasicKeyEventTransitionPrivate, sourceState)
{
    Q_D(QBasicKeyEventTransition);
    d->eventType = type;
    d->key = key;
    d->modifierMask = modifierMask;
}

/*!
  Destroys this event transition.
*/
QBasicKeyEventTransition::~QBasicKeyEventTransition()
{
}

/*!
  Returns the event type that this key event transition is associated with.
*/
QEvent::Type QBasicKeyEventTransition::eventType() const
{
    Q_D(const QBasicKeyEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this key event transition is associated with.
*/
void QBasicKeyEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QBasicKeyEventTransition);
    d->eventType = type;
}

/*!
  Returns the key that this key event transition checks for.
*/
int QBasicKeyEventTransition::key() const
{
    Q_D(const QBasicKeyEventTransition);
    return d->key;
}

/*!
  Sets the key that this key event transition will check for.
*/
void QBasicKeyEventTransition::setKey(int key)
{
    Q_D(QBasicKeyEventTransition);
    d->key = key;
}

QBindable<int> QBasicKeyEventTransition::bindableKey()
{
    Q_D(QBasicKeyEventTransition);
    return &d->key;
}

/*!
  Returns the keyboard modifier mask that this key event transition checks
  for.
*/
Qt::KeyboardModifiers QBasicKeyEventTransition::modifierMask() const
{
    Q_D(const QBasicKeyEventTransition);
    return d->modifierMask;
}

/*!
  Sets the keyboard modifier mask that this key event transition will check
  for.
*/
void QBasicKeyEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
    Q_D(QBasicKeyEventTransition);
    d->modifierMask = modifierMask;
}

QBindable<Qt::KeyboardModifiers> QBasicKeyEventTransition::bindableModifierMask()
{
    Q_D(QBasicKeyEventTransition);
    return &d->modifierMask;
}

/*!
  \reimp
*/
bool QBasicKeyEventTransition::eventTest(QEvent *event)
{
    Q_D(const QBasicKeyEventTransition);
    if (event->type() == d->eventType) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        return (ke->key() == d->key)
            && ((ke->modifiers() & d->modifierMask.value()) == d->modifierMask.value());
    }
    return false;
}

/*!
  \reimp
*/
void QBasicKeyEventTransition::onTransition(QEvent *)
{
}

QT_END_NAMESPACE

#include "moc_qbasickeyeventtransition_p.cpp"
