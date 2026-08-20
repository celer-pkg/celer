// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

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

#include <QtCore/private/qproperty_p.h>
#include <QtStateMachine/QStateMachine>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlListProperty>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class Q_STATEMACHINEQML_EXPORT StateMachine : public QStateMachine, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QObject> children READ children
               NOTIFY childrenChanged BINDABLE bindableChildren)

    // Override to delay execution after componentComplete()
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY qmlRunningChanged)

    Q_CLASSINFO("DefaultProperty", "children")
    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 0)

public:
    explicit StateMachine(QObject *parent = 0);

    void classBegin() override {}
    void componentComplete() override;
    QQmlListProperty<QObject> children();
    QBindable<QQmlListProperty<QObject>> bindableChildren() const;

    bool isRunning() const;
    void setRunning(bool running);

private Q_SLOTS:
    void checkChildMode();

Q_SIGNALS:
    void childrenChanged();
    /*!
     * \internal
     */
    void qmlRunningChanged();

private:
    // See the childrenActualCalculation for the mutable explanation
    mutable ChildrenPrivate<StateMachine, ChildrenMode::StateOrTransition> m_children;
    friend ChildrenPrivate<StateMachine, ChildrenMode::StateOrTransition>;
    void childrenContentChanged();
    QQmlListProperty<QObject> childrenActualCalculation() const;
    Q_OBJECT_COMPUTED_PROPERTY(StateMachine, QQmlListProperty<QObject>, m_childrenComputedProperty,
                               &StateMachine::childrenActualCalculation);
    bool m_completed;
    bool m_running;
};

QT_END_NAMESPACE

#endif
