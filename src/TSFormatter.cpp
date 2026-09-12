#include "fmtpp/TSFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
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
    // ponytail: export/async ride in m_modifiers so nothing is dropped
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    // ponytail: constructors print bare (see JSFormatter)
    if (!node.m_modifiers.empty() || node.m_name != "constructor")
    {
        printToken("function");
        if (spaceAfterKeywords()) { printSpace(); }
    }
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

void TSFormatter::visitVariableDeclNode(VariableDeclNode& node)
{
    // TS: kind name[: Type] [= init]; — the type must never be dropped
    if (!node.m_kind.empty())
    {
        printToken(node.m_kind.c_str());
        printSpace();
    }
    printToken(node.m_name.c_str());
    if (!node.m_type.empty())
    {
        printToken(": ");
        printToken(node.m_type.c_str());
    }
    if (node.m_initializer != NULL) { printToken(" = "); node.m_initializer->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void TSFormatter::visitPropertyNode(PropertyNode& node)
{
    // TS interface/object-type member: key[: Type];
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    if (node.m_value != NULL)
    {
        printToken(": ");
        node.m_value->accept(*this);
    }
    printSemicolonIfNeeded(&node);
    printNewline();
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
    // ponytail: modifiers ride in the kind ("const enum"); bare and plain
    // "enum" print the keyword, anything else prints verbatim
    if (!node.m_kind.empty() && node.m_kind != "enum")
    {
        printToken(node.m_kind.c_str());
    }
    else
    {
        printToken("enum");
    }
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
