// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant
#ifndef QV4SCRIPT_H
#define QV4SCRIPT_H

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

#include <private/qv4compilercontext_p.h>
#include <private/qv4engine_p.h>
#include <private/qv4global_p.h>
#include <private/qv4qmlcontext_p.h>

#include <QtQml/qqmlerror.h>

QT_BEGIN_NAMESPACE

class QQmlContextData;

namespace QQmlJS {
class Engine;
}

namespace QV4 {

struct Q_QML_EXPORT Script
{
    enum class InheritContext { No, Yes };
    static QQmlRefPointer<QV4::CompiledData::CompilationUnit> precompile(
            QV4::Compiler::Module *module, QQmlJS::Engine *jsEngine,
            Compiler::JSUnitGenerator *unitGenerator, const QString &fileName,
            const QString &source, QList<QQmlError> *reportedErrors = nullptr,
            QV4::Compiler::ContextType contextType = QV4::Compiler::ContextType::Global,
            InheritContext inheritContext = InheritContext::No);

    Script(
            ExecutionContext *scope, QV4::Compiler::ContextType mode, const QString &sourceCode,
            const QString &source = QString(), int line = 1)
        : m_sourceFile(source)
        , m_sourceCode(sourceCode)
        , m_context(scope)
        , m_line(line)
        , m_contextType(mode)
    {}

    Script(
            ExecutionEngine *engine, QmlContext *qml, bool parseAsBinding, const QString &sourceCode,
            const QString &source = QString(), int line = 1)
        : m_sourceFile(source)
        , m_sourceCode(sourceCode)
        , m_context(engine->rootContext())
        , m_line(line)
        , m_parseAsBinding(parseAsBinding)
        , m_inheritContext(true)
    {
        if (qml)
            m_qmlContext.set(engine, *qml);
    }

    Script(
            ExecutionEngine *engine, QmlContext *qml,
            const QQmlRefPointer<ExecutableCompilationUnit> &compilationUnit);

    ~Script();

    void setStrictMode(bool strictMode = true) { m_strictMode = strictMode; }
    void setInheritContext(bool inheritContext = true) { m_inheritContext = inheritContext; }
    void setParseAsBinding(bool parseAsBinding = true) { m_parseAsBinding = parseAsBinding; }
    QQmlRefPointer<ExecutableCompilationUnit> compilationUnit() const { return m_compilationUnit; }

    void parse();
    ReturnedValue run(const QV4::Value *thisObject = nullptr);

    Function *function();

private:
    QString m_sourceFile;
    QString m_sourceCode;
    QV4::PersistentValue m_qmlContext;
    QQmlRefPointer<ExecutableCompilationUnit> m_compilationUnit;
    QV4::WriteBarrier::Pointer<Function> m_vmFunction;

    ExecutionContext *m_context = nullptr;

    int m_line = 1;
    QV4::Compiler::ContextType m_contextType = QV4::Compiler::ContextType::Eval;

    bool m_strictMode = false;
    bool m_parseAsBinding = false;
    bool m_inheritContext = false;
    bool m_parsed = false;
};

}

QT_END_NAMESPACE

#endif
