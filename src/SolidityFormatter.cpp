#include "fmtpp/SolidityFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include <cstdio>

// Easter egg: Solidity formatter - gas-optimized formatting, no reentrancy on style

SolidityFormatter::SolidityFormatter(OutputBuffer& out, const Config& config)
    : JSFormatter(out, config)
{
}

SolidityFormatter::~SolidityFormatter()
{
}

void SolidityFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // Solidity: function name(params) [modifiers] [returns] { body }
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
    if (!node.m_modifiers.empty())
    {
        printSpace();
        printToken(node.m_modifiers.c_str());
    }
    if (!node.m_returnType.empty())
    {
        printSpace();
        printToken(node.m_returnType.c_str());
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void SolidityFormatter::visitClassDecl(ClassDeclNode& node)
{
    // Solidity: contract|interface|library Name [is Bases] { body }
    printToken(node.m_kind.empty() ? "contract" : node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    bool first = true;
    if (node.m_superClass != NULL)
    {
        printToken(" is ");
        node.m_superClass->accept(*this);
        first = false;
    }
    size_t ic = node.m_implements.size();
    for (size_t i = 0; i < ic; i++)
    {
        if (first) { printToken(" is "); first = false; }
        else { printComma(); }
        if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void SolidityFormatter::visitImportDecl(ImportDeclNode& node)
{
    // Solidity: import "path";
    printToken("import");
    printSpace();
    printToken("\"");
    printToken(node.m_source.c_str());
    printToken("\";");
    printNewline();
}

// Easter egg: SolidityFormatter complete - deployed to mainnet of readability
