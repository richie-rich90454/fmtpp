#include "fmtpp/SwiftFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: Swift formatter - optional semicolons, non-optional safety

SwiftFormatter::SwiftFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

SwiftFormatter::~SwiftFormatter()
{
}

bool SwiftFormatter::requiresSemicolon(ASTNode*) const
{
    return false;
}

void SwiftFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // Swift: [modifiers] func name(params)[ -> ReturnType] { body }.
    // Initializers, deinitializers and subscripts take no func keyword.
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    if (node.m_name != "init" && node.m_name != "deinit"
        && node.m_name.compare(0, 9, "subscript") != 0)
    {
        printToken("func");
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
    if (!node.m_returnType.empty())
    {
        printToken(" -> ");
        printToken(node.m_returnType.c_str());
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void SwiftFormatter::visitClassDecl(ClassDeclNode& node)
{
    // Swift: class|struct|actor Name[: Bases] { body }
    printToken(node.m_kind.empty() ? "class" : node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    bool first = true;
    if (node.m_superClass != NULL)
    {
        printToken(": ");
        node.m_superClass->accept(*this);
        first = false;
    }
    size_t ic = node.m_implements.size();
    for (size_t i = 0; i < ic; i++)
    {
        if (first) { printToken(": "); first = false; }
        else { printComma(); }
        if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void SwiftFormatter::visitImportDecl(ImportDeclNode& node)
{
    printToken("import");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printNewline();
}

void SwiftFormatter::visitForStmt(ForStmtNode& node)
{
    // ponytail: a C-style triple here means invalid Swift input;
    // print it part-preserving rather than scrambling it
    if (node.m_classic)
    {
        CstyleFormatterBase::visitForStmt(node);
        return;
    }
    // Swift for-in takes no parentheses; the loop variable rejoins here
    printToken("for");
    printSpace();
    if (node.m_init != NULL)
    {
        node.m_init->accept(*this);
        printToken(" in ");
    }
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void SwiftFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    // Swift switch takes no parentheses around the subject
    printToken("switch");
    if (node.m_discriminant != NULL)
    {
        printSpace();
        node.m_discriminant->accept(*this);
    }
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

// Easter egg: SwiftFormatter complete - value semantics for formatting too
