// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "signaltransition_p.h"

#include <QStateMachine>
#include <QMetaProperty>
#include <QQmlInfo>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlExpression>

#include <private/qv4qobjectwrapper_p.h>
#include <private/qjsvalue_p.h>
#include <private/qv4scopedvalue_p.h>
#include <private/qqmlcontext_p.h>

SignalTransition::SignalTransition(QState *parent)
    : QSignalTransition(this, SIGNAL(invokeYourself()), parent), m_complete(false), m_signalExpression(nullptr)
{
    connect(this, &SignalTransition::signalChanged, this, [this](){ m_signal.notify(); });
}

bool SignalTransition::eventTest(QEvent *event)
{
    Q_ASSERT(event);
    if (!QSignalTransition::eventTest(event))
        return false;

    if (m_guard.value().isEmpty())
        return true;

    QQmlContext *outerContext = QQmlEngine::contextForObject(this);
    QQmlContext context(outerContext);
    QQmlContextData::get(&context)->setImports(QQmlContextData::get(outerContext)->imports());

    QStateMachine::SignalEvent *e = static_cast<QStateMachine::SignalEvent*>(event);

    // Set arguments as context properties
    int count = e->arguments().size();
    QMetaMethod metaMethod = e->sender()->metaObject()->method(e->signalIndex());
    const auto parameterNames = metaMethod.parameterNames();
    for (int i = 0; i < count; i++)
        context.setContextProperty(QString::fromUtf8(parameterNames[i]), QVariant::fromValue(e->arguments().at(i)));

    QQmlExpression expr(m_guard.value(), &context, this);
    QVariant result = expr.evaluate();

    return result.toBool();
}

void SignalTransition::onTransition(QEvent *event)
{
    if (QQmlEnginePrivate *engine = m_signalExpression
            ? QQmlEnginePrivate::get(m_signalExpression->engine())
            : nullptr) {

        QStateMachine::SignalEvent *e = static_cast<QStateMachine::SignalEvent*>(event);

        QVarLengthArray<void *, 2> argValues;
        QVarLengthArray<QMetaType, 2> argTypes;

        QVariantList eventArguments = e->arguments();
        const int argCount = eventArguments.size();
        argValues.reserve(argCount + 1);
        argTypes.reserve(argCount + 1);

        // We're not interested in the return value
        argValues.append(nullptr);
        argTypes.append(QMetaType());

        for (QVariant &arg : eventArguments) {
            argValues.append(arg.data());
            argTypes.append(arg.metaType());
        }

        engine->referenceScarceResources();
        m_signalExpression->QQmlJavaScriptExpression::evaluate(
                    argValues.data(), argTypes.constData(), argCount);
        engine->dereferenceScarceResources();
    }
    QSignalTransition::onTransition(event);
}

const QJSValue& SignalTransition::signal()
{
    return m_signal;
}

void SignalTransition::setSignal(const QJSValue &signal)
{
    m_signal.removeBindingUnlessInWrapper();
    if (m_signal.valueBypassingBindings().strictlyEquals(signal))
        return;

    QV4::ExecutionEngine *jsEngine = QQmlEngine::contextForObject(this)->engine()->handle();
    QV4::Scope scope(jsEngine);

    QObject *sender;
    QMetaMethod signalMethod;

    m_signal.setValueBypassingBindings(signal);
    QV4::ScopedValue value(scope, QJSValuePrivate::asReturnedValue(&signal));

    // Did we get the "slot" that can be used to invoke the signal?
    if (QV4::QObjectMethod *signalSlot = value->as<QV4::QObjectMethod>()) {
        sender = signalSlot->object();
        Q_ASSERT(sender);
        signalMethod = sender->metaObject()->method(signalSlot->methodIndex());
    } else if (QV4::QmlSignalHandler *signalObject = value->as<QV4::QmlSignalHandler>()) {
        // or did we get the signal object (the one with the connect()/disconnect() functions) ?
        sender = signalObject->object();
        Q_ASSERT(sender);
        signalMethod = sender->metaObject()->method(signalObject->signalIndex());
    } else {
        qmlWarning(this) << tr("Specified signal does not exist.");
        return;
    }

    QSignalTransition::setSenderObject(sender);
    // the call below will emit change signal, and the interceptor lambda in ctor will notify()
    QSignalTransition::setSignal(signalMethod.methodSignature());

    connectTriggered();
}

QBindable<QJSValue> SignalTransition::bindableSignal()
{
    return &m_signal;
}

QQmlScriptString SignalTransition::guard() const
{
    return m_guard;
}

void SignalTransition::setGuard(const QQmlScriptString &guard)
{
    m_guard = guard;
}

QBindable<QQmlScriptString> SignalTransition::bindableGuard()
{
    return &m_guard;
}

void SignalTransition::invoke()
{
    emit invokeYourself();
}

void SignalTransition::connectTriggered()
{
    if (!m_complete || !m_compilationUnit)
        return;

    const QObject *target = senderObject();
    QQmlData *ddata = QQmlData::get(this);
    QQmlRefPointer<QQmlContextData> ctxtdata = ddata ? ddata->outerContext : nullptr;

    Q_ASSERT(m_bindings.size() == 1);
    const QV4::CompiledData::Binding *binding = m_bindings.at(0);
    Q_ASSERT(binding->type() == QV4::CompiledData::Binding::Type_Script);

    QV4::ExecutionEngine *jsEngine = QQmlEngine::contextForObject(this)->engine()->handle();
    QV4::Scope scope(jsEngine);
    QV4::Scoped<QV4::QObjectMethod> qobjectSignal(
                scope, QJSValuePrivate::asReturnedValue(&m_signal.value()));
    if (!qobjectSignal) {
        m_signalExpression.adopt(nullptr);
        return;
    }

    QMetaMethod metaMethod = target->metaObject()->method(qobjectSignal->methodIndex());
    int signalIndex = QMetaObjectPrivate::signalIndex(metaMethod);

    auto f = m_compilationUnit->runtimeFunctions[binding->value.compiledScriptIndex];
    if (ctxtdata) {
        QQmlRefPointer<QQmlBoundSignalExpression> expression(
                new QQmlBoundSignalExpression(target, signalIndex, ctxtdata, this, f),
                QQmlRefPointer<QQmlBoundSignalExpression>::Adopt);
        expression->setNotifyOnValueChanged(false);
        m_signalExpression = expression;
    } else {
        m_signalExpression.adopt(nullptr);
    }
}

void SignalTransitionParser::verifyBindings(
        const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compilationUnit,
        const QList<const QV4::CompiledData::Binding *> &props)
{
    for (int ii = 0; ii < props.size(); ++ii) {
        const QV4::CompiledData::Binding *binding = props.at(ii);

        QString propName = compilationUnit->stringAt(binding->propertyNameIndex);

        if (propName != QLatin1String("onTriggered")) {
            error(props.at(ii), SignalTransition::tr("Cannot assign to non-existent property \"%1\"").arg(propName));
            return;
        }

        if (binding->type() != QV4::CompiledData::Binding::Type_Script) {
            error(binding, SignalTransition::tr("SignalTransition: script expected"));
            return;
        }
    }
}

void SignalTransitionParser::applyBindings(
        QObject *object, const QQmlRefPointer<QV4::ExecutableCompilationUnit> &compilationUnit,
        const QList<const QV4::CompiledData::Binding *> &bindings)
{
    SignalTransition *st = qobject_cast<SignalTransition*>(object);
    st->m_compilationUnit = compilationUnit;
    st->m_bindings = bindings;
}

/*!
    \qmltype QAbstractTransition
    \inqmlmodule QtQml.StateMachine
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief The QAbstractTransition type is the base type of transitions between QAbstractState objects.

    The QAbstractTransition type is the abstract base type of transitions
    between states (QAbstractState objects) of a StateMachine.
    QAbstractTransition is part of \l{Qt State Machine QML Guide}{Qt State Machine QML API}


    The sourceState() property has the source of the transition. The
    targetState and targetStates properties return the target(s) of the
    transition.

    The triggered() signal is emitted when the transition has been triggered.

    Do not use QAbstractTransition directly; use SignalTransition or
    TimeoutTransition instead.

    \sa SignalTransition, TimeoutTransition
*/

/*!
    \qmlproperty bool QAbstractTransition::sourceState
    \readonly sourceState

    \brief The source state (parent) of this transition.
*/

/*!
    \qmlproperty QAbstractState QAbstractTransition::targetState

    \brief The target state of this transition.

    If a transition has no target state, the transition may still be
    triggered, but this will not cause the state machine's configuration to
    change (i.e. the current state will not be exited and re-entered).
*/

/*!
    \qmlproperty list<QAbstractState> QAbstractTransition::targetStates

    \brief The target states of this transition.

    If multiple states are specified, they all must be descendants of the
    same parallel group state.
*/

/*!
    \qmlsignal QAbstractTransition::triggered()

    This signal is emitted when the transition has been triggered.
*/

/*!
    \qmltype QSignalTransition
    \inqmlmodule QtQml.StateMachine
    \inherits QAbstractTransition
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief The QSignalTransition type provides a transition based on a Qt signal.

    Do not use QSignalTransition directly; use SignalTransition or
    TimeoutTransition instead.

    \sa SignalTransition, TimeoutTransition
*/

/*!
    \qmlproperty string QSignalTransition::signal

    \brief The signal which is associated with this signal transition.
*/

/*!
    \qmlproperty QObject QSignalTransition::senderObject

    \brief The sender object which is associated with this signal transition.
*/


/*!
    \qmltype SignalTransition
    \inqmlmodule QtQml.StateMachine
    \inherits QSignalTransition
    \ingroup statemachine-qmltypes
    \since 5.4

    \brief The SignalTransition type provides a transition based on a Qt signal.

    SignalTransition is part of \l{Qt State Machine QML Guide}{Qt State Machine QML API}.

    \section1 Example Usage

    \snippet qml/statemachine/signaltransition.qml document

    \clearfloat

    \sa StateMachine, FinalState, TimeoutTransition
*/

/*!
    \qmlproperty signal SignalTransition::signal

    \brief The signal which is associated with this signal transition.

    \snippet qml/statemachine/signaltransitionsignal.qml document
*/

/*!
    \qmlproperty bool SignalTransition::guard

    Guard conditions affect the behavior of a state machine by enabling
    transitions only when they evaluate to true and disabling them when
    they evaluate to false.

    When the signal associated with this signal transition is emitted the
    guard condition is evaluated. In the guard condition the arguments
    of the signal can be used as demonstrated in the example below.

    \snippet qml/statemachine/guardcondition.qml document

    \sa signal
*/

#include "moc_signaltransition_p.cpp"
