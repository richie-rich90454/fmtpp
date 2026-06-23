#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * Java code formatter.
 * Extends CstyleFormatterBase with Java-specific formatting:
 * public/private/protected modifiers, class extends/implements,
 * import statements, switch/case with break, try/catch/finally.
 *
 * Easter egg: Java - where everything is an object, except the primitives
 */
class JavaFormatter : public CstyleFormatterBase
{
public:
    JavaFormatter(OutputBuffer& out, const Config& config);
    virtual ~JavaFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitSwitchStmt(SwitchStmtNode& node);
    virtual void visitTryCatchStmt(TryCatchStmtNode& node);
};