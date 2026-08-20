// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QMOUSEEVENTTRANSITION_H
#define QMOUSEEVENTTRANSITION_H

#include <QtStateMachine/qeventtransition.h>

QT_BEGIN_NAMESPACE

class QMouseEventTransitionPrivate;
class QPainterPath;
class Q_STATEMACHINE_EXPORT QMouseEventTransition : public QEventTransition
{
    Q_OBJECT
    Q_PROPERTY(Qt::MouseButton button READ button WRITE setButton BINDABLE bindableButton)
    Q_PROPERTY(Qt::KeyboardModifiers modifierMask READ modifierMask WRITE setModifierMask
               BINDABLE bindableModifierMask)
public:
    QMouseEventTransition(QState *sourceState = nullptr);
    QMouseEventTransition(QObject *object, QEvent::Type type,
                          Qt::MouseButton button, QState *sourceState = nullptr);
    ~QMouseEventTransition();

    Qt::MouseButton button() const;
    void setButton(Qt::MouseButton button);
    QBindable<Qt::MouseButton> bindableButton();

    Qt::KeyboardModifiers modifierMask() const;
    void setModifierMask(Qt::KeyboardModifiers modifiers);
    QBindable<Qt::KeyboardModifiers> bindableModifierMask();

    QPainterPath hitTestPath() const;
    void setHitTestPath(const QPainterPath &path);

protected:
    void onTransition(QEvent *event) override;
    bool eventTest(QEvent *event) override;

private:
    Q_DISABLE_COPY(QMouseEventTransition)
    Q_DECLARE_PRIVATE(QMouseEventTransition)
};

QT_END_NAMESPACE

#endif
