// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QBASICKEYEVENTTRANSITION_P_H
#define QBASICKEYEVENTTRANSITION_P_H

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

#include <QtGui/qevent.h>
#include <QtStateMachine/qabstracttransition.h>
#include <QtCore/private/qglobal_p.h>
#include <private/qstatemachineglobal_p.h>

QT_REQUIRE_CONFIG(qeventtransition);

QT_BEGIN_NAMESPACE

class QBasicKeyEventTransitionPrivate;
class Q_AUTOTEST_EXPORT QBasicKeyEventTransition : public QAbstractTransition
{
    Q_OBJECT
public:
    QBasicKeyEventTransition(QState *sourceState = nullptr);
    QBasicKeyEventTransition(QEvent::Type type, int key, QState *sourceState = nullptr);
    QBasicKeyEventTransition(QEvent::Type type, int key,
                             Qt::KeyboardModifiers modifierMask,
                             QState *sourceState = nullptr);
    ~QBasicKeyEventTransition();

    QEvent::Type eventType() const;
    void setEventType(QEvent::Type type);

    int key() const;
    void setKey(int key);
    QBindable<int> bindableKey();

    Qt::KeyboardModifiers modifierMask() const;
    void setModifierMask(Qt::KeyboardModifiers modifiers);
    QBindable<Qt::KeyboardModifiers> bindableModifierMask();

protected:
    bool eventTest(QEvent *event) override;
    void onTransition(QEvent *) override;

private:
    Q_DISABLE_COPY_MOVE(QBasicKeyEventTransition)
    Q_DECLARE_PRIVATE(QBasicKeyEventTransition)
};

QT_END_NAMESPACE

#endif
