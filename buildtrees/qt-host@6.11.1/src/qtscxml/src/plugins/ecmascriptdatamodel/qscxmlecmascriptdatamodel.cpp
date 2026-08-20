// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:code-generation

#include <QtScxml/private/qscxmlglobals_p.h>
#include "qscxmlecmascriptdatamodel_p.h"
#include "qscxmlecmascriptplatformproperties_p.h"
#include <QtScxml/private/qscxmlexecutablecontent_p.h>
#include <QtScxml/private/qscxmlstatemachine_p.h>
#include <QtScxml/private/qscxmldatamodel_p.h>

#include <qjsengine.h>
#include <qjsondocument.h>
#include <QtQml/private/qjsvalue_p.h>
#include <QtQml/private/qv4scopedvalue_p.h>

#include <functional>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qscxmlEsLog, "qt.scxml.statemachine")

using namespace QScxmlExecutableContent;

typedef std::function<QString (bool *)> ToStringEvaluator;
typedef std::function<bool (bool *)> ToBoolEvaluator;
typedef std::function<QVariant (bool *)> ToVariantEvaluator;
typedef std::function<void (bool *)> ToVoidEvaluator;
typedef std::function<bool (bool *, std::function<bool ()>)> ForeachEvaluator;

class QScxmlEcmaScriptDataModelPrivate : public QScxmlDataModelPrivate
{
    Q_DECLARE_PUBLIC(QScxmlEcmaScriptDataModel)
public:
    QScxmlEcmaScriptDataModelPrivate()
        : jsEngine(nullptr)
    {}

    QString evalStr(const QString &expr, const QString &context, bool *ok)
    {
        QString script = QStringLiteral("(%1).toString()").arg(expr);
        QJSValue v = eval(script, context, ok);
        if (*ok)
            return v.toString();
        else
            return QString();
    }

    bool evalBool(const QString &expr, const QString &context, bool *ok)
    {
        QString script = QStringLiteral("(function(){return !!(%1); })()").arg(expr);
        QJSValue v = eval(script, context, ok);
        if (*ok)
            return v.toBool();
        else
            return false;
    }

    QJSValue evalJSValue(const QString &expr, const QString &context, bool *ok)
    {
        assertEngine();

        QString script = QStringLiteral("(function(){'use strict'; return (\n%1\n); })()").arg(expr);
        return eval(script, context, ok);
    }

    QJSValue eval(const QString &script, const QString &context, bool *ok)
    {
        Q_ASSERT(ok);
        QJSEngine *engine = assertEngine();

        // TODO: copy QJSEngine::evaluate and handle the case of v4->catchException() "our way"

        QJSValue v = engine->evaluate(QStringLiteral("'use strict'; ") + script, QStringLiteral("<expr>"), 0);
        if (v.isError()) {
            *ok = false;
            submitError(QStringLiteral("error.execution"),
                        QStringLiteral("%1 in %2").arg(v.toString(), context));
            return QJSValue(QJSValue::UndefinedValue);
        } else {
            *ok = true;
            return v;
        }
    }

    void setupDataModel()
    {
        QJSEngine *engine = assertEngine();
        dataModel = engine->globalObject();

        qCDebug(qscxmlEsLog) << m_stateMachine << "initializing the datamodel";
        setupSystemVariables();
    }

    void setupSystemVariables()
    {
        setReadonlyProperty(&dataModel, QStringLiteral("_sessionid"),
                            m_stateMachine->sessionId());

        setReadonlyProperty(&dataModel, QStringLiteral("_name"), m_stateMachine->name());

        QJSEngine *engine = assertEngine();
        auto scxml = engine->newObject();
        scxml.setProperty(QStringLiteral("location"), QStringLiteral("#_scxml_%1")
                          .arg(m_stateMachine->sessionId()));
        auto ioProcs = engine->newObject();
        setReadonlyProperty(&ioProcs, QStringLiteral("scxml"), scxml);
        setReadonlyProperty(&dataModel, QStringLiteral("_ioprocessors"), ioProcs);

        auto platformVars = QScxmlPlatformProperties::create(engine, m_stateMachine);
        dataModel.setProperty(QStringLiteral("_x"), platformVars->jsValue());

        dataModel.setProperty(QStringLiteral("In"), engine->evaluate(
                                  QStringLiteral("(function(id){return _x.inState(id);})")));
    }

    void assignEvent(const QScxmlEvent &event)
    {
        if (event.name().isEmpty())
            return;

        QJSEngine *engine = assertEngine();
        QJSValue _event = engine->newObject();
        QJSValue dataValue = eventDataAsJSValue(event.data());
        _event.setProperty(QStringLiteral("data"), dataValue.isUndefined() ? QJSValue(QJSValue::UndefinedValue)
                                                                           : dataValue);
        _event.setProperty(QStringLiteral("invokeid"), event.invokeId().isEmpty() ? QJSValue(QJSValue::UndefinedValue)
                                                                                  : engine->toScriptValue(event.invokeId()));
        if (!event.originType().isEmpty())
            _event.setProperty(QStringLiteral("origintype"), engine->toScriptValue(event.originType()));
        _event.setProperty(QStringLiteral("origin"), event.origin().isEmpty() ? QJSValue(QJSValue::UndefinedValue)
                                                                              : engine->toScriptValue(event.origin()) );
        _event.setProperty(QStringLiteral("sendid"), event.sendId().isEmpty() ? QJSValue(QJSValue::UndefinedValue)
                                                                              : engine->toScriptValue(event.sendId()));
        _event.setProperty(QStringLiteral("type"), engine->toScriptValue(event.scxmlType()));
        _event.setProperty(QStringLiteral("name"), engine->toScriptValue(event.name()));
        _event.setProperty(QStringLiteral("raw"), QStringLiteral("unsupported")); // See test178
        if (event.isErrorEvent())
            _event.setProperty(QStringLiteral("errorMessage"), event.errorMessage());

        setReadonlyProperty(&dataModel, QStringLiteral("_event"), _event);
    }

    QJSValue eventDataAsJSValue(const QVariant &eventData)
    {
        if (!eventData.isValid()) {
            return QJSValue(QJSValue::UndefinedValue);
        }

        QJSEngine *engine = assertEngine();
        if (eventData.canConvert<QVariantMap>()) {
            auto keyValues = eventData.value<QVariantMap>();
            auto data = engine->newObject();

            for (QVariantMap::const_iterator it = keyValues.begin(), eit = keyValues.end(); it != eit; ++it) {
                data.setProperty(it.key(), engine->toScriptValue(it.value()));
            }

            return data;
        }

        if (eventData == QVariant(QMetaType(QMetaType::VoidStar), nullptr)) {
            return QJSValue(QJSValue::NullValue);
        }

        QString data = eventData.toString();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError)
            return engine->toScriptValue(doc.toVariant());
        else
            return engine->toScriptValue(data);
    }

    QJSEngine *assertEngine()
    {
        if (!jsEngine) {
            Q_Q(QScxmlEcmaScriptDataModel);
            setEngine(new QJSEngine(q->stateMachine()));
        }

        return jsEngine;
    }

    QJSEngine *engine() const
    {
        return jsEngine;
    }

    void setEngine(QJSEngine *engine)
    { jsEngine = engine; }

    QString string(StringId id) const
    {
        return m_stateMachine->tableData()->string(id);
    }

    bool hasProperty(const QString &name) const
    { return dataModel.hasProperty(name); }

    QJSValue property(const QString &name) const
    { return dataModel.property(name); }

    bool setProperty(const QString &name, const QJSValue &value, const QString &context)
    {
        QString msg;
        switch (setProperty(&dataModel, name, value)) {
        case SetPropertySucceeded:
            return true;
        case SetReadOnlyPropertyFailed:
            msg = QStringLiteral("cannot assign to read-only property %1 in %2");
            break;
        case SetUnknownPropertyFailed:
            msg = QStringLiteral("cannot assign to unknown propety %1 in %2");
            break;
        case SetPropertyFailedForAnotherReason:
            msg = QStringLiteral("assignment to property %1 failed in %2");
            break;
        default:
            Q_UNREACHABLE();
        }

        submitError(QStringLiteral("error.execution"), msg.arg(name, context));
        return false;
    }

    void submitError(const QString &type, const QString &msg, const QString &sendid = QString())
    {
        QScxmlStateMachinePrivate::get(m_stateMachine)->submitError(type, msg, sendid);
    }

public:
    QStringList initialDataNames;

private: // Uses private API
    static void setReadonlyProperty(QJSValue *object, const QString &name, const QJSValue &value)
    {
        qCDebug(qscxmlEsLog) << "setting read-only property" << name;
        QV4::ExecutionEngine *engine = QJSValuePrivate::engine(object);
        Q_ASSERT(engine);
        QV4::Scope scope(engine);

        QV4::ScopedObject o(scope, QJSValuePrivate::asManagedType<QV4::Object>(object));
        if (!o)
            return;

        if (!QJSValuePrivate::checkEngine(engine, value)) {
            qCWarning(qscxmlEsLog, "EcmaScriptDataModel::setReadonlyProperty(%s) failed: cannot set value created in a different engine", name.toUtf8().constData());
            return;
        }

        QV4::ScopedString s(scope, engine->newString(name));
        QV4::ScopedPropertyKey key(scope, s->toPropertyKey());
        if (key->isArrayIndex()) {
            Q_UNIMPLEMENTED();
            return;
        }

        QV4::ScopedValue v(scope, QJSValuePrivate::convertToReturnedValue(engine, value));
        o->defineReadonlyProperty(s, v);
        if (engine->hasException)
            engine->catchException();
    }

    enum SetPropertyResult {
        SetPropertySucceeded,
        SetReadOnlyPropertyFailed,
        SetUnknownPropertyFailed,
        SetPropertyFailedForAnotherReason,
    };

    static SetPropertyResult setProperty(QJSValue *object, const QString &name, const QJSValue &value)
    {
        QV4::ExecutionEngine *engine = QJSValuePrivate::engine(object);
        Q_ASSERT(engine);
        if (engine->hasException)
            return SetPropertyFailedForAnotherReason;

        QV4::Scope scope(engine);
        QV4::ScopedObject o(scope, QJSValuePrivate::asManagedType<QV4::Object>(object));
        if (o == nullptr) {
            return SetPropertyFailedForAnotherReason;
        }

        QV4::ScopedString s(scope, engine->newString(name));
        QV4::ScopedPropertyKey key(scope, s->toPropertyKey());
        if (key->isArrayIndex()) {
            Q_UNIMPLEMENTED();
            return SetPropertyFailedForAnotherReason;
        }

        QV4::PropertyAttributes attrs = o->getOwnProperty(s->toPropertyKey());
        if (attrs.isWritable() || attrs.isEmpty()) {
            QV4::ScopedValue v(scope, QJSValuePrivate::convertToReturnedValue(engine, value));
            o->insertMember(s, v);
            if (engine->hasException) {
                engine->catchException();
                return SetPropertyFailedForAnotherReason;
            } else {
                return SetPropertySucceeded;
            }
        } else {
            return SetReadOnlyPropertyFailed;
        }
    }

private:
    QJSEngine *jsEngine;
    QJSValue dataModel;
};

/*
 * The QScxmlEcmaScriptDataModel class is the ECMAScript data model for
 * a Qt SCXML state machine.
 *
 * This class implements the ECMAScript data model as described in
 * "SCXML Specification - B.2 The ECMAScript Data Model". It can be
 * subclassed to perform custom initialization.
 *
 * See also QScxmlStateMachine QScxmlDataModel
 */

/*
 * Creates a new ECMAScript data model, with the parent object parent.
 */
QScxmlEcmaScriptDataModel::QScxmlEcmaScriptDataModel(QObject *parent)
    : QScxmlDataModel(*(new QScxmlEcmaScriptDataModelPrivate), parent)
{}

bool QScxmlEcmaScriptDataModel::setup(const QVariantMap &initialDataValues)
{
    Q_D(QScxmlEcmaScriptDataModel);
    d->setupDataModel();

    bool ok = true;
    QJSValue undefined(QJSValue::UndefinedValue); // See B.2.1, and test456.
    int count;
    StringId *names = d->m_stateMachine->tableData()->dataNames(&count);
    for (int i = 0; i < count; ++i) {
        auto name = d->string(names[i]);
        QJSValue v = undefined;
        QVariantMap::const_iterator it = initialDataValues.find(name);
        if (it != initialDataValues.end()) {
            QJSEngine *engine = d->assertEngine();
            v = engine->toScriptValue(it.value());
        }
        if (!d->setProperty(name, v, QStringLiteral("<data>"))) {
            ok = false;
        }
    }
    d->initialDataNames = initialDataValues.keys();

    return ok;
}

QString QScxmlEcmaScriptDataModel::evaluateToString(QScxmlExecutableContent::EvaluatorId id,
                                                    bool *ok)
{
    Q_D(QScxmlEcmaScriptDataModel);
    const EvaluatorInfo &info = d->m_stateMachine->tableData()->evaluatorInfo(id);

    return d->evalStr(d->string(info.expr), d->string(info.context), ok);
}

bool QScxmlEcmaScriptDataModel::evaluateToBool(QScxmlExecutableContent::EvaluatorId id,
                                               bool *ok)
{
    Q_D(QScxmlEcmaScriptDataModel);
    const EvaluatorInfo &info = d->m_stateMachine->tableData()->evaluatorInfo(id);

    return d->evalBool(d->string(info.expr), d->string(info.context), ok);
}

QVariant QScxmlEcmaScriptDataModel::evaluateToVariant(QScxmlExecutableContent::EvaluatorId id,
                                                      bool *ok)
{
    Q_D(QScxmlEcmaScriptDataModel);
    const EvaluatorInfo &info = d->m_stateMachine->tableData()->evaluatorInfo(id);

    return d->evalJSValue(d->string(info.expr), d->string(info.context), ok).toVariant();
}

void QScxmlEcmaScriptDataModel::evaluateToVoid(QScxmlExecutableContent::EvaluatorId id,
                                               bool *ok)
{
    Q_D(QScxmlEcmaScriptDataModel);
    const EvaluatorInfo &info = d->m_stateMachine->tableData()->evaluatorInfo(id);

    d->eval(d->string(info.expr), d->string(info.context), ok);
}

void QScxmlEcmaScriptDataModel::evaluateAssignment(QScxmlExecutableContent::EvaluatorId id,
                                                   bool *ok)
{
    Q_D(QScxmlEcmaScriptDataModel);
    Q_ASSERT(ok);

    const AssignmentInfo &info = d->m_stateMachine->tableData()->assignmentInfo(id);

    QString dest = d->string(info.dest);

    if (hasScxmlProperty(dest)) {
        QJSValue v = d->evalJSValue(d->string(info.expr), d->string(info.context), ok);
        if (*ok)
            *ok = d->setProperty(dest, v, d->string(info.context));
    } else {
        *ok = false;
        d->submitError(QStringLiteral("error.execution"),
                       QStringLiteral("%1 in %2 does not exist").arg(dest, d->string(info.context)));
    }
}

void QScxmlEcmaScriptDataModel::evaluateInitialization(QScxmlExecutableContent::EvaluatorId id,
                                                       bool *ok)
{
    Q_D(QScxmlEcmaScriptDataModel);
    const AssignmentInfo &info = d->m_stateMachine->tableData()->assignmentInfo(id);
    QString dest = d->string(info.dest);
    if (d->initialDataNames.contains(dest)) {
        *ok = true; // silently ignore the <data> tag
        return;
    }

    evaluateAssignment(id, ok);
}

void QScxmlEcmaScriptDataModel::evaluateForeach(QScxmlExecutableContent::EvaluatorId id, bool *ok,
                                                ForeachLoopBody *body)
{
    Q_D(QScxmlEcmaScriptDataModel);
    Q_ASSERT(ok);
    Q_ASSERT(body);
    const ForeachInfo &info = d->m_stateMachine->tableData()->foreachInfo(id);

    QJSValue jsArray = d->evalJSValue(d->string(info.array), d->string(info.context), ok);
    if (!jsArray.isArray()) {
        d->submitError(QStringLiteral("error.execution"), QStringLiteral("invalid array '%1' in %2").arg(d->string(info.array), d->string(info.context)));
        *ok = false;
        return;
    }

    QString item = d->string(info.item);

    QJSEngine *engine = d->assertEngine();
    if (engine->evaluate(QStringLiteral("(function(){var %1 = 0})()").arg(item)).isError()) {
        d->submitError(QStringLiteral("error.execution"), QStringLiteral("invalid item '%1' in %2")
                      .arg(d->string(info.item), d->string(info.context)));
        *ok = false;
        return;
    }

    const int length = jsArray.property(QStringLiteral("length")).toInt();
    QString idx = d->string(info.index);
    QString context = d->string(info.context);
    const bool hasIndex = !idx.isEmpty();

    for (int currentIndex = 0; currentIndex < length; ++currentIndex) {
        QJSValue currentItem = jsArray.property(static_cast<quint32>(currentIndex));
        *ok = d->setProperty(item, currentItem, context);
        if (!*ok)
            return;
        if (hasIndex) {
            *ok = d->setProperty(idx, currentIndex, context);
            if (!*ok)
                return;
        }
        body->run(ok);
        if (!*ok)
            return;
    }
    *ok = true;
}

void QScxmlEcmaScriptDataModel::setScxmlEvent(const QScxmlEvent &event)
{
    Q_D(QScxmlEcmaScriptDataModel);
    d->assignEvent(event);
}

QVariant QScxmlEcmaScriptDataModel::scxmlProperty(const QString &name) const
{
    Q_D(const QScxmlEcmaScriptDataModel);
    return d->property(name).toVariant();
}

bool QScxmlEcmaScriptDataModel::hasScxmlProperty(const QString &name) const
{
    Q_D(const QScxmlEcmaScriptDataModel);
    return d->hasProperty(name);
}

bool QScxmlEcmaScriptDataModel::setScxmlProperty(const QString &name, const QVariant &value,
                                                 const QString &context)
{
    Q_D(QScxmlEcmaScriptDataModel);
    Q_ASSERT(hasScxmlProperty(name));

    QJSEngine *engine = d->assertEngine();
    QJSValue v = engine->toScriptValue(
                value.canConvert<QJSValue>() ? value.value<QJSValue>().toVariant() : value);
    return d->setProperty(name, v, context);
}

QT_END_NAMESPACE
