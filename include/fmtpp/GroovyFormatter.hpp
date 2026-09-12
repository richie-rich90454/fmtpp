#pragma once
#include "fmtpp/JavaFormatter.hpp"

/**
 * Groovy code formatter.
 * Extends JavaFormatter, reusing switch and try/catch handling.
 * Overrides function (def, no public), class (no public),
 * namespace (package), import (no semicolons) and disables semicolons.
 *
 * Easter egg: Groovy - Java's cool younger sibling with a dynamic streak
 */
class GroovyFormatter : public JavaFormatter
{
public:
    GroovyFormatter(OutputBuffer& out, const Config& config);
    virtual ~GroovyFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitNamespace(NamespaceNode& node);

protected:
    virtual bool requiresSemicolon(ASTNode* node) const;
};
