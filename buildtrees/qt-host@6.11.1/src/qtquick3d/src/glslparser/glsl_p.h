// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_GLSL_H
#define QSSG_GLSL_H

#include <QtQuick3DGlslParser/qtquick3dglslparserexports.h>

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

QT_BEGIN_NAMESPACE

namespace GLSL {
class Engine;
class Lexer;
class Parser;
class MemoryPool;

// types
class Type;
class UndefinedType;
class VoidType;
class ScalarType;
class BoolType;
class IntType;
class UIntType;
class FloatType;
class DoubleType;
class IndexType;
class VectorType;
class MatrixType;
class ArrayType;
class SamplerType;

// symbols
class Symbol;
class Scope;
class Struct;
class Function;
class Argument;
class Block;
class Variable;
class OverloadSet;
class Namespace;

class AST;
class TranslationUnitAST;
template <typename T> class List;
}

QT_END_NAMESPACE

#endif // QSSG_GLSL_H
