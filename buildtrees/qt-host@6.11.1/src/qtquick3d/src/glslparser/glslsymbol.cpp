// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "glslsymbol_p.h"
#include <QStringList>

QT_BEGIN_NAMESPACE

using namespace GLSL;

Symbol::Symbol(Scope *scope)
    : _scope(scope)
{
}

Symbol::~Symbol()
{
}

Scope *Symbol::scope() const
{
    return _scope;
}

void Symbol::setScope(Scope *scope)
{
    _scope = scope;
}

QString Symbol::name() const
{
    return _name;
}

void Symbol::setName(const QString &name)
{
    _name = name;
}

Scope::Scope(Scope *enclosingScope)
    : Symbol(enclosingScope)
{
}

Symbol *Scope::lookup(const QString &name) const
{
    if (Symbol *s = find(name))
        return s;
    if (Scope *s = scope())
        return s->lookup(name);

    return nullptr;
}

QList<Symbol *> Scope::members() const
{
    return QList<Symbol *>();
}

QT_END_NAMESPACE
