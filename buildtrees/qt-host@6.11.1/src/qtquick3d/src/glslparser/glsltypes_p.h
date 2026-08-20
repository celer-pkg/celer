// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_GLSLTYPES_H
#define QSSG_GLSLTYPES_H

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

#include <QtQuick3DGlslParser/private/glsltype_p.h>
#include <QtQuick3DGlslParser/private/glslsymbol_p.h>
#include <QVector>
#include <QHash>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE

namespace GLSL {

class Q_QUICK3DGLSLPARSER_EXPORT ScalarType: public Type
{
public:
    const ScalarType *asScalarType() const override { return this; }
};

class Q_QUICK3DGLSLPARSER_EXPORT UndefinedType: public Type
{
public:
    QString toString() const override { return QLatin1String("undefined"); }
    const UndefinedType *asUndefinedType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

class Q_QUICK3DGLSLPARSER_EXPORT VoidType: public Type
{
public:
    QString toString() const override { return QLatin1String("void"); }
    const VoidType *asVoidType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

class Q_QUICK3DGLSLPARSER_EXPORT BoolType: public ScalarType
{
public:
    QString toString() const override { return QLatin1String("bool"); }
    const BoolType *asBoolType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

class Q_QUICK3DGLSLPARSER_EXPORT IntType: public ScalarType
{
public:
    QString toString() const override { return QLatin1String("int"); }
    const IntType *asIntType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

class Q_QUICK3DGLSLPARSER_EXPORT UIntType: public ScalarType
{
public:
    QString toString() const override { return QLatin1String("uint"); }
    const UIntType *asUIntType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

class Q_QUICK3DGLSLPARSER_EXPORT FloatType: public ScalarType
{
public:
    QString toString() const override { return QLatin1String("float"); }
    const FloatType *asFloatType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

class Q_QUICK3DGLSLPARSER_EXPORT DoubleType: public ScalarType
{
public:
    QString toString() const override { return QLatin1String("double"); }
    const DoubleType *asDoubleType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

// Type that can be indexed with the [] operator.
class Q_QUICK3DGLSLPARSER_EXPORT IndexType: public Type
{
public:
    IndexType(const Type *indexElementType) : _indexElementType(indexElementType) {}

    const Type *indexElementType() const { return _indexElementType; }

    const IndexType *asIndexType() const override { return this; }

private:
    const Type *_indexElementType;
};

class Q_QUICK3DGLSLPARSER_EXPORT VectorType: public IndexType, public Scope
{
public:
    VectorType(const Type *elementType, int dimension)
        : IndexType(elementType), _dimension(dimension) {}

    QString toString() const override;
    const Type *elementType() const { return indexElementType(); }
    int dimension() const { return _dimension; }

    QList<Symbol *> members() const override { return _members.values(); }

    void add(Symbol *symbol) override;
    Symbol *find(const QString &name) const override;
    const Type *type() const override { return this; }

    const VectorType *asVectorType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;

private:
    int _dimension;
    QHash<QString, Symbol *> _members;

    friend class Engine;

    void populateMembers(Engine *engine);
    void populateMembers(Engine *engine, const char *components);
};

class Q_QUICK3DGLSLPARSER_EXPORT MatrixType: public IndexType
{
public:
    MatrixType(const Type *elementType, int columns, int rows, const Type *columnType)
        : IndexType(columnType), _elementType(elementType), _columns(columns), _rows(rows) {}

    const Type *elementType() const { return _elementType; }
    const Type *columnType() const { return indexElementType(); }
    int columns() const { return _columns; }
    int rows() const { return _rows; }

    QString toString() const override;
    const MatrixType *asMatrixType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;

private:
    const Type *_elementType;
    int _columns;
    int _rows;
};

class Q_QUICK3DGLSLPARSER_EXPORT ArrayType: public IndexType
{
public:
    explicit ArrayType(const Type *elementType)
        : IndexType(elementType) {}

    const Type *elementType() const { return indexElementType(); }

    QString toString() const override;
    const ArrayType *asArrayType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;
};

class Q_QUICK3DGLSLPARSER_EXPORT Struct: public Type, public Scope
{
public:
    Struct(Scope *scope = nullptr)
        : Scope(scope) {}

    QList<Symbol *> members() const override;
    void add(Symbol *member) override;
    Symbol *find(const QString &name) const override;

    // as Type
    QString toString() const override { return name(); }
    const Struct *asStructType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;

    // as Symbol
    Struct *asStruct() override { return this; } // as Symbol
    const Type *type() const override { return this; }

private:
    QVector<Symbol *> _members;
};

class Q_QUICK3DGLSLPARSER_EXPORT Function: public Type, public Scope
{
public:
    Function(Scope *scope = nullptr)
        : Scope(scope) {}

    const Type *returnType() const;
    void setReturnType(const Type *returnType);

    QVector<Argument *> arguments() const;
    void addArgument(Argument *arg);
    int argumentCount() const;
    Argument *argumentAt(int index) const;

    // as Type
    QString prettyPrint() const;
    QString toString() const override;
    const Function *asFunctionType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;

    // as Symbol
    Function *asFunction() override { return this; }
    const Type *type() const override { return this; }

    Symbol *find(const QString &name) const override;

    QList<Symbol *> members() const override;
    void add(Symbol *symbol) override {
        if (! symbol)
            return;
        else if (Argument *arg = symbol->asArgument())
            addArgument(arg);
    }

private:
    const Type *_returnType;
    QVector<Argument *> _arguments;
};

class Q_QUICK3DGLSLPARSER_EXPORT SamplerType: public Type
{
public:
    explicit SamplerType(int kind) : _kind(kind) {}

    // Kind of sampler as a token code; e.g. T_SAMPLER2D.
    int kind() const { return _kind; }

    QString toString() const override;
    const SamplerType *asSamplerType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;

private:
    int _kind;
};

class Q_QUICK3DGLSLPARSER_EXPORT OverloadSet: public Type, public Scope
{
public:
    OverloadSet(Scope *enclosingScope = nullptr);

    QVector<Function *> functions() const;
    void addFunction(Function *function);

    // as symbol
    OverloadSet *asOverloadSet() override { return this; }
    const Type *type() const override;
    Symbol *find(const QString &name) const override;
    void add(Symbol *symbol) override;

    // as type
    QString toString() const override { return QLatin1String("overload"); }
    const OverloadSet *asOverloadSetType() const override { return this; }
    bool isEqualTo(const Type *other) const override;
    bool isLessThan(const Type *other) const override;

private:
    QVector<Function *> _functions;
};

} // namespace GLSL

QT_END_NAMESPACE

#endif // QSSG_GLSLTYPES_H
