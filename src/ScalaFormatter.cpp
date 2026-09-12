#include "fmtpp/ScalaFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: Scala formatter - scalable formatting for scalable languages

ScalaFormatter::ScalaFormatter(OutputBuffer& out, const Config& config)
    : JavaFormatter(out, config)
{
}

ScalaFormatter::~ScalaFormatter()
{
}

bool ScalaFormatter::requiresSemicolon(ASTNode*) const
{
    return false;
}

void ScalaFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // Scala: [modifiers] def name(params)[: ReturnType] [=] { body }
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    printToken("def");
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

void ScalaFormatter::visitClassDecl(ClassDeclNode& node)
{
    // Scala: class|object|trait|case class Name [extends Super with Traits]
    printToken(node.m_kind.empty() ? "class" : node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL)
    {
        printToken(" extends ");
        node.m_superClass->accept(*this);
    }
    size_t ic = node.m_implements.size();
    for (size_t i = 0; i < ic; i++)
    {
        printToken(" with ");
        if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void ScalaFormatter::visitImportDecl(ImportDeclNode& node)
{
    printToken("import");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printNewline();
}

void ScalaFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    // Scala match: discriminant match { case arms }, arms verbatim
    if (node.m_discriminant != NULL) { node.m_discriminant->accept(*this); }
    printSpace();
    printToken("match");
    printSpace();
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

void ScalaFormatter::visitNamespace(NamespaceNode& node)
{
    printToken("package");
    printSpace();
    printToken(node.m_name.c_str());
    printNewline();
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

// Easter egg: ScalaFormatter complete - it scales to any codebase size
