// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "invokedservices_p.h"
#include <QtScxml/qscxmlinvokableservice.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype InvokedServices
//!    \nativetype QScxmlInvokedServices
    \inqmlmodule QtScxml
    \since QtScxml 5.8

    \brief Provices access to the services invoked by state machines.

    Makes the invoked services easily accessible by their names, without
    constantly iterating through QScxmlStateMachine::invokedServices.

    The services are called from state machines via the mechanism described in
    \l{SCXML Specification - 6.4 <invoke>}.
*/

QScxmlInvokedServices::QScxmlInvokedServices(QObject *parent) : QObject(parent)
{
}

/*!
    \qmlproperty var InvokedServices::children

    The services invoked by the state machine.
*/

QVariantMap QScxmlInvokedServices::children()
{
    return m_children.value();
}

QVariantMap QScxmlInvokedServices::childrenActualCalculation() const
{
    QVariantMap ret;
    if (m_stateMachine.value()) {
        const QList<QScxmlInvokableService *> children = m_stateMachine->invokedServices();
        for (QScxmlInvokableService *service : children)
            ret.insert(service->name(), QVariant::fromValue(service));
    }
    return ret;
}

QBindable<QVariantMap> QScxmlInvokedServices::bindableChildren()
{
    return &m_children;
}

void QScxmlInvokedServices::classBegin()
{
}

/*!
    \qmlproperty ScxmlStateMachine InvokedServices::stateMachine

    The state machine that invoked the services.
*/

QScxmlStateMachine *QScxmlInvokedServices::stateMachine() const
{
    return m_stateMachine;
}

void QScxmlInvokedServices::setStateMachine(QScxmlStateMachine *stateMachine)
{
    m_stateMachine.removeBindingUnlessInWrapper();
    if (stateMachine == m_stateMachine.valueBypassingBindings())
        return;

    QObject::disconnect(m_serviceConnection);
    m_stateMachine.setValueBypassingBindings(stateMachine);

    if (stateMachine) {
        m_serviceConnection =
                QObject::connect(stateMachine,
                                 &QScxmlStateMachine::invokedServicesChanged, this, [this]() {
                                     m_children.notify();
                                     emit childrenChanged();
                                 });
    }
    m_stateMachine.notify();
    m_children.notify();
    emit childrenChanged();
}

QBindable<QScxmlStateMachine*> QScxmlInvokedServices::bindableStateMachine()
{
    return &m_stateMachine;
}

/*!
    \qmlproperty list<QtObject> InvokedServices::qmlChildren

    A list of additional QtObject types nested in this type.
*/

QQmlListProperty<QObject> QScxmlInvokedServices::qmlChildren()
{
    return QQmlListProperty<QObject>(this, &m_qmlChildren);
}

void QScxmlInvokedServices::componentComplete()
{
    if (!m_stateMachine.value())
        setStateMachine(qobject_cast<QScxmlStateMachine *>(parent()));
}

QT_END_NAMESPACE
