#pragma once
#include "fmtpp/CFormatter.hpp"

/**
 * PHP code formatter.
 * Extends CFormatter. Functions use the function keyword with optional
 * visibility modifiers, variables carry the $ sigil, use/namespace map
 * to import/namespace, and <?php passes through verbatim.
 *
 * Easter egg: PHP - Hypertext Preformatter, recursively
 */
class PhpFormatter : public CFormatter
{
public:
    PhpFormatter(OutputBuffer& out, const Config& config);
    virtual ~PhpFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitVariableDecl(VariableDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitNamespace(NamespaceNode& node);
};
