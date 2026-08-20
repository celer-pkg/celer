// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSTATE_P_H
#define QSTATE_P_H

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

#include "qstate.h"
#include "private/qabstractstate_p.h"

#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qpointer.h>
#include <QtCore/qvariant.h>
#include <QtCore/private/qproperty_p.h>

QT_REQUIRE_CONFIG(statemachine);

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PROPERTIES

struct QPropertyAssignment
{
    QPropertyAssignment()
        : object(nullptr), explicitlySet(true) {}
    QPropertyAssignment(QObject *o, const QByteArray &n,
                        const QVariant &v, bool es = true)
        : object(o), propertyName(n), value(v), explicitlySet(es)
        {}

    bool objectDeleted() const { return !object; }
    void write() const { Q_ASSERT(object != nullptr); object->setProperty(propertyName, value); }
    bool hasTarget(QObject *o, const QByteArray &pn) const
    { return object == o && propertyName == pn; }

    QPointer<QObject> object;
    QByteArray propertyName;
    QVariant value;
    bool explicitlySet; // false means the property is being restored to its old value
};
Q_DECLARE_TYPEINFO(QPropertyAssignment, Q_RELOCATABLE_TYPE);

#endif // QT_NO_PROPERTIES

class QAbstractTransition;
class QHistoryState;

class QState;
class Q_STATEMACHINE_EXPORT QStatePrivate : public QAbstractStatePrivate
{
    Q_DECLARE_PUBLIC(QState)
public:
    QStatePrivate();
    ~QStatePrivate();

    static QStatePrivate *get(QState *q) { return q ? q->d_func() : nullptr; }
    static const QStatePrivate *get(const QState *q) { return q? q->d_func() : nullptr; }

    QList<QAbstractState*> childStates() const;
    QList<QHistoryState*> historyStates() const;
    QList<QAbstractTransition*> transitions() const;

    void emitFinished();
    void emitPropertiesAssigned();

    void initialStateChanged()
    {
        emit q_func()->initialStateChanged(QState::QPrivateSignal());
    }
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QStatePrivate, QAbstractState*, initialState,
                                       nullptr, &QStatePrivate::initialStateChanged);

    void errorStateChanged()
    {
        emit q_func()->errorStateChanged(QState::QPrivateSignal());
    }
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QStatePrivate, QAbstractState*, errorState,
                                       nullptr, &QStatePrivate::errorStateChanged);

    void childModeChanged()
    {
        emit q_func()->childModeChanged(QState::QPrivateSignal());
    }
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(QStatePrivate, QState::ChildMode, childMode,
                                         QState::ExclusiveStates, &QStatePrivate::childModeChanged);

    mutable bool childStatesListNeedsRefresh;
    mutable bool transitionsListNeedsRefresh;
    mutable QList<QAbstractState*> childStatesList;
    mutable QList<QAbstractTransition*> transitionsList;

#ifndef QT_NO_PROPERTIES
    QList<QPropertyAssignment> propertyAssignments;
#endif
};

QT_END_NAMESPACE

#endif
