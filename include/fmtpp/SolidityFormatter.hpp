#pragma once
#include "fmtpp/JSFormatter.hpp"

/**
 * Solidity code formatter.
 * Extends JSFormatter (function keyword, semicolons). Contracts print
 * with is-bases, functions carry modifiers and returns clauses,
 * imports are quoted paths, pragmas pass through verbatim.
 *
 * Easter egg: Solidity - formatting contracts you can trust (audited for beauty)
 */
class SolidityFormatter : public JSFormatter
{
public:
    SolidityFormatter(OutputBuffer& out, const Config& config);
    virtual ~SolidityFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
};
