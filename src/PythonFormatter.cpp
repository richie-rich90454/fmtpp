#include "fmtpp/PythonFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/WithNode.hpp"
#include "fmtpp/ast/SwitchCaseNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include <cstdio>

// Easter egg: Python formatter - readable, explicit, beautifully indented

PythonFormatter::PythonFormatter(OutputBuffer& out, const Config& config)
    : IndentFormatterBase(out, config)
{
}

PythonFormatter::~PythonFormatter()
{
}

void PythonFormatter::printEmptySuite()
{
    // ponytail: an empty Python suite is a syntax error without pass
    printIndent();
    printToken("pass");
}

void PythonFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
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
        printToken(" -> ");
        printToken(node.m_returnType.c_str());
    }
    printToken(":");
    printSuite(node.m_body);
}

void PythonFormatter::visitClassDecl(ClassDeclNode& node)
{
    printToken("class");
    printSpace();
    printToken(node.m_name.c_str());
    bool first = true;
    if (node.m_superClass != NULL)
    {
        printToken("(");
        node.m_superClass->accept(*this);
        first = false;
    }
    size_t ic = node.m_implements.size();
    for (size_t i = 0; i < ic; i++)
    {
        if (first) { printToken("("); first = false; }
        else { printComma(); }
        if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
    }
    if (!first) { printToken(")"); }
    printToken(":");
    printSuite(node.m_body);
}

void PythonFormatter::printIfChain(IfStmtNode& node)
{
    if (!node.m_keyword.empty())
    {
        printToken(node.m_keyword.c_str());
    }
    else
    {
        printToken("if");
    }
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    printToken(":");
    printSuite(node.m_thenBranch);
    if (node.m_elseBranch != NULL)
    {
        printIndent();
        if (node.m_elseBranch->isIfStmt())
        {
            // ponytail: elif chains carry their keyword, so recurse directly
            IfStmtNode* elif = static_cast<IfStmtNode*>(node.m_elseBranch);
            printIfChain(*elif);
        }
        else
        {
            printToken("else:");
            printSuite(node.m_elseBranch);
        }
    }
}

void PythonFormatter::visitIfStmt(IfStmtNode& node)
{
    printIfChain(node);
}

void PythonFormatter::visitForStmt(ForStmtNode& node)
{
    if (!node.m_keyword.empty())
    {
        printToken(node.m_keyword.c_str());
    }
    else
    {
        printToken("for");
    }
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    printToken(":");
    printSuite(node.m_body);
    if (node.m_elseBlock != NULL)
    {
        printIndent();
        printToken("else:");
        printSuite(node.m_elseBlock);
    }
}

void PythonFormatter::visitWhileStmt(WhileStmtNode& node)
{
    printToken("while");
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    printToken(":");
    printSuite(node.m_body);
    if (node.m_elseBlock != NULL)
    {
        printIndent();
        printToken("else:");
        printSuite(node.m_elseBlock);
    }
}

void PythonFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    printToken("match");
    if (node.m_discriminant != NULL)
    {
        printSpace();
        node.m_discriminant->accept(*this);
    }
    printToken(":");
    printNewline();
    indent();
    size_t cc = node.m_cases.size();
    for (size_t i = 0; i < cc; i++)
    {
        // ponytail: arms self-terminate through their suites, so unlike the
        // brace engine there is no separator newline here
        printIndent();
        if (node.m_cases[i] != NULL) { node.m_cases[i]->accept(*this); }
    }
    dedent();
}

void PythonFormatter::visitTryCatchStmt(TryCatchStmtNode& node)
{
    // ponytail: extras (further catches) carry no try body, so no header;
    // the statement's first line always rides the caller slot, later parts
    // start fresh lines with their own indent
    bool bare = (node.m_body == NULL);
    if (node.m_body != NULL)
    {
        printToken("try:");
        printSuite(node.m_body);
    }
    if (node.m_catchClause != NULL || node.m_catchBody != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("except");
        if (node.m_catchClause != NULL)
        {
            printSpace();
            node.m_catchClause->accept(*this);
        }
        printToken(":");
        if (node.m_catchBody != NULL) { printSuite(node.m_catchBody); }
        else
        {
            printNewline();
            indent();
            printIndent();
            printToken("pass");
            printNewline();
            dedent();
        }
    }
    for (size_t eci = 0; eci < node.m_extraCatches.size(); eci++)
    {
        if (node.m_extraCatches[eci] == NULL) { continue; }
        // ponytail: slot indent for the extra's bare first line
        printIndent();
        node.m_extraCatches[eci]->accept(*this);
    }
    if (node.m_elseBlock != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("else:");
        printSuite(node.m_elseBlock);
    }
    if (node.m_finallyBlock != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("finally:");
        printSuite(node.m_finallyBlock);
    }
}

void PythonFormatter::visitImportDecl(ImportDeclNode& node)
{
    if (!node.m_specifiers.empty())
    {
        // from x import y, z
        printToken("from");
        printSpace();
        printToken(node.m_source.c_str());
        printSpace();
        printToken("import");
        printSpace();
        size_t sc = node.m_specifiers.size();
        for (size_t i = 0; i < sc; i++)
        {
            if (node.m_specifiers[i] != NULL) { node.m_specifiers[i]->accept(*this); }
            if (i < sc - 1) { printComma(); }
        }
        printNewline();
        return;
    }
    printToken("import");
    if (!node.m_source.empty())
    {
        printSpace();
        printToken(node.m_source.c_str());
    }
    if (node.m_defaultImport != NULL)
    {
        printToken(" as ");
        node.m_defaultImport->accept(*this);
    }
    printNewline();
}

void PythonFormatter::visitWith(WithNode& node)
{
    if (!node.m_keyword.empty())
    {
        printToken(node.m_keyword.c_str());
    }
    else
    {
        printToken("with");
    }
    if (node.m_context != NULL)
    {
        printSpace();
        node.m_context->accept(*this);
    }
    printToken(":");
    printSuite(node.m_body);
}

// Easter egg: PythonFormatter complete - import this moment of beauty
