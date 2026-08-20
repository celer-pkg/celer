// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qbasicmouseeventtransition_p.h"

#include <QtGui/qevent.h>
#include <QtGui/qpainterpath.h>

#include <private/qabstracttransition_p.h>

QT_BEGIN_NAMESPACE

/*!
  \internal
  \class QBasicMouseEventTransition
  \since 4.6
  \ingroup statemachine

  \brief The QBasicMouseEventTransition class provides a transition for Qt mouse events.
*/

class QBasicMouseEventTransitionPrivate : public QAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QBasicMouseEventTransition)
public:
    QBasicMouseEventTransitionPrivate() = default;

    static QBasicMouseEventTransitionPrivate *get(QBasicMouseEventTransition *q);

    QEvent::Type eventType = QEvent::None;
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QBasicMouseEventTransitionPrivate, Qt::MouseButton,
                                         button, Qt::NoButton);
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QBasicMouseEventTransitionPrivate, Qt::KeyboardModifiers,
                                          modifierMask, Qt::NoModifier);
    QPainterPath path;
};

QBasicMouseEventTransitionPrivate *QBasicMouseEventTransitionPrivate::get(QBasicMouseEventTransition *q)
{
    return q->d_func();
}

/*!
  Constructs a new mouse event transition with the given \a sourceState.
*/
QBasicMouseEventTransition::QBasicMouseEventTransition(QState *sourceState)
    : QAbstractTransition(*new QBasicMouseEventTransitionPrivate, sourceState)
{
}

/*!
  Constructs a new mouse event transition for events of the given \a type.
*/
QBasicMouseEventTransition::QBasicMouseEventTransition(QEvent::Type type,
                                                       Qt::MouseButton button,
                                                       QState *sourceState)
    : QAbstractTransition(*new QBasicMouseEventTransitionPrivate, sourceState)
{
    Q_D(QBasicMouseEventTransition);
    d->eventType = type;
    d->button = button;
}

/*!
  Destroys this mouse event transition.
*/
QBasicMouseEventTransition::~QBasicMouseEventTransition()
{
}

/*!
  Returns the event type that this mouse event transition is associated with.
*/
QEvent::Type QBasicMouseEventTransition::eventType() const
{
    Q_D(const QBasicMouseEventTransition);
    return d->eventType;
}

/*!
  Sets the event \a type that this mouse event transition is associated with.
*/
void QBasicMouseEventTransition::setEventType(QEvent::Type type)
{
    Q_D(QBasicMouseEventTransition);
    d->eventType = type;
}

/*!
  Returns the button that this mouse event transition checks for.
*/
Qt::MouseButton QBasicMouseEventTransition::button() const
{
    Q_D(const QBasicMouseEventTransition);
    return d->button;
}

/*!
  Sets the button that this mouse event transition will check for.
*/
void QBasicMouseEventTransition::setButton(Qt::MouseButton button)
{
    Q_D(QBasicMouseEventTransition);
    d->button = button;
}

QBindable<Qt::MouseButton> QBasicMouseEventTransition::bindableButton()
{
    Q_D(QBasicMouseEventTransition);
    return &d->button;
}

/*!
  Returns the keyboard modifier mask that this mouse event transition checks
  for.
*/
Qt::KeyboardModifiers QBasicMouseEventTransition::modifierMask() const
{
    Q_D(const QBasicMouseEventTransition);
    return d->modifierMask;
}

/*!
  Sets the keyboard modifier mask that this mouse event transition will check
  for.
*/
void QBasicMouseEventTransition::setModifierMask(Qt::KeyboardModifiers modifierMask)
{
    Q_D(QBasicMouseEventTransition);
    d->modifierMask = modifierMask;
}

QBindable<Qt::KeyboardModifiers> QBasicMouseEventTransition::bindableModifierMask()
{
    Q_D(QBasicMouseEventTransition);
    return &d->modifierMask;
}


/*!
  Returns the hit test path for this mouse event transition.
*/
QPainterPath QBasicMouseEventTransition::hitTestPath() const
{
    Q_D(const QBasicMouseEventTransition);
    return d->path;
}

/*!
  Sets the hit test path for this mouse event transition.
*/
void QBasicMouseEventTransition::setHitTestPath(const QPainterPath &path)
{
    Q_D(QBasicMouseEventTransition);
    d->path = path;
}

/*!
  \reimp
*/
bool QBasicMouseEventTransition::eventTest(QEvent *event)
{
    Q_D(const QBasicMouseEventTransition);
    if (event->type() == d->eventType) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        return (me->button() == d->button)
            && ((me->modifiers() & d->modifierMask.value()) == d->modifierMask.value())
            && (d->path.isEmpty() || d->path.contains(me->position().toPoint()));
    }
    return false;
}

/*!
  \reimp
*/
void QBasicMouseEventTransition::onTransition(QEvent *)
{
}

QT_END_NAMESPACE

#include "moc_qbasicmouseeventtransition_p.cpp"
