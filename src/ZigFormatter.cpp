#include "fmtpp/ZigFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: Zig formatter - every error set handled, even formatting ones

ZigFormatter::ZigFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

ZigFormatter::~ZigFormatter()
{
}

void ZigFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // Zig: [modifiers] fn name(params) [ReturnType] { body }
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    printToken("fn");
    printSpace();
    printToken(node.m_name.c_str());
    printToken("(");
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    printToken(")");
    if (!node.m_returnType.empty())
    {
        printSpace();
        printToken(node.m_returnType.c_str());
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void ZigFormatter::visitClassDecl(ClassDeclNode& node)
{
    // Zig: const Name = struct|enum|union [(...)] { body }
    printToken("const");
    printSpace();
    printToken(node.m_name.c_str());
    printToken(" = ");
    printToken(node.m_kind.empty() ? "struct" : node.m_kind.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void ZigFormatter::visitImportDecl(ImportDeclNode& node)
{
    // Zig: const name = @import("path");
    if (node.m_defaultImport != NULL)
    {
        printToken("const");
        printSpace();
        node.m_defaultImport->accept(*this);
        printToken(" = ");
    }
    printToken("@import(\"");
    printToken(node.m_source.c_str());
    printToken("\");");
    printNewline();
}

void ZigFormatter::visitForStmt(ForStmtNode& node)
{
    // Zig: for (items) |capture| { body } — the condition stays raw
    printToken("for");
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// Easter egg: ZigFormatter complete - defer complaining about formatting
