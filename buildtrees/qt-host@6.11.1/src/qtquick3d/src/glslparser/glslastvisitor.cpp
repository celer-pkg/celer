// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "glslastvisitor_p.h"

QT_BEGIN_NAMESPACE

using namespace GLSL;

Visitor::Visitor()
{
}

Visitor::~Visitor()
{
}

void Visitor::accept(AST *ast)
{
    if (ast)
        ast->accept(this);
}

QT_END_NAMESPACE
