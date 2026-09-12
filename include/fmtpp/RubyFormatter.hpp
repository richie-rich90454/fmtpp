#pragma once
#include "fmtpp/IndentFormatterBase.hpp"

/**
 * Ruby code formatter.
 * def/class/module with end suites, if/elsif/else/unless, while/until,
 * case/when, begin/rescue/ensure, do blocks, require. No semicolons.
 *
 * Easter egg: Ruby - optimized for programmer happiness, formatted for joy
 */
class RubyFormatter : public IndentFormatterBase
{
public:
    RubyFormatter(OutputBuffer& out, const Config& config);
    virtual ~RubyFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitNamespace(NamespaceNode& node);
    virtual void visitIfStmt(IfStmtNode& node);
    virtual void visitForStmt(ForStmtNode& node);
    virtual void visitWhileStmt(WhileStmtNode& node);
    virtual void visitSwitchStmt(SwitchStmtNode& node);
    virtual void visitTryCatchStmt(TryCatchStmtNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitBlock(BlockNode& node);
    virtual void visitLiteral(LiteralNode& node);

protected:
    void printIfChain(IfStmtNode& node);
};
