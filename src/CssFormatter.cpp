#include "fmtpp/CssFormatter.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: CSS formatter - centered, with a 0 auto margin for error

CssFormatter::CssFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

CssFormatter::~CssFormatter()
{
}

void CssFormatter::visitClassDecl(ClassDeclNode& node)
{
    // CSS rule: selector { declarations }. Bases never occur; the name
    // carries the whole selector, including @media/@supports prefixes.
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void CssFormatter::visitProperty(PropertyNode& node)
{
    // CSS declaration: prop: value;
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    printToken(": ");
    if (node.m_value != NULL) { node.m_value->accept(*this); }
    printToken(";");
    printNewline();
}

void CssFormatter::visitImportDecl(ImportDeclNode& node)
{
    // CSS: @import "url";
    printToken("@import");
    printSpace();
    printToken("\"");
    printToken(node.m_source.c_str());
    printToken("\";");
    printNewline();
}

// Easter egg: CssFormatter complete - specificity: 1, 0, 0 for beauty
