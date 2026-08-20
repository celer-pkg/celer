// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_GLSLAST_H
#define QSSG_GLSLAST_H

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
#include <QtQuick3DGlslParser/private/glslmemorypool_p.h>
#include <qstring.h>

QT_BEGIN_NAMESPACE

namespace GLSL {

class AST;
class TranslationUnitAST;
class ExpressionAST;
class IdentifierExpressionAST;
class LiteralExpressionAST;
class BinaryExpressionAST;
class UnaryExpressionAST;
class TernaryExpressionAST;
class AssignmentExpressionAST;
class MemberAccessExpressionAST;
class FunctionCallExpressionAST;
class FunctionIdentifierAST;
class DeclarationExpressionAST;
class StatementAST;
class ExpressionStatementAST;
class CompoundStatementAST;
class IfStatementAST;
class WhileStatementAST;
class DoStatementAST;
class ForStatementAST;
class JumpStatementAST;
class ReturnStatementAST;
class SwitchStatementAST;
class CaseLabelStatementAST;
class DeclarationStatementAST;
class TypeAST;
class BasicTypeAST;
class NamedTypeAST;
class ArrayTypeAST;
class StructTypeAST;
class LayoutQualifierAST;
class QualifiedTypeAST;
class DeclarationAST;
class PrecisionDeclarationAST;
class ParameterDeclarationAST;
class VariableDeclarationAST;
class TypeDeclarationAST;
class TypeAndVariableDeclarationAST;
class InvariantDeclarationAST;
class InitDeclarationAST;
class FunctionDeclarationAST;
class Visitor;

template <typename T>
class Q_QUICK3DGLSLPARSER_EXPORT List: public Managed
{
public:
    List(const T &value_)
        : value(value_), next(this), lineno(0) {}

    List(List *previous, const T &value_)
        : value(value_), lineno(0)
    {
        next = previous->next;
        previous->next = this;
    }

    List *finish()
    {
        List *head = next;
        next = nullptr;
        return head;
    }

    T value;
    List *next;
    int lineno;
};

// Append two lists, which are assumed to still be circular, pre-finish.
template <typename T>
List<T> *appendLists(List<T> *first, List<T> *second)
{
    if (!first)
        return second;
    else if (!second)
        return first;
    List<T> *firstHead = first->next;
    List<T> *secondHead = second->next;
    first->next = secondHead;
    second->next = firstHead;
    return second;
}

class Q_QUICK3DGLSLPARSER_EXPORT AST: public Managed
{
public:
    enum Kind {
        Kind_Undefined,

        // Translation unit
        Kind_TranslationUnit,

        // Primary expressions
        Kind_Identifier,
        Kind_Literal,

        // Unary expressions
        Kind_PreIncrement,
        Kind_PostIncrement,
        Kind_PreDecrement,
        Kind_PostDecrement,
        Kind_UnaryPlus,
        Kind_UnaryMinus,
        Kind_LogicalNot,
        Kind_BitwiseNot,

        // Binary expressions
        Kind_Plus,
        Kind_Minus,
        Kind_Multiply,
        Kind_Divide,
        Kind_Modulus,
        Kind_ShiftLeft,
        Kind_ShiftRight,
        Kind_Equal,
        Kind_NotEqual,
        Kind_LessThan,
        Kind_LessEqual,
        Kind_GreaterThan,
        Kind_GreaterEqual,
        Kind_LogicalAnd,
        Kind_LogicalOr,
        Kind_LogicalXor,
        Kind_BitwiseAnd,
        Kind_BitwiseOr,
        Kind_BitwiseXor,
        Kind_Comma,
        Kind_ArrayAccess,

        // Other expressions
        Kind_Conditional,
        Kind_MemberAccess,
        Kind_FunctionCall,
        Kind_MemberFunctionCall,
        Kind_FunctionIdentifier,
        Kind_DeclarationExpression,

        // Assignment expressions
        Kind_Assign,
        Kind_AssignPlus,
        Kind_AssignMinus,
        Kind_AssignMultiply,
        Kind_AssignDivide,
        Kind_AssignModulus,
        Kind_AssignShiftLeft,
        Kind_AssignShiftRight,
        Kind_AssignAnd,
        Kind_AssignOr,
        Kind_AssignXor,

        // Statements
        Kind_ExpressionStatement,
        Kind_CompoundStatement,
        Kind_If,
        Kind_While,
        Kind_Do,
        Kind_For,
        Kind_Break,
        Kind_Continue,
        Kind_Discard,
        Kind_Return,
        Kind_ReturnExpression,
        Kind_Switch,
        Kind_CaseLabel,
        Kind_DefaultLabel,
        Kind_DeclarationStatement,

        // Types
        Kind_BasicType,
        Kind_NamedType,
        Kind_ArrayType,
        Kind_OpenArrayType,
        Kind_StructType,
        Kind_AnonymousStructType,
        Kind_StructField,
        Kind_LayoutQualifier,
        Kind_QualifiedType,

        // Declarations
        Kind_PrecisionDeclaration,
        Kind_ParameterDeclaration,
        Kind_VariableDeclaration,
        Kind_TypeDeclaration,
        Kind_TypeAndVariableDeclaration,
        Kind_InvariantDeclaration,
        Kind_InitDeclaration,
        Kind_FunctionDeclaration
    };

    virtual TranslationUnitAST *asTranslationUnit() { return nullptr; }

    virtual ExpressionAST *asExpression() { return nullptr; }
    virtual IdentifierExpressionAST *asIdentifierExpression() { return nullptr; }
    virtual LiteralExpressionAST *asLiteralExpression() { return nullptr; }
    virtual BinaryExpressionAST *asBinaryExpression() { return nullptr; }
    virtual UnaryExpressionAST *asUnaryExpression() { return nullptr; }
    virtual TernaryExpressionAST *asTernaryExpression() { return nullptr; }
    virtual AssignmentExpressionAST *asAssignmentExpression() { return nullptr; }
    virtual MemberAccessExpressionAST *asMemberAccessExpression() { return nullptr; }
    virtual FunctionCallExpressionAST *asFunctionCallExpression() { return nullptr; }
    virtual FunctionIdentifierAST *asFunctionIdentifier() { return nullptr; }
    virtual DeclarationExpressionAST *asDeclarationExpression() { return nullptr; }

    virtual StatementAST *asStatement() { return nullptr; }
    virtual ExpressionStatementAST *asExpressionStatement() { return nullptr; }
    virtual CompoundStatementAST *asCompoundStatement() { return nullptr; }
    virtual IfStatementAST *asIfStatement() { return nullptr; }
    virtual WhileStatementAST *asWhileStatement() { return nullptr; }
    virtual DoStatementAST *asDoStatement() { return nullptr; }
    virtual ForStatementAST *asForStatement() { return nullptr; }
    virtual JumpStatementAST *asJumpStatement() { return nullptr; }
    virtual ReturnStatementAST *asReturnStatement() { return nullptr; }
    virtual SwitchStatementAST *asSwitchStatement() { return nullptr; }
    virtual CaseLabelStatementAST *asCaseLabelStatement() { return nullptr; }
    virtual DeclarationStatementAST *asDeclarationStatement() { return nullptr; }

    virtual TypeAST *asType() { return nullptr; }
    virtual BasicTypeAST *asBasicType() { return nullptr; }
    virtual NamedTypeAST *asNamedType() { return nullptr; }
    virtual ArrayTypeAST *asArrayType() { return nullptr; }
    virtual StructTypeAST *asStructType() { return nullptr; }
    virtual QualifiedTypeAST *asQualifiedType() { return nullptr; }
    virtual LayoutQualifierAST *asLayoutQualifier() { return nullptr; }

    virtual DeclarationAST *asDeclaration() { return nullptr; }
    virtual PrecisionDeclarationAST *asPrecisionDeclaration() { return nullptr; }
    virtual ParameterDeclarationAST *asParameterDeclaration() { return nullptr; }
    virtual VariableDeclarationAST *asVariableDeclaration() { return nullptr; }
    virtual TypeDeclarationAST *asTypeDeclaration() { return nullptr; }
    virtual TypeAndVariableDeclarationAST *asTypeAndVariableDeclaration() { return nullptr; }
    virtual InvariantDeclarationAST *asInvariantDeclaration() { return nullptr; }
    virtual InitDeclarationAST *asInitDeclaration() { return nullptr; }
    virtual FunctionDeclarationAST *asFunctionDeclaration() { return nullptr; }

    void accept(Visitor *visitor);
    static void accept(AST *ast, Visitor *visitor);

    template <typename T>
    static void accept(List<T> *it, Visitor *visitor)
    {
        for (; it; it = it->next)
            accept(it->value, visitor);
    }

    virtual void accept0(Visitor *visitor) = 0;

protected:
    AST(Kind _kind) : kind(_kind), lineno(0) {}

    template <typename T>
    static List<T> *finish(List<T> *list)
    {
        if (! list)
            return nullptr;
        return list->finish(); // convert the circular list with a linked list.
    }

public: // attributes
    int kind;
    int lineno;

protected:
    ~AST() override {}       // Managed types cannot be deleted.
};

class Q_QUICK3DGLSLPARSER_EXPORT TranslationUnitAST: public AST
{
public:
    TranslationUnitAST(List<DeclarationAST *> *declarations_)
        : AST(Kind_TranslationUnit), declarations(finish(declarations_)) {}

    TranslationUnitAST *asTranslationUnit() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    List<DeclarationAST *> *declarations;
};

class Q_QUICK3DGLSLPARSER_EXPORT ExpressionAST: public AST
{
protected:
    ExpressionAST(Kind _kind) : AST(_kind) {}

public:
    ExpressionAST *asExpression() override { return this; }
};

class Q_QUICK3DGLSLPARSER_EXPORT IdentifierExpressionAST: public ExpressionAST
{
public:
    IdentifierExpressionAST(const QString *_name)
        : ExpressionAST(Kind_Identifier), name(_name) {}

    IdentifierExpressionAST *asIdentifierExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    const QString *name;
};

class Q_QUICK3DGLSLPARSER_EXPORT LiteralExpressionAST: public ExpressionAST
{
public:
    LiteralExpressionAST(const QString *_value)
        : ExpressionAST(Kind_Literal), value(_value) {}

    LiteralExpressionAST *asLiteralExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    const QString *value;
};

class Q_QUICK3DGLSLPARSER_EXPORT BinaryExpressionAST: public ExpressionAST
{
public:
    BinaryExpressionAST(Kind _kind, ExpressionAST *_left, ExpressionAST *_right)
        : ExpressionAST(_kind), left(_left), right(_right) {}

    BinaryExpressionAST *asBinaryExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *left;
    ExpressionAST *right;
};

class Q_QUICK3DGLSLPARSER_EXPORT UnaryExpressionAST: public ExpressionAST
{
public:
    UnaryExpressionAST(Kind _kind, ExpressionAST *_expr)
        : ExpressionAST(_kind), expr(_expr) {}

    UnaryExpressionAST *asUnaryExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *expr;
};

class Q_QUICK3DGLSLPARSER_EXPORT TernaryExpressionAST: public ExpressionAST
{
public:
    TernaryExpressionAST(Kind _kind, ExpressionAST *_first, ExpressionAST *_second, ExpressionAST *_third)
        : ExpressionAST(_kind), first(_first), second(_second), third(_third) {}

    TernaryExpressionAST *asTernaryExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *first;
    ExpressionAST *second;
    ExpressionAST *third;
};

class Q_QUICK3DGLSLPARSER_EXPORT AssignmentExpressionAST: public ExpressionAST
{
public:
    AssignmentExpressionAST(Kind _kind, ExpressionAST *_variable, ExpressionAST *_value)
        : ExpressionAST(_kind), variable(_variable), value(_value) {}

    AssignmentExpressionAST *asAssignmentExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *variable;
    ExpressionAST *value;
};

class Q_QUICK3DGLSLPARSER_EXPORT MemberAccessExpressionAST: public ExpressionAST
{
public:
    MemberAccessExpressionAST(ExpressionAST *_expr, const QString *_field)
        : ExpressionAST(Kind_MemberAccess), expr(_expr), field(_field) {}

    MemberAccessExpressionAST *asMemberAccessExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *expr;
    const QString *field;
};

class Q_QUICK3DGLSLPARSER_EXPORT FunctionCallExpressionAST: public ExpressionAST
{
public:
    FunctionCallExpressionAST(FunctionIdentifierAST *_id,
                           List<ExpressionAST *> *_arguments)
        : ExpressionAST(Kind_FunctionCall), expr(nullptr), id(_id)
        , arguments(finish(_arguments)) {}
    FunctionCallExpressionAST(ExpressionAST *_expr, FunctionIdentifierAST *_id,
                           List<ExpressionAST *> *_arguments)
        : ExpressionAST(Kind_MemberFunctionCall), expr(_expr), id(_id)
        , arguments(finish(_arguments)) {}

    FunctionCallExpressionAST *asFunctionCallExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *expr;
    FunctionIdentifierAST *id;
    List<ExpressionAST *> *arguments;
};

class Q_QUICK3DGLSLPARSER_EXPORT FunctionIdentifierAST: public AST
{
public:
    FunctionIdentifierAST(const QString *_name)
        : AST(Kind_FunctionIdentifier), name(_name), type(nullptr) {}
    FunctionIdentifierAST(TypeAST *_type)
        : AST(Kind_FunctionIdentifier), name(nullptr), type(_type) {}

    FunctionIdentifierAST *asFunctionIdentifier() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    const QString *name;
    TypeAST *type;
};

class Q_QUICK3DGLSLPARSER_EXPORT DeclarationExpressionAST: public ExpressionAST
{
public:
    DeclarationExpressionAST(TypeAST *_type, const QString *_name,
                          ExpressionAST *_initializer)
        : ExpressionAST(Kind_DeclarationExpression), type(_type)
        , name(_name), initializer(_initializer) {}

    DeclarationExpressionAST *asDeclarationExpression() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    TypeAST *type;
    const QString *name;
    ExpressionAST *initializer;
};

class Q_QUICK3DGLSLPARSER_EXPORT StatementAST: public AST
{
protected:
    StatementAST(Kind _kind) : AST(_kind) {}

public:
    StatementAST *asStatement() override { return this; }
};

class Q_QUICK3DGLSLPARSER_EXPORT ExpressionStatementAST: public StatementAST
{
public:
    ExpressionStatementAST(ExpressionAST *_expr)
        : StatementAST(Kind_ExpressionStatement), expr(_expr) {}

    ExpressionStatementAST *asExpressionStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *expr;
};

class Q_QUICK3DGLSLPARSER_EXPORT CompoundStatementAST: public StatementAST
{
public:
    CompoundStatementAST()
        : StatementAST(Kind_CompoundStatement), statements(nullptr)
        , start(0), end(0), symbol(nullptr) {}
    CompoundStatementAST(List<StatementAST *> *_statements)
        : StatementAST(Kind_CompoundStatement), statements(finish(_statements))
        , start(0), end(0), symbol(nullptr) {}

    CompoundStatementAST *asCompoundStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    List<StatementAST *> *statements;
    int start;
    int end;
    Block *symbol; // decoration
};

class Q_QUICK3DGLSLPARSER_EXPORT IfStatementAST: public StatementAST
{
public:
    IfStatementAST(ExpressionAST *_condition, StatementAST *_thenClause, StatementAST *_elseClause)
        : StatementAST(Kind_If), condition(_condition)
        , thenClause(_thenClause), elseClause(_elseClause) {}

    IfStatementAST *asIfStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *condition;
    StatementAST *thenClause;
    StatementAST *elseClause;
};

class Q_QUICK3DGLSLPARSER_EXPORT WhileStatementAST: public StatementAST
{
public:
    WhileStatementAST(ExpressionAST *_condition, StatementAST *_body)
        : StatementAST(Kind_While), condition(_condition), body(_body) {}

    WhileStatementAST *asWhileStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *condition;
    StatementAST *body;
};

class Q_QUICK3DGLSLPARSER_EXPORT DoStatementAST: public StatementAST
{
public:
    DoStatementAST(StatementAST *_body, ExpressionAST *_condition)
        : StatementAST(Kind_Do), body(_body), condition(_condition) {}

    DoStatementAST *asDoStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    StatementAST *body;
    ExpressionAST *condition;
};

class Q_QUICK3DGLSLPARSER_EXPORT ForStatementAST: public StatementAST
{
public:
    ForStatementAST(StatementAST *_init, ExpressionAST *_condition, ExpressionAST *_increment, StatementAST *_body)
        : StatementAST(Kind_For), init(_init), condition(_condition), increment(_increment), body(_body) {}

    ForStatementAST *asForStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    StatementAST *init;
    ExpressionAST *condition;
    ExpressionAST *increment;
    StatementAST *body;
};

class Q_QUICK3DGLSLPARSER_EXPORT JumpStatementAST: public StatementAST
{
public:
    JumpStatementAST(Kind _kind) : StatementAST(_kind) {}

    JumpStatementAST *asJumpStatement() override { return this; }

    void accept0(Visitor *visitor) override;
};

class Q_QUICK3DGLSLPARSER_EXPORT ReturnStatementAST: public StatementAST
{
public:
    ReturnStatementAST() : StatementAST(Kind_Return), expr(nullptr) {}
    ReturnStatementAST(ExpressionAST *_expr)
        : StatementAST(Kind_ReturnExpression), expr(_expr) {}

    ReturnStatementAST *asReturnStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *expr;
};

class Q_QUICK3DGLSLPARSER_EXPORT SwitchStatementAST: public StatementAST
{
public:
    SwitchStatementAST(ExpressionAST *_expr, StatementAST *_body)
        : StatementAST(Kind_Switch), expr(_expr), body(_body) {}

    SwitchStatementAST *asSwitchStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *expr;
    StatementAST *body;
};

class Q_QUICK3DGLSLPARSER_EXPORT CaseLabelStatementAST: public StatementAST
{
public:
    CaseLabelStatementAST() : StatementAST(Kind_DefaultLabel), expr(nullptr) {}
    CaseLabelStatementAST(ExpressionAST *_expr)
        : StatementAST(Kind_CaseLabel), expr(_expr) {}

    CaseLabelStatementAST *asCaseLabelStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    ExpressionAST *expr;
};

class Q_QUICK3DGLSLPARSER_EXPORT DeclarationStatementAST: public StatementAST
{
public:
    DeclarationStatementAST(DeclarationAST *_decl)
        : StatementAST(Kind_DeclarationStatement), decl(_decl) {}

    DeclarationStatementAST *asDeclarationStatement() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    DeclarationAST *decl;
};

class Q_QUICK3DGLSLPARSER_EXPORT TypeAST: public AST
{
protected:
    TypeAST(Kind _kind) : AST(_kind) {}

public:
    enum Precision
    {
        PrecNotValid,       // Precision not valid (e.g. structs).
        PrecUnspecified,    // Precision not known, but can be validly set.
        Lowp,
        Mediump,
        Highp
    };

    TypeAST *asType() override { return this; }

    virtual Precision precision() const = 0;

    // Set the precision for the innermost basic type.  Returns false if it
    // is not valid to set a precision (e.g. structs).
    virtual bool setPrecision(Precision precision) = 0;
};

class Q_QUICK3DGLSLPARSER_EXPORT BasicTypeAST: public TypeAST
{
public:
    // Pass the parser's token code: T_VOID, T_VEC4, etc.
    BasicTypeAST(int _token, const char *_name);

    BasicTypeAST *asBasicType() override { return this; }

    void accept0(Visitor *visitor) override;

    Precision precision() const override;
    bool setPrecision(Precision precision) override;

public: // attributes
    Precision prec;
    int token;
    const char *name;
};

class Q_QUICK3DGLSLPARSER_EXPORT NamedTypeAST: public TypeAST
{
public:
    NamedTypeAST(const QString *_name) : TypeAST(Kind_NamedType), name(_name) {}

    NamedTypeAST *asNamedType() override { return this; }

    void accept0(Visitor *visitor) override;

    Precision precision() const override;
    bool setPrecision(Precision precision) override;

public: // attributes
    const QString *name;
};

class Q_QUICK3DGLSLPARSER_EXPORT ArrayTypeAST: public TypeAST
{
public:
    ArrayTypeAST(TypeAST *_elementType)
        : TypeAST(Kind_OpenArrayType), elementType(_elementType), size(nullptr) {}
    ArrayTypeAST(TypeAST *_elementType, ExpressionAST *_size)
        : TypeAST(Kind_ArrayType), elementType(_elementType), size(_size) {}

    ArrayTypeAST *asArrayType() override { return this; }

    void accept0(Visitor *visitor) override;

    Precision precision() const override;
    bool setPrecision(Precision precision) override;

public: // attributes
    TypeAST *elementType;
    ExpressionAST *size;
};

class Q_QUICK3DGLSLPARSER_EXPORT StructTypeAST: public TypeAST
{
public:
    class Field: public AST
    {
    public:
        Field(const QString *_name)
            : AST(Kind_StructField), name(_name), type(nullptr) {}

        // Takes the outer shell of an array type with the innermost
        // element type set to null.  The fixInnerTypes() function will
        // set the innermost element type to a meaningful value.
        Field(const QString *_name, TypeAST *_type)
            : AST(Kind_StructField), name(_name), type(_type) {}

        void accept0(Visitor *visitor) override;

        void setInnerType(TypeAST *innerType);

        const QString *name;
        TypeAST *type;
    };

    StructTypeAST(List<Field *> *_fields)
        : TypeAST(Kind_AnonymousStructType), name(nullptr), fields(finish(_fields)) {}
    StructTypeAST(const QString *_name, List<Field *> *_fields)
        : TypeAST(Kind_StructType), name(_name), fields(finish(_fields)) {}

    StructTypeAST *asStructType() override { return this; }

    void accept0(Visitor *visitor) override;

    Precision precision() const override;
    bool setPrecision(Precision precision) override;

    // Fix the inner types of a field list.  The "innerType" will
    // be copied into the "array holes" of all fields.
    static List<Field *> *fixInnerTypes(TypeAST *innerType, List<Field *> *fields);

public: // attributes
    const QString *name;
    List<Field *> *fields;
};

class Q_QUICK3DGLSLPARSER_EXPORT LayoutQualifierAST: public AST
{
public:
    LayoutQualifierAST(const QString *_name, const QString *_number)
        : AST(Kind_LayoutQualifier), name(_name), number(_number) {}

    LayoutQualifierAST *asLayoutQualifier() override { return this; }
    void accept0(Visitor *visitor) override;

public: // attributes
    const QString *name;
    const QString *number;
};

class Q_QUICK3DGLSLPARSER_EXPORT QualifiedTypeAST: public TypeAST
{
public:
    QualifiedTypeAST(int _qualifiers, TypeAST *_type, List<LayoutQualifierAST *> *_layout_list)
        : TypeAST(Kind_QualifiedType), qualifiers(_qualifiers), type(_type)
        , layout_list(finish(_layout_list)) {}

    enum
    {
        StorageMask         = 0x000000FF,
        NoStorage           = 0x00000000,
        Const               = 0x00000001,
        Attribute           = 0x00000002,
        Varying             = 0x00000003,
        CentroidVarying     = 0x00000004,
        In                  = 0x00000005,
        Out                 = 0x00000006,
        CentroidIn          = 0x00000007,
        CentroidOut         = 0x00000008,
        PatchIn             = 0x00000009,
        PatchOut            = 0x0000000A,
        SampleIn            = 0x0000000B,
        SampleOut           = 0x0000000C,
        Uniform             = 0x0000000D,
        InterpolationMask   = 0x00000F00,
        NoInterpolation     = 0x00000000,
        Smooth              = 0x00000100,
        Flat                = 0x00000200,
        NoPerspective       = 0x00000300,
        Invariant           = 0x00010000,
        Struct              = 0x00020000
    };

    QualifiedTypeAST *asQualifiedType() override { return this; }

    void accept0(Visitor *visitor) override;

    Precision precision() const override { return type->precision(); }
    bool setPrecision(Precision precision) override { return type->setPrecision(precision); }

public: // attributes
    int qualifiers;
    TypeAST *type;
    List<LayoutQualifierAST *> *layout_list;
};

class Q_QUICK3DGLSLPARSER_EXPORT DeclarationAST: public AST
{
protected:
    DeclarationAST(Kind _kind) : AST(_kind) {}

public:
    DeclarationAST *asDeclaration() override { return this; }
};

class Q_QUICK3DGLSLPARSER_EXPORT PrecisionDeclarationAST: public DeclarationAST
{
public:
    PrecisionDeclarationAST(TypeAST::Precision _precision, TypeAST *_type)
        : DeclarationAST(Kind_PrecisionDeclaration)
        , precision(_precision), type(_type) {}

    PrecisionDeclarationAST *asPrecisionDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    TypeAST::Precision precision;
    TypeAST *type;
};

class Q_QUICK3DGLSLPARSER_EXPORT ParameterDeclarationAST: public DeclarationAST
{
public:
    enum Qualifier
    {
        In,
        Out,
        InOut
    };
    ParameterDeclarationAST(TypeAST *_type, Qualifier _qualifier,
                         const QString *_name)
        : DeclarationAST(Kind_ParameterDeclaration), type(_type)
        , qualifier(_qualifier), name(_name) {}

    ParameterDeclarationAST *asParameterDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    TypeAST *type;
    Qualifier qualifier;
    const QString *name;
};

class Q_QUICK3DGLSLPARSER_EXPORT VariableDeclarationAST: public DeclarationAST
{
public:
    VariableDeclarationAST(TypeAST *_type, const QString *_name,
                        ExpressionAST *_initializer = nullptr)
        : DeclarationAST(Kind_VariableDeclaration), type(_type)
        , name(_name), initializer(_initializer) {}

    VariableDeclarationAST *asVariableDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

    static TypeAST *declarationType(List<DeclarationAST *> *decls);

public: // attributes
    TypeAST *type;
    const QString *name;
    ExpressionAST *initializer;
};

class Q_QUICK3DGLSLPARSER_EXPORT TypeDeclarationAST: public DeclarationAST
{
public:
    TypeDeclarationAST(TypeAST *_type)
        : DeclarationAST(Kind_TypeDeclaration), type(_type) {}

    TypeDeclarationAST *asTypeDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    TypeAST *type;
};

class Q_QUICK3DGLSLPARSER_EXPORT TypeAndVariableDeclarationAST: public DeclarationAST
{
public:
    TypeAndVariableDeclarationAST(TypeDeclarationAST *_typeDecl,
                               VariableDeclarationAST *_varDecl)
        : DeclarationAST(Kind_TypeAndVariableDeclaration)
        , typeDecl(_typeDecl), varDecl(_varDecl) {}

    TypeAndVariableDeclarationAST *asTypeAndVariableDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    TypeDeclarationAST *typeDecl;
    VariableDeclarationAST *varDecl;
};

class Q_QUICK3DGLSLPARSER_EXPORT InvariantDeclarationAST: public DeclarationAST
{
public:
    InvariantDeclarationAST(const QString *_name)
        : DeclarationAST(Kind_InvariantDeclaration), name(_name) {}

    InvariantDeclarationAST *asInvariantDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    const QString *name;
};

class Q_QUICK3DGLSLPARSER_EXPORT InitDeclarationAST: public DeclarationAST
{
public:
    InitDeclarationAST(List<DeclarationAST *> *_decls)
        : DeclarationAST(Kind_InitDeclaration), decls(finish(_decls)) {}

    InitDeclarationAST *asInitDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

public: // attributes
    List<DeclarationAST *> *decls;
};

class Q_QUICK3DGLSLPARSER_EXPORT FunctionDeclarationAST : public DeclarationAST
{
public:
    FunctionDeclarationAST(TypeAST *_returnType, const QString *_name)
        : DeclarationAST(Kind_FunctionDeclaration), returnType(_returnType)
        , name(_name), params(nullptr), body(nullptr) {}

    FunctionDeclarationAST *asFunctionDeclaration() override { return this; }

    void accept0(Visitor *visitor) override;

    void finishParams() { params = finish(params); }

    bool isPrototype() const { return body == nullptr; }

public: // attributes
    TypeAST *returnType;
    const QString *name;
    List<ParameterDeclarationAST *> *params;
    StatementAST *body;
};

} // namespace GLSL

QT_END_NAMESPACE

#endif // QSSG_GLSLAST_H
