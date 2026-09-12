#include "fmtpp/PhpFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include <cstdio>

// Easter egg: PHP formatter - it works, and that's no illusion (just concatenation)

PhpFormatter::PhpFormatter(OutputBuffer& out, const Config& config)
    : CFormatter(out, config)
{
}

PhpFormatter::~PhpFormatter()
{
}

void PhpFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // PHP: [modifiers] function name(params) { body }
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    printToken("function");
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
        printToken(": ");
        printToken(node.m_returnType.c_str());
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void PhpFormatter::visitVariableDecl(VariableDeclNode& node)
{
    // PHP: [Type] $name [= value]; the $ sigil rides in the name
    if (!node.m_kind.empty())
    {
        printToken(node.m_kind.c_str());
        printSpace();
    }
    if (node.m_name.empty() || node.m_name[0] != '$')
    {
        printToken("$");
    }
    printToken(node.m_name.c_str());
    if (node.m_initializer != NULL) { printToken(" = "); node.m_initializer->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void PhpFormatter::visitClassDecl(ClassDeclNode& node)
{
    // PHP: class|interface|trait|enum Name [extends Super] [implements Ifaces]
    printToken(node.m_kind.empty() ? "class" : node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL)
    {
        printToken(" extends ");
        node.m_superClass->accept(*this);
    }
    size_t ic = node.m_implements.size();
    if (ic > 0)
    {
        printToken(" implements ");
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void PhpFormatter::visitImportDecl(ImportDeclNode& node)
{
    // PHP: use Vendor\Package;
    printToken("use");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printToken(";");
    printNewline();
}

void PhpFormatter::visitNamespace(NamespaceNode& node)
{
    // PHP: namespace Name; (braceless form)
    printToken("namespace");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// Easter egg: PhpFormatter complete - echo "beautiful";
