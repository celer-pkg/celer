// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QQMLFINALSTATE_H
#define QQMLFINALSTATE_H

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

#include "qstatemachineqmlglobals_p.h"
#include "childrenprivate_p.h"
#include "statemachine_p.h"

#include <QtCore/private/qproperty_p.h>
#include <QtStateMachine/QFinalState>
#include <QtQml/QQmlListProperty>
#include <QtQml/qqml.h>


QT_BEGIN_NAMESPACE

class Q_STATEMACHINEQML_EXPORT FinalState : public QFinalState
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> children READ children
               NOTIFY childrenChanged BINDABLE bindableChildren)
    Q_CLASSINFO("DefaultProperty", "children")
    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 0)

public:
    explicit FinalState(QState *parent = 0);

    QQmlListProperty<QObject> children();
    QBindable<QQmlListProperty<QObject>> bindableChildren() const;

Q_SIGNALS:
    void childrenChanged();

private:
    // See the childrenActualCalculation for the mutable explanation
    mutable ChildrenPrivate<FinalState, ChildrenMode::State> m_children;
    friend ChildrenPrivate<FinalState, ChildrenMode::State>;
    void childrenContentChanged();
    QQmlListProperty<QObject> childrenActualCalculation() const;
    Q_OBJECT_COMPUTED_PROPERTY(FinalState, QQmlListProperty<QObject>, m_childrenComputedProperty,
                               &FinalState::childrenActualCalculation);
};

QT_END_NAMESPACE
#endif
