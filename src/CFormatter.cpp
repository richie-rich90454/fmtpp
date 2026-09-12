#include "fmtpp/CFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"

// Easter egg: C - the language that started it all

CFormatter::CFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

CFormatter::~CFormatter()
{
}

void CFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // C function: [modifiers] returnType name(params) { body }
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    if (!node.m_returnType.empty())
    {
        printToken(node.m_returnType.c_str());
        printSpace();
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
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void CFormatter::visitEnumDecl(EnumDeclNode& node)
{
    // C enum: enum Name { members }; — the trailing semicolon is required
    printToken("enum");
    printSpace();
    printToken(node.m_name.c_str());
    printSpace();
    printToken("{");
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
    printToken("}");
    printToken(";");
    printNewline();
}