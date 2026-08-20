// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "statemachineloader_p.h"

#include <QtScxml/qscxmlstatemachine.h>

#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlfile.h>
#include <QtQml/qqmlinfo.h>

#include <QtCore/qfile.h>

/*!
    \qmltype StateMachineLoader
//!    \nativetype QScxmlStateMachineLoader
    \inqmlmodule QtScxml

    \brief Dynamically loads an SCXML document and instantiates the state machine.

    \since QtScxml 5.7
 */

QScxmlStateMachineLoader::QScxmlStateMachineLoader(QObject *parent)
    : QObject(parent)
    , m_implicitDataModel(nullptr)
{
}

/*!
    \qmlproperty ScxmlStateMachine StateMachineLoader::stateMachine

    The state machine instance.
 */
QT_PREPEND_NAMESPACE(QScxmlStateMachine) *QScxmlStateMachineLoader::stateMachine() const
{
    return m_stateMachine;
}

void QScxmlStateMachineLoader::setStateMachine(QScxmlStateMachine* stateMachine)
{
    if (m_stateMachine.valueBypassingBindings() != stateMachine) {
        delete m_stateMachine.valueBypassingBindings();
        m_stateMachine.setValueBypassingBindings(stateMachine);
    }
}

QBindable<QScxmlStateMachine*> QScxmlStateMachineLoader::bindableStateMachine()
{
    return &m_stateMachine;
}

/*!
    \qmlproperty url StateMachineLoader::source

    The URL of the SCXML document to load. Only synchronously accessible URLs
    are supported.
 */
QUrl QScxmlStateMachineLoader::source()
{
    return m_source;
}

void QScxmlStateMachineLoader::setSource(const QUrl &source)
{
    if (!source.isValid())
        return;

    m_source.removeBindingUnlessInWrapper();

    const QUrl oldSource = m_source.valueBypassingBindings();
    const auto *oldStateMachine = m_stateMachine.valueBypassingBindings();
    setStateMachine(nullptr);
    m_implicitDataModel = nullptr;

    if (parse(source))
        m_source.setValueBypassingBindings(source);
    else
        m_source.setValueBypassingBindings(QUrl());

    if (oldSource != m_source.valueBypassingBindings())
        m_source.notify();

    if (oldStateMachine != m_stateMachine.valueBypassingBindings())
        m_stateMachine.notify();
}

QBindable<QUrl> QScxmlStateMachineLoader::bindableSource()
{
    return &m_source;
}

QVariantMap QScxmlStateMachineLoader::initialValues() const
{
    return m_initialValues;
}

void QScxmlStateMachineLoader::setInitialValues(const QVariantMap &initialValues)
{
    m_initialValues.removeBindingUnlessInWrapper();
    if (initialValues == m_initialValues.valueBypassingBindings())
        return;

    m_initialValues.setValueBypassingBindings(initialValues);

    const auto stateMachine = m_stateMachine.valueBypassingBindings();
    if (stateMachine)
        stateMachine->setInitialValues(initialValues);
    m_initialValues.notify();
}

QBindable<QVariantMap> QScxmlStateMachineLoader::bindableInitialValues()
{
    return &m_initialValues;
}

QScxmlDataModel *QScxmlStateMachineLoader::dataModel() const
{
    return m_dataModel;
}

void QScxmlStateMachineLoader::setDataModel(QScxmlDataModel *dataModel)
{
    m_dataModel.removeBindingUnlessInWrapper();
    if (dataModel == m_dataModel.valueBypassingBindings()) {
        return;
    }
    m_dataModel.setValueBypassingBindings(dataModel);
    const auto stateMachine = m_stateMachine.valueBypassingBindings();
    if (stateMachine) {
        if (dataModel)
            stateMachine->setDataModel(dataModel);
        else
            stateMachine->setDataModel(m_implicitDataModel);
    }
    m_dataModel.notify();
}

QBindable<QScxmlDataModel*> QScxmlStateMachineLoader::bindableDataModel()
{
    return &m_dataModel;
}

bool QScxmlStateMachineLoader::parse(const QUrl &source)
{
    if (!QQmlFile::isSynchronous(source)) {
        qmlWarning(this) << QStringLiteral("Cannot open '%1' for reading: only synchronous access is supported.")
                         .arg(source.url());
        return false;
    }

    const QString fileName = QQmlFile::urlToLocalFileOrQrc(source);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qmlWarning(this) << QStringLiteral("Cannot open '%1' for reading: %2")
            .arg(fileName, file.errorString());
        return false;
    }

    auto stateMachine = QScxmlStateMachine::fromData(&file, fileName);
    stateMachine->setParent(this);
    m_implicitDataModel = stateMachine->dataModel();

    if (stateMachine->parseErrors().isEmpty()) {
        if (m_dataModel.value())
            stateMachine->setDataModel(m_dataModel.value());
        stateMachine->setInitialValues(m_initialValues.value());
        setStateMachine(stateMachine);
        // as this is deferred any pending property updates to m_dataModel and m_initialValues
        // should still occur before start().
        QMetaObject::invokeMethod(m_stateMachine.valueBypassingBindings(), "start", Qt::QueuedConnection);
        return true;
    } else {
        qmlWarning(this) << QStringLiteral("Something went wrong while parsing '%1':")
                         .arg(source.url())
                      << Qt::endl;
        const auto errors = stateMachine->parseErrors();
        for (const QScxmlError &error : errors) {
            qmlWarning(this) << error.toString();
        }
        return false;
    }
}
