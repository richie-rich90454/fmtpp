#include "fmtpp/CppFormatter.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: C++ formatter - because sometimes you need more pluses, and more templates

CppFormatter::CppFormatter(OutputBuffer& out, const Config& config)
    : CFormatter(out, config)
{
}

CppFormatter::~CppFormatter()
{
}

void CppFormatter::visitClassDecl(ClassDeclNode& node)
{
    // C++ class with access specifiers
    printToken("class");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL)
    {
        printToken(" : public ");
        node.m_superClass->accept(*this);
    }
    if (node.m_implements.size() > 0)
    {
        if (node.m_superClass != NULL) { printComma(); }
        else { printToken(" : public "); }
        size_t ic = node.m_implements.size();
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void CppFormatter::visitNamespace(NamespaceNode& node)
{
    printToken("namespace");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void CppFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // C++ function: returnType name(params) { body }
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
    if (node.m_body != NULL) { printNewline(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// Easter egg: CppFormatter complete - C++: the language that makes you earn every semicolon