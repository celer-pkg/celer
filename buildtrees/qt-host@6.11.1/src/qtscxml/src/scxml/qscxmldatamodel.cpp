// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:execute-external-code

#include "qscxmldatamodel_p.h"
#include "qscxmlnulldatamodel.h"
#include "qscxmlstatemachine_p.h"

#include <QtCore/private/qfactoryloader_p.h>
#include "qscxmldatamodelplugin_p.h"

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
        ("org.qt-project.qt.scxml.datamodel.plugin",
         QStringLiteral("/scxmldatamodel")))

/*!
  \class QScxmlDataModel::ForeachLoopBody
  \brief The ForeachLoopBody class represents a function to be executed on
  each iteration of an SCXML foreach loop.
  \since 5.8
  \inmodule QtScxml
 */

/*!
  Creates a new foreach loop body.
 */
QScxmlDataModel::ForeachLoopBody::ForeachLoopBody()
{}
/*!
  Destroys a foreach loop body.
 */
QScxmlDataModel::ForeachLoopBody::~ForeachLoopBody()
{}

/*!
  \fn QScxmlDataModel::ForeachLoopBody::run(bool *ok)

  This function is executed on each iteration. If the execution fails, \a ok is
  set to \c false, otherwise it is set to \c true.
 */

/*!
 * \class QScxmlDataModel
 * \brief The QScxmlDataModel class is the data model base class for a Qt SCXML
 * state machine.
 * \since 5.7
 * \inmodule QtScxml
 *
 * SCXML data models are described in
 * \l {SCXML Specification - 5 Data Model and Data Manipulation}. For more
 * information about supported data models, see \l {SCXML Compliance}.
 *
 * One data model can only belong to one state machine.
 *
 * \sa QScxmlStateMachine QScxmlCppDataModel QScxmlNullDataModel
 */

/*!
  \property QScxmlDataModel::stateMachine

  \brief The state machine this data model belongs to.

  A data model can only belong to a single state machine and a state machine
  can only have one data model. This relation needs to be set up before the
  state machine is started. Setting this property on a data model will
  automatically set the corresponding \c dataModel property on the
  \a stateMachine.
*/

/*!
 * Creates a new data model, with the parent object \a parent.
 */
QScxmlDataModel::QScxmlDataModel(QObject *parent)
    : QObject(*(new QScxmlDataModelPrivate), parent)
{
}

/*!
  Creates a new data model from the private object \a dd, with the parent
  object \a parent.
 */
QScxmlDataModel::QScxmlDataModel(QScxmlDataModelPrivate &dd, QObject *parent) :
    QObject(dd, parent)
{
}

/*!
 * Sets the state machine this model belongs to to \a stateMachine. There is a
 * 1:1 relation between state machines and models. After setting the state
 * machine once you cannot change it anymore. Any further attempts to set the
 * state machine using this method will be ignored.
 */
void QScxmlDataModel::setStateMachine(QScxmlStateMachine *stateMachine)
{
    Q_D(QScxmlDataModel);

    if (d->m_stateMachine.valueBypassingBindings() == nullptr && stateMachine != nullptr) {
        // the binding is removed only on the first valid set
        // as the later attempts are ignored
        d->m_stateMachine.removeBindingUnlessInWrapper();
        d->m_stateMachine.setValueBypassingBindings(stateMachine);
        stateMachine->setDataModel(this);
        d->m_stateMachine.notify();
    }
}

/*!
 * Returns the state machine associated with the data model.
 */
QScxmlStateMachine *QScxmlDataModel::stateMachine() const
{
    Q_D(const QScxmlDataModel);
    return d->m_stateMachine;
}

QBindable<QScxmlStateMachine*> QScxmlDataModel::bindableStateMachine()
{
    Q_D(QScxmlDataModel);
    return &d->m_stateMachine;
}

/*!
 * Creates a data model from a plugin specified by a \a pluginKey.
 */
QScxmlDataModel *QScxmlDataModel::createScxmlDataModel(const QString& pluginKey)
{
    QScxmlDataModel *model = nullptr;

    int pluginIndex = loader()->indexOf(pluginKey);

    if (QObject *object = loader()->instance(pluginIndex)) {
        if (auto *plugin = qobject_cast<QScxmlDataModelPlugin *>(object)) {
            model = plugin->createScxmlDataModel();
            if (!model)
                qWarning() << pluginKey << " data model was not instantiated, createScxmlDataModel() returned null.";

        } else {
            qWarning() << "plugin object for" << pluginKey << "is not a QScxmlDatModelPlugin.";
        }
        delete object;
    } else {
        qWarning() << pluginKey << " plugin not found." ;
    }
    return model;
}

QScxmlDataModel *QScxmlDataModelPrivate::instantiateDataModel(DocumentModel::Scxml::DataModelType type)
{
    QScxmlDataModel *dataModel = nullptr;
    switch (type) {
    case DocumentModel::Scxml::NullDataModel:
        dataModel = new QScxmlNullDataModel;
        break;
    case DocumentModel::Scxml::JSDataModel:
        dataModel = QScxmlDataModel::createScxmlDataModel(QStringLiteral("ecmascriptdatamodel"));
        break;
    case DocumentModel::Scxml::CppDataModel:
        break;
    default:
        Q_UNREACHABLE();
    }
    return dataModel;
}

/*!
 * \fn QScxmlDataModel::setup(const QVariantMap &initialDataValues)
 *
 * Initializes the data model with the initial values specified by
 * \a initialDataValues.
 *
 * Returns \c false if parse errors occur or if any of the initialization steps
 * fail. Returns \c true otherwise.
 */

/*!
 * \fn QScxmlDataModel::setScxmlEvent(const QScxmlEvent &event)
 *
 * Sets the \a event to use in the subsequent executable content execution.
 */

/*!
 * \fn QScxmlDataModel::scxmlProperty(const QString &name) const
 *
 * Returns the value of the property \a name.
 */

/*!
 * \fn QScxmlDataModel::hasScxmlProperty(const QString &name) const
 *
 * Returns \c true if a property with the given \a name exists, \c false
 * otherwise.
 */

/*!
 * \fn QScxmlDataModel::setScxmlProperty(const QString &name,
 *                                       const QVariant &value,
 *                                       const QString &context)
 *
 * Sets a the value \a value for the property \a name.
 *
 * The \a context is a string that is used in error messages to indicate the
 * location in the SCXML file where the error occurred.
 *
 * Returns \c true if successful or \c false if an error occurred.
 */

/*!
 * \fn QScxmlDataModel::evaluateToString(
 *           QScxmlExecutableContent::EvaluatorId id, bool *ok)
 * Evaluates the executable content pointed to by \a id and sets \a ok to
 * \c false if there was an error or to \c true if there was not.
 * Returns the result of the evaluation as a QString.
 */

/*!
 * \fn QScxmlDataModel::evaluateToBool(QScxmlExecutableContent::EvaluatorId id,
 *                                     bool *ok)
 * Evaluates the executable content pointed to by \a id and sets \a ok to
 * \c false if there was an error or to \c true if there was not.
 * Returns the result of the evaluation as a boolean value.
 */

/*!
 * \fn QScxmlDataModel::evaluateToVariant(
 *           QScxmlExecutableContent::EvaluatorId id, bool *ok)
 * Evaluates the executable content pointed to by \a id and sets \a ok to
 * \c false if there was an error or to \c true if there was not.
 * Returns the result of the evaluation as a QVariant.
 */

/*!
 * \fn QScxmlDataModel::evaluateToVoid(QScxmlExecutableContent::EvaluatorId id,
 *                                     bool *ok)
 * Evaluates the executable content pointed to by \a id and sets \a ok to
 * \c false if there was an error or to \c true if there was not.
 * The execution is expected to return no result.
 */

/*!
 * \fn QScxmlDataModel::evaluateAssignment(
 *           QScxmlExecutableContent::EvaluatorId id, bool *ok)
 * Evaluates the assignment pointed to by \a id and sets \a ok to
 * \c false if there was an error or to \c true if there was not.
 */

/*!
 * \fn QScxmlDataModel::evaluateInitialization(
 *           QScxmlExecutableContent::EvaluatorId id, bool *ok)
 * Evaluates the initialization pointed to by \a id and sets \a ok to
 * \c false if there was an error or to \c true if there was not.
 */

/*!
 * \fn QScxmlDataModel::evaluateForeach(
 *           QScxmlExecutableContent::EvaluatorId id, bool *ok,
 *           ForeachLoopBody *body)
 * Evaluates the foreach loop pointed to by \a id and sets \a ok to
 * \c false if there was an error or to \c true if there was not. The
 * \a body is executed on each iteration.
 */

QT_END_NAMESPACE
