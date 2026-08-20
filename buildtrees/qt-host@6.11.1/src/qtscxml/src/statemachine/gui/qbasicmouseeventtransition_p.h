// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QBASICMOUSEEVENTTRANSITION_P_H
#define QBASICMOUSEEVENTTRANSITION_P_H

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

class QPainterPath;

class QBasicMouseEventTransitionPrivate;
class Q_AUTOTEST_EXPORT QBasicMouseEventTransition : public QAbstractTransition
{
    Q_OBJECT
public:
    QBasicMouseEventTransition(QState *sourceState = nullptr);
    QBasicMouseEventTransition(QEvent::Type type, Qt::MouseButton button,
                               QState *sourceState = nullptr);
    ~QBasicMouseEventTransition();

    QEvent::Type eventType() const;
    void setEventType(QEvent::Type type);

    Qt::MouseButton button() const;
    void setButton(Qt::MouseButton button);
    QBindable<Qt::MouseButton> bindableButton();

    Qt::KeyboardModifiers modifierMask() const;
    void setModifierMask(Qt::KeyboardModifiers modifiers);
    QBindable<Qt::KeyboardModifiers> bindableModifierMask();

    QPainterPath hitTestPath() const;
    void setHitTestPath(const QPainterPath &path);

protected:
    bool eventTest(QEvent *event) override;
    void onTransition(QEvent *) override;

private:
    Q_DISABLE_COPY_MOVE(QBasicMouseEventTransition)
    Q_DECLARE_PRIVATE(QBasicMouseEventTransition)
};

QT_END_NAMESPACE

#endif
