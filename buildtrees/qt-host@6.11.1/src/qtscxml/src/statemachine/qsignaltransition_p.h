// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSIGNALTRANSITION_P_H
#define QSIGNALTRANSITION_P_H

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

QT_REQUIRE_CONFIG(statemachine);

QT_BEGIN_NAMESPACE

class QSignalTransition;
class QSignalTransitionPrivate : public QAbstractTransitionPrivate
{
    Q_DECLARE_PUBLIC(QSignalTransition)
public:
    QSignalTransitionPrivate();

    static QSignalTransitionPrivate *get(QSignalTransition *q)
    { return q->d_func(); }

    void unregister();
    void maybeRegister();

    void callOnTransition(QEvent *e) override;

    void setSenderObject(const QObject* sender)
    {
        q_func()->setSenderObject(sender);
    }
    Q_OBJECT_COMPAT_PROPERTY_WITH_ARGS(QSignalTransitionPrivate, const QObject*,
                                       senderObject, &QSignalTransitionPrivate::setSenderObject,
                                       nullptr);

    void setSignal(const QByteArray& signal)
    {
        q_func()->setSignal(signal);
    }
    Q_OBJECT_COMPAT_PROPERTY(QSignalTransitionPrivate, QByteArray,
                             signal, &QSignalTransitionPrivate::setSignal);
    int signalIndex;
    int originalSignalIndex;
};

QT_END_NAMESPACE

#endif
