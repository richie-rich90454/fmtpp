#include "fmtpp/TSFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: TypeScript formatter - where types meet JavaScript

TSFormatter::TSFormatter(OutputBuffer& out, const Config& config)
    : JSFormatter(out, config)
{
}

TSFormatter::~TSFormatter()
{
}

void TSFormatter::visitFunctionDeclNode(FunctionDeclNode& node)
{
    printToken("function");
    if (spaceAfterKeywords()) { printSpace(); }
    printToken(node.m_name.c_str());
    printToken("(");
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    printToken(")");
    if (!node.m_returnType.empty()) { printToken(": "); printToken(node.m_returnType.c_str()); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printSemicolon(); printNewline(); }
}

void TSFormatter::visitInterfaceDeclNode(InterfaceDeclNode& node)
{
    printToken("interface");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_extends.size() > 0)
    {
        printToken(" extends ");
        size_t ec = node.m_extends.size();
        for (size_t i = 0; i < ec; i++)
        {
            if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
            if (i < ec - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void TSFormatter::visitEnumDeclNode(EnumDeclNode& node)
{
    printToken("enum");
    printSpace();
    printToken(node.m_name.c_str());
    printOpeningBrace();
    printNewline();
    indent();
    size_t mc = node.m_members.size();
    for (size_t i = 0; i < mc; i++)
    {
        printIndent();
        if (node.m_members[i] != NULL) { node.m_members[i]->accept(*this); }
        if (i < mc - 1) { printToken(","); }
        printNewline();
    }
    dedent();
    printIndent();
    printClosingBrace();
}

void TSFormatter::visitTypeAliasNode(TypeAliasNode& node)
{
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    printToken(" = ");
    if (node.m_typeAnnotation != NULL) { node.m_typeAnnotation->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

// Easter egg: TSFormatter complete - TypeScript: because JavaScript needed to grow up
