// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qv4include_p.h"
#include "qv4scopedvalue_p.h"
#include "qv4jscall_p.h"

#include <QtQml/qjsengine.h>
#if QT_CONFIG(qml_network)
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#endif
#include <QtCore/qfile.h>
#include <QtQml/qqmlfile.h>

#include <private/qqmlengine_p.h>
#include <private/qqmlscriptblob_p.h>
#include <private/qqmlscriptdata_p.h>
#include <private/qv4context_p.h>
#include <private/qv4engine_p.h>
#include <private/qv4functionobject_p.h>
#include <private/qv4script_p.h>

QT_BEGIN_NAMESPACE

QV4::ReturnedValue QV4Include::resultValue(QV4::ExecutionEngine *v4)
{
    QV4::Scope scope(v4);

    // XXX It seems inefficient to create this object from scratch each time.
    QV4::ScopedObject o(scope, v4->newObject());
    QV4::ScopedString s(scope);
    QV4::ScopedValue v(scope);
    o->put((s = v4->newString(QStringLiteral("OK"))), (v = QV4::Value::fromInt32(Ok)));
    o->put((s = v4->newString(QStringLiteral("LOADING"))), (v = QV4::Value::fromInt32(Loading)));
    o->put((s = v4->newString(QStringLiteral("NETWORK_ERROR"))), (v = QV4::Value::fromInt32(NetworkError)));
    o->put((s = v4->newString(QStringLiteral("EXCEPTION"))), (v = QV4::Value::fromInt32(Exception)));
    return o.asReturnedValue();
}

void QV4Include::populateResultValue(QV4::Object *o, Status status, const QString &statusText)
{
    QV4::Scope scope(o->engine());
    QV4::ScopedString statusKey(scope, scope.engine->newString(QStringLiteral("status")));
    QV4::ScopedValue statusValue(scope, QV4::Value::fromInt32(status));
    o->put(statusKey, statusValue);
    if (statusText.isEmpty())
        return;

    QV4::ScopedString textKey(scope, scope.engine->newString(QStringLiteral("statusText")));
    QV4::ScopedValue textValue(scope, scope.engine->newString(statusText));
    o->put(textKey, textValue);
}

QV4Include::QV4Include(
        QV4::Object *result, QV4::FunctionObject *callbackFunction, QV4::QmlContext *qmlContext)
{
    Q_ASSERT(result);
    QV4::ExecutionEngine *engine = result->engine();
    m_resultObject.set(engine, result->asReturnedValue());
    if (callbackFunction)
        m_callbackFunction.set(engine, callbackFunction->asReturnedValue());
    if (qmlContext)
        m_qmlContext.set(engine, qmlContext->asReturnedValue());
}

void QV4Include::callback(QV4::FunctionObject *callback, QV4::Object *result)
{
    if (!callback)
        return;

    QV4::Scope scope(callback->engine());
    QV4::JSCallArguments jsCallData(scope, 1);
    *jsCallData.thisObject = scope.engine->globalObject->asReturnedValue();
    jsCallData.args[0] = result;
    callback->call(jsCallData);
    if (scope.hasException())
        scope.engine->catchException();
}

/*
    Documented in qv4engine.cpp
*/
QJSValue QV4Include::method_include(QV4::ExecutionEngine *engine, const QUrl &url,
                                    const QJSValue &callbackFunction)
{
    QQmlRefPointer<QQmlContextData> context = engine->callingQmlContext();

    if ((!context || !context->isJSContext()) && engine->qmlEngine()) {
        return QJSValuePrivate::fromReturnedValue(
                    engine->throwError(
                        QString::fromUtf8(
                            "Qt.include(): Can only be called from JavaScript files")));
    }

    // The JavaScript compilation unit inherits its context when doing Qt.include().
    // This means we need to turn fast lookups off at compile time.
    // Signal this through the URL fragment.
    QUrl includeUrl = url;
    includeUrl.setFragment(QLatin1String("include"));
    QQmlRefPointer<QQmlScriptBlob> scriptBlob = engine->typeLoader()->getScript(includeUrl);

    QV4::Scope scope(engine);
    QV4::ScopedObject result(scope, resultValue(engine));
    QV4::ScopedFunctionObject callback(
            scope, QJSValuePrivate::asReturnedValue(&callbackFunction));
    QV4::Scoped<QV4::QmlContext> qmlContext(scope, scope.engine->qmlContext());

    if (scriptBlob->isCompleteOrError()) {
        processScriptBlob(scriptBlob.data(), result, callback, qmlContext);
        return QJSValuePrivate::fromReturnedValue(result.asReturnedValue());
    }

    scriptBlob->registerCallback(new QV4Include(result, callback, qmlContext));
    populateResultValue(result, Loading);
    return QJSValuePrivate::fromReturnedValue(result.asReturnedValue());
}

void QV4Include::ready(QQmlNotifyingBlob *notifyingBlob)
{
    Q_ASSERT(notifyingBlob);
    Q_ASSERT(notifyingBlob->type() == QQmlDataBlob::JavaScriptFile);

    processScriptBlob(
            static_cast<QQmlScriptBlob *>(notifyingBlob), m_resultObject.as<QV4::Object>(),
            m_callbackFunction.as<QV4::FunctionObject>(), m_qmlContext.as<QV4::QmlContext>());

    delete this;
}

void QV4Include::processScriptBlob(
        QQmlScriptBlob *scriptBlob, QV4::Object *result, QV4::FunctionObject *callbackFunction,
        QV4::QmlContext *qmlContext)
{
    Q_ASSERT(scriptBlob);
    Q_ASSERT(result);

    QV4::Scope scope(result->engine());

    if (scriptBlob->isError()) {
        const QList<QQmlError> errors = scriptBlob->errors();
        Q_ASSERT(!errors.isEmpty());
        const QString errorString = QLatin1String("Error opening source file %1: %2")
                                            .arg(scriptBlob->finalUrlString(), errors[0].toString());
        populateResultValue(result, NetworkError, errorString);
        callback(callbackFunction, result);
        return;
    }

    Q_ASSERT(scriptBlob->isComplete());
    const auto cu = scope.engine->executableCompilationUnit(
            scriptBlob->scriptData()->compilationUnit());
    if (QV4::Function *vmFunction = cu->rootFunction()) {
        QScopedValueRollback<QV4::Function *> savedGlobal(scope.engine->globalCode, vmFunction);
        vmFunction->call(nullptr, nullptr, 0, qmlContext ? qmlContext : scope.engine->rootContext());
    }

    if (scope.hasException()) {
        QV4::ScopedValue ex(scope, scope.engine->catchException());
        populateResultValue(result, Exception);
        QV4::ScopedString exception(scope, scope.engine->newString(QStringLiteral("exception")));
        result->put(exception, ex);
    } else {
        populateResultValue(result, Ok);
    }

    callback(callbackFunction, result);
}

QT_END_NAMESPACE

