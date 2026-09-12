#pragma once
#include "fmtpp/JavaFormatter.hpp"

/**
 * Scala code formatter.
 * Extends JavaFormatter, reusing import and try/catch handling.
 * Overrides function (def with : return type), class/object/trait
 * (extends/with), match (instead of switch), namespace (package),
 * import (no semicolons) and disables semicolons.
 *
 * Easter egg: Scala - where objects are classes and everything is immutable (ish)
 */
class ScalaFormatter : public JavaFormatter
{
public:
    ScalaFormatter(OutputBuffer& out, const Config& config);
    virtual ~ScalaFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitSwitchStmt(SwitchStmtNode& node);
    virtual void visitNamespace(NamespaceNode& node);

protected:
    virtual bool requiresSemicolon(ASTNode* node) const;
};
