// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QKEYEVENTTRANSITION_H
#define QKEYEVENTTRANSITION_H

#include <QtStateMachine/qeventtransition.h>

QT_BEGIN_NAMESPACE

class QKeyEventTransitionPrivate;
class Q_STATEMACHINE_EXPORT QKeyEventTransition : public QEventTransition
{
    Q_OBJECT
    Q_PROPERTY(int key READ key WRITE setKey BINDABLE bindableKey)
    Q_PROPERTY(Qt::KeyboardModifiers modifierMask READ modifierMask WRITE setModifierMask
               BINDABLE bindableModifierMask)
public:
    QKeyEventTransition(QState *sourceState = nullptr);
    QKeyEventTransition(QObject *object, QEvent::Type type, int key,
                        QState *sourceState = nullptr);
    ~QKeyEventTransition();

    int key() const;
    void setKey(int key);
    QBindable<int> bindableKey();

    Qt::KeyboardModifiers modifierMask() const;
    void setModifierMask(Qt::KeyboardModifiers modifiers);
    QBindable<Qt::KeyboardModifiers> bindableModifierMask();

protected:
    void onTransition(QEvent *event) override;
    bool eventTest(QEvent *event) override;

private:
    Q_DISABLE_COPY(QKeyEventTransition)
    Q_DECLARE_PRIVATE(QKeyEventTransition)
};

QT_END_NAMESPACE

#endif
