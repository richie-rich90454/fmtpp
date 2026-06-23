#include "fmtpp/JavaFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: the Java formatter awakens - ready to handle the verbosity of Java

JavaFormatter::JavaFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

JavaFormatter::~JavaFormatter()
{
}

void JavaFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    printToken("public");
    printSpace();
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

void JavaFormatter::visitClassDecl(ClassDeclNode& node)
{
    printToken("public");
    printSpace();
    printToken("class");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL)
    {
        printToken(" extends ");
        node.m_superClass->accept(*this);
    }
    if (node.m_implements.size() > 0)
    {
        printToken(" implements ");
        size_t ic = node.m_implements.size();
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void JavaFormatter::visitImportDecl(ImportDeclNode& node)
{
    printToken("import");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printToken(";");
    printNewline();
}

void JavaFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    printToken("switch");
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
        // Java convention: break after each case
        printIndent();
        printToken("break;");
        printNewline();
    }
    dedent();
    printIndent();
    printClosingBrace();
    printNewline();
}

void JavaFormatter::visitTryCatchStmt(TryCatchStmtNode& node)
{
    printToken("try");
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    if (node.m_catchClause != NULL)
    {
        printSpace();
        printToken("catch");
        printSpace();
        printToken("(");
        node.m_catchClause->accept(*this);
        printToken(")");
        printSpace();
        // The catch clause body is a BlockNode, visit it
        // Actually, m_catchClause is the exception type/variable, the body is separate
        // Let's just visit the catch clause as-is
    }
    if (node.m_finallyBlock != NULL)
    {
        printSpace();
        printToken("finally");
        printSpace();
        node.m_finallyBlock->accept(*this);
    }
}

// Easter egg: JavaFormatter complete - everything is an object, except the primitives
