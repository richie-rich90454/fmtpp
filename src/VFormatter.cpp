#include "fmtpp/VFormatter.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include <cstdio>

// Easter egg: V formatter - formatting at the speed of C, with the soul of Go

VFormatter::VFormatter(OutputBuffer& out, const Config& config)
    : GoFormatter(out, config)
{
}

VFormatter::~VFormatter()
{
}

void VFormatter::visitVariableDeclNode(VariableDeclNode& node)
{
    if (node.m_kind == "mut" && node.m_initializer != NULL)
    {
        printToken("mut");
        printSpace();
        printToken(node.m_name.c_str());
        printToken(" := ");
        node.m_initializer->accept(*this);
        printNewline();
        return;
    }
    GoFormatter::visitVariableDeclNode(node);
}

void VFormatter::visitFunctionDeclNode(FunctionDeclNode& node)
{
    // V: fn name(params) Ret (Go spells the keyword func)
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
    if (!node.m_returnType.empty()) { printSpace(); printToken(node.m_returnType.c_str()); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void VFormatter::visitClassDeclNode(ClassDeclNode& node)
{
    // V: struct Name { ... } (not Go's type Name struct)
    printToken("struct");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void VFormatter::visitEnumDeclNode(EnumDeclNode& node)
{
    // V has real enums, not iota blocks
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
        printNewline();
    }
    dedent();
    printIndent();
    printToken("}");
    printNewline();
}

void VFormatter::visitNamespaceNode(NamespaceNode& node)
{
    // V: module name
    printToken("module");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printNewline(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void VFormatter::visitImportDeclNode(ImportDeclNode& node)
{
    // V imports are unquoted: import os
    printToken("import");
    if (node.m_source.empty())
    {
        printNewline();
        return;
    }
    printSpace();
    printToken(node.m_source.c_str());
    printNewline();
}

void VFormatter::visitPreprocessorDirectiveNode(PreprocessorDirectiveNode& node)
{
    // V #flag/#include lines pass through verbatim, not as go:build tags
    printToken(node.m_directive.c_str());
    printNewline();
}

void VFormatter::visitLiteralNode(LiteralNode& node)
{
    // ponytail: v fmt prefers single quotes; double quotes only when the
    // value holds an apostrophe (and never corrupts embedded doubles)
    if (node.m_type == LiteralNode::TYPE_STRING
        && node.m_value.find('\'') == std::string::npos)
    {
        m_out.printf("'%s'", node.m_value.c_str());
        m_currentColumn += static_cast<int>(node.m_value.size()) + 2;
        return;
    }
    GoFormatter::visitLiteralNode(node);
}

const char* VFormatter::switchKeyword() const
{
    // ponytail: V spells it match; printing switch would emit invalid V
    // that the next pass cannot even parse back (idempotency repair)
    return "match";
}

// Easter egg: VFormatter complete - v fmt . would be proud
