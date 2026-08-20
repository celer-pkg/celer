// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QEVENTTRANSITION_P_H
#define QEVENTTRANSITION_P_H

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

#include "private/qabstracttransition_p.h"
#include <QtCore/private/qproperty_p.h>

QT_REQUIRE_CONFIG(qeventtransition);

QT_BEGIN_NAMESPACE

class QEventTransition;
class Q_STATEMACHINE_EXPORT QEventTransitionPrivate : public QAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QEventTransition)
public:
    QEventTransitionPrivate() = default;
    ~QEventTransitionPrivate();

    static QEventTransitionPrivate *get(QEventTransition *q)
    { return q->d_func(); }

    void unregister();
    void maybeRegister();

    void setEventSource(QObject* eventSource)
    {
        q_func()->setEventSource(eventSource);
    }
    Q_OBJECT_COMPAT_PROPERTY_WITH_ARGS(QEventTransitionPrivate, QObject*, object,
                                       &QEventTransitionPrivate::setEventSource, nullptr);

    void setEventType(QEvent::Type eventType)
    {
        q_func()->setEventType(eventType);
    }
    Q_OBJECT_COMPAT_PROPERTY_WITH_ARGS(QEventTransitionPrivate, QEvent::Type, eventType,
                                       &QEventTransitionPrivate::setEventType, QEvent::None);
    bool registered = false;
};

QT_END_NAMESPACE

#endif
