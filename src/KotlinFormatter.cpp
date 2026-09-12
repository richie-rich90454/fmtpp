#include "fmtpp/KotlinFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: Kotlin formatter - where semicolons are optional and nulls are banished

KotlinFormatter::KotlinFormatter(OutputBuffer& out, const Config& config)
    : JavaFormatter(out, config)
{
}

KotlinFormatter::~KotlinFormatter()
{
}

bool KotlinFormatter::requiresSemicolon(ASTNode*) const
{
    return false;
}

void KotlinFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // Kotlin: [modifiers] fun name(params)[: ReturnType] { body }
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    printToken("fun");
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
    else { printNewline(); }
}

void KotlinFormatter::visitClassDecl(ClassDeclNode& node)
{
    // Kotlin: class|object Name [: Super(), Traits] { body }
    printToken(node.m_kind.empty() ? "class" : node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    bool first = true;
    if (node.m_superClass != NULL)
    {
        printToken(" : ");
        node.m_superClass->accept(*this);
        first = false;
    }
    size_t ic = node.m_implements.size();
    for (size_t i = 0; i < ic; i++)
    {
        if (first) { printToken(" : "); first = false; }
        else { printComma(); }
        if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void KotlinFormatter::visitImportDecl(ImportDeclNode& node)
{
    printToken("import");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printNewline();
}

void KotlinFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    // Kotlin when: arms print verbatim, separated by newlines, no commas
    printToken("when");
    if (spaceAfterKeywords()) { printSpace(); }
    if (node.m_discriminant != NULL) { printToken("("); node.m_discriminant->accept(*this); printToken(")"); }
    printOpeningBrace();
    printNewline();
    indent();
    size_t cc = node.m_cases.size();
    for (size_t i = 0; i < cc; i++)
    {
        printIndent();
        if (node.m_cases[i] != NULL) { node.m_cases[i]->accept(*this); }
        printNewline();
    }
    dedent();
    printIndent();
    printClosingBrace();
    printNewline();
}

void KotlinFormatter::visitNamespace(NamespaceNode& node)
{
    printToken("package");
    printSpace();
    printToken(node.m_name.c_str());
    printNewline();
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

// Easter egg: KotlinFormatter complete - null safety achieved (in the formatter, at least)
