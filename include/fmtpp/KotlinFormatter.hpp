#pragma once
#include "fmtpp/JavaFormatter.hpp"

/**
 * Kotlin code formatter.
 * Extends JavaFormatter, reusing import and try/catch handling.
 * Overrides function (fun with : return type), class (no public,
 * : bases), when (no parens-to-switch rewrite), namespace (package),
 * import (no semicolons) and disables semicolons.
 *
 * Easter egg: Kotlin - concise, safe, and null-pointer-free (mostly)
 */
class KotlinFormatter : public JavaFormatter
{
public:
    KotlinFormatter(OutputBuffer& out, const Config& config);
    virtual ~KotlinFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitSwitchStmt(SwitchStmtNode& node);
    virtual void visitNamespace(NamespaceNode& node);

protected:
    virtual bool requiresSemicolon(ASTNode* node) const;
};
