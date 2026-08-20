// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef STATE_H
#define STATE_H

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

#include <QtCore/qproperty.h>
#include <QtStateMachine/QState>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlListProperty>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class Q_STATEMACHINEQML_EXPORT State : public QState, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QObject> children READ children
               NOTIFY childrenChanged BINDABLE bindableChildren)
    Q_CLASSINFO("DefaultProperty", "children")
    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 0)

public:
    explicit State(QState *parent = 0);

    void classBegin() override {}
    void componentComplete() override;

    QQmlListProperty<QObject> children();
    QBindable<QQmlListProperty<QObject>> bindableChildren() const;

Q_SIGNALS:
    void childrenChanged();

private:
    // See the childrenActualCalculation for the mutable explanation
    mutable ChildrenPrivate<State, ChildrenMode::StateOrTransition> m_children;
    friend ChildrenPrivate<State, ChildrenMode::StateOrTransition>;
    void childrenContentChanged();
    QQmlListProperty<QObject> childrenActualCalculation() const;
    Q_OBJECT_COMPUTED_PROPERTY(State, QQmlListProperty<QObject>, m_childrenComputedProperty,
                               &State::childrenActualCalculation);
};

QT_END_NAMESPACE

#endif
