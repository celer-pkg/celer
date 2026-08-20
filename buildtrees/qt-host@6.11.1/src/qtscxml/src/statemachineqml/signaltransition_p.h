// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef SIGNALTRANSITION_H
#define SIGNALTRANSITION_H

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

#include <QtStateMachine/QSignalTransition>
#include <QtCore/QVariant>
#include <QtQml/QJSValue>

#include <QtQml/qqmlscriptstring.h>
#include <QtQml/qqmlparserstatus.h>
#include <private/qqmlcustomparser_p.h>
#include <private/qqmlrefcount_p.h>
#include <private/qqmlboundsignal_p.h>
#include <QtCore/private/qproperty_p.h>

QT_BEGIN_NAMESPACE

class Q_STATEMACHINEQML_EXPORT SignalTransition : public QSignalTransition, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QJSValue signal READ signal WRITE setSignal
               NOTIFY qmlSignalChanged BINDABLE bindableSignal)
    Q_PROPERTY(QQmlScriptString guard READ guard WRITE setGuard
               NOTIFY guardChanged BINDABLE bindableGuard)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 0)
    QML_CUSTOMPARSER

public:
    explicit SignalTransition(QState *parent = nullptr);

    QQmlScriptString guard() const;
    void setGuard(const QQmlScriptString &guard);
    QBindable<QQmlScriptString> bindableGuard();

    bool eventTest(QEvent *event) override;
    void onTransition(QEvent *event) override;

    const QJSValue &signal();
    void setSignal(const QJSValue &signal);
    QBindable<QJSValue> bindableSignal();

    Q_INVOKABLE void invoke();

Q_SIGNALS:
    void guardChanged();
    void invokeYourself();
    /*!
     * \internal
     */
    void qmlSignalChanged();

private:
    void classBegin() override { m_complete = false; }
    void componentComplete() override { m_complete = true; connectTriggered(); }
    void connectTriggered();

    friend class SignalTransitionParser;

    Q_OBJECT_COMPAT_PROPERTY(SignalTransition, QJSValue, m_signal, &SignalTransition::setSignal,
                             &SignalTransition::qmlSignalChanged);
    Q_OBJECT_BINDABLE_PROPERTY(SignalTransition, QQmlScriptString,
                               m_guard, &SignalTransition::guardChanged);
    bool m_complete;
    QQmlRefPointer<QV4::ExecutableCompilationUnit> m_compilationUnit;
    QList<const QV4::CompiledData::Binding *> m_bindings;
    QQmlRefPointer<QQmlBoundSignalExpression> m_signalExpression;
};

class SignalTransitionParser : public QQmlCustomParser
{
public:
    void verifyBindings(
            const QQmlRefPointer<QV4::CompiledData::CompilationUnit> &compilationUnit,
            const QList<const QV4::CompiledData::Binding *> &props) override;
    void applyBindings(
            QObject *object, const QQmlRefPointer<QV4::ExecutableCompilationUnit> &compilationUnit,
            const QList<const QV4::CompiledData::Binding *> &bindings) override;
};

template<>
inline QQmlCustomParser *qmlCreateCustomParser<SignalTransition>()
{
    return new SignalTransitionParser;
}

QT_END_NAMESPACE

#endif
