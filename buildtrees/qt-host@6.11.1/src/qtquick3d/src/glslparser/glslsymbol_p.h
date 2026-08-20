// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_GLSLSYMBOL_H
#define QSSG_GLSLSYMBOL_H

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

#include <QtQuick3DGlslParser/private/glsl_p.h>
#include <QString>

QT_BEGIN_NAMESPACE

namespace GLSL {

class Symbol;
class Scope;

class Q_QUICK3DGLSLPARSER_EXPORT Symbol
{
public:
    Symbol(Scope *scope = nullptr);
    virtual ~Symbol();

    Scope *scope() const;
    void setScope(Scope *scope);

    QString name() const;
    void setName(const QString &name);

    virtual Scope *asScope() { return nullptr; }
    virtual Struct *asStruct() { return nullptr; }
    virtual Function *asFunction() { return nullptr; }
    virtual Argument *asArgument() { return nullptr; }
    virtual Block *asBlock() { return nullptr; }
    virtual Variable *asVariable() { return nullptr; }
    virtual OverloadSet *asOverloadSet() { return nullptr; }
    virtual Namespace *asNamespace() { return nullptr; }

    virtual const Type *type() const = 0;

private:
    Scope *_scope;
    QString _name;
};

class Q_QUICK3DGLSLPARSER_EXPORT Scope: public Symbol
{
public:
    Scope(Scope *sscope = nullptr);

    Symbol *lookup(const QString &name) const;

    virtual QList<Symbol *> members() const;
    virtual void add(Symbol *symbol) = 0;
    virtual Symbol *find(const QString &name) const = 0;

    Scope *asScope() override { return this; }
};

} // namespace GLSL

QT_END_NAMESPACE

#endif // QSSG_GLSLSYMBOL_H
