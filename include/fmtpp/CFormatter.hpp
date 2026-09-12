#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * C language code formatter.
 * Extends CstyleFormatterBase with C-style functions (return type
 * prefix, optional modifiers). Preprocessor lines pass through verbatim.
 * C doesn't have classes, namespaces, or templates.
 *
 * Easter egg: C - the language that started it all, still going strong
 */
class CFormatter : public CstyleFormatterBase
{
public:
    CFormatter(OutputBuffer& out, const Config& config);
    virtual ~CFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitEnumDecl(EnumDeclNode& node);
};