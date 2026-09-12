#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * CSS (and SCSS/Less, flat subset) formatter.
 * Rules print as selector-plus-block, declarations as properties
 * with semicolons, @import with quotes. Nested rules indent naturally.
 *
 * Easter egg: CSS - !importantly well formatted
 */
class CssFormatter : public CstyleFormatterBase
{
public:
    CssFormatter(OutputBuffer& out, const Config& config);
    virtual ~CssFormatter();

    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitProperty(PropertyNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
};
