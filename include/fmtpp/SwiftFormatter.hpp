#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * Swift code formatter.
 * Func/let/var with -> return types, class/struct/actor/enum,
 * import without semicolons. No semicolons anywhere.
 *
 * Easter egg: Swift - fast, safe, and missing only the semicolons
 */
class SwiftFormatter : public CstyleFormatterBase
{
public:
    SwiftFormatter(OutputBuffer& out, const Config& config);
    virtual ~SwiftFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitIfStmt(IfStmtNode& node);
    virtual void visitWhileStmt(WhileStmtNode& node);
    virtual void visitForStmt(ForStmtNode& node);
    virtual void visitSwitchStmt(SwitchStmtNode& node);

protected:
    virtual bool requiresSemicolon(ASTNode* node) const;
};
