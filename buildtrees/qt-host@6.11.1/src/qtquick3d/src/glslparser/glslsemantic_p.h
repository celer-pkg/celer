// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_GLSLSEMANTIC_H
#define QSSG_GLSLSEMANTIC_H

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

#include <QtQuick3DGlslParser/private/glslastvisitor_p.h>
#include <QtQuick3DGlslParser/private/glsltype_p.h>

QT_BEGIN_NAMESPACE

namespace GLSL {

class Q_QUICK3DGLSLPARSER_EXPORT Semantic: protected Visitor
{
public:
    Semantic();
    ~Semantic() override;

    struct ExprResult {
        ExprResult(const Type *type_ = nullptr, bool isConstant_ = false)
            : type(type_), isConstant(isConstant_) {}

        ~ExprResult() { }

        bool isValid() const {
            if (! type)
                return false;
            else if (type->asUndefinedType() != nullptr)
                return false;
            return true;
        }

        explicit operator bool() const { return isValid(); }

        const Type *type;
        bool isConstant;
    };

    void translationUnit(TranslationUnitAST *ast, Scope *globalScope, Engine *engine);
    ExprResult expression(ExpressionAST *ast, Scope *scope, Engine *engine);

protected:
    Engine *switchEngine(Engine *engine);
    Scope *switchScope(Scope *scope);

    bool implicitCast(const Type *type, const Type *target) const;

    ExprResult expression(ExpressionAST *ast);
    void statement(StatementAST *ast);
    const Type *type(TypeAST *ast);
    void declaration(DeclarationAST *ast);
    ExprResult functionIdentifier(FunctionIdentifierAST *ast);
    Symbol *field(StructTypeAST::Field *ast);
    void parameterDeclaration(ParameterDeclarationAST *ast, Function *fun);

    bool visit(TranslationUnitAST *ast) override;
    bool visit(FunctionIdentifierAST *ast) override;
    bool visit(StructTypeAST::Field *ast) override;

    // expressions
    bool visit(IdentifierExpressionAST *ast) override;
    bool visit(LiteralExpressionAST *ast) override;
    bool visit(BinaryExpressionAST *ast) override;
    bool visit(UnaryExpressionAST *ast) override;
    bool visit(TernaryExpressionAST *ast) override;
    bool visit(AssignmentExpressionAST *ast) override;
    bool visit(MemberAccessExpressionAST *ast) override;
    bool visit(FunctionCallExpressionAST *ast) override;
    bool visit(DeclarationExpressionAST *ast) override;

    // statements
    bool visit(ExpressionStatementAST *ast) override;
    bool visit(CompoundStatementAST *ast) override;
    bool visit(IfStatementAST *ast) override;
    bool visit(WhileStatementAST *ast) override;
    bool visit(DoStatementAST *ast) override;
    bool visit(ForStatementAST *ast) override;
    bool visit(JumpStatementAST *ast) override;
    bool visit(ReturnStatementAST *ast) override;
    bool visit(SwitchStatementAST *ast) override;
    bool visit(CaseLabelStatementAST *ast) override;
    bool visit(DeclarationStatementAST *ast) override;

    // types
    bool visit(BasicTypeAST *ast) override;
    bool visit(NamedTypeAST *ast) override;
    bool visit(ArrayTypeAST *ast) override;
    bool visit(StructTypeAST *ast) override;
    bool visit(QualifiedTypeAST *ast) override;
    bool visit(LayoutQualifierAST *ast) override { return Visitor::visit(ast); }

    // declarations
    bool visit(PrecisionDeclarationAST *ast) override;
    bool visit(ParameterDeclarationAST *ast) override;
    bool visit(VariableDeclarationAST *ast) override;
    bool visit(TypeDeclarationAST *ast) override;
    bool visit(TypeAndVariableDeclarationAST *ast) override;
    bool visit(InvariantDeclarationAST *ast) override;
    bool visit(InitDeclarationAST *ast) override;
    bool visit(FunctionDeclarationAST *ast) override;

private:
    Engine *_engine;
    Scope *_scope;
    const Type *_type;
    ExprResult _expr;
};

} // namespace GLSL

QT_END_NAMESPACE

#endif // QSSG_GLSLSEMANTIC_H
