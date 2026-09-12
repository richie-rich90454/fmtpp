#include "fmtpp/RubyFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include <cstdio>

// Easter egg: Ruby formatter - making programmers happy, one end at a time

RubyFormatter::RubyFormatter(OutputBuffer& out, const Config& config)
    : IndentFormatterBase(out, config)
{
}

RubyFormatter::~RubyFormatter()
{
}

void RubyFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    printToken("def");
    printSpace();
    printToken(node.m_name.c_str());
    if (!node.m_params.empty())
    {
        printToken("(");
        size_t pc = node.m_params.size();
        for (size_t i = 0; i < pc; i++)
        {
            if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
            if (i < pc - 1) { printComma(); }
        }
        printToken(")");
    }
    printSuite(node.m_body);
    printIndent();
    printToken("end");
    printNewline();
}

void RubyFormatter::visitClassDecl(ClassDeclNode& node)
{
    printToken("class");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL)
    {
        printToken(" < ");
        node.m_superClass->accept(*this);
    }
    printSuite(node.m_body);
    printIndent();
    printToken("end");
    printNewline();
}

void RubyFormatter::visitNamespace(NamespaceNode& node)
{
    printToken("module");
    printSpace();
    printToken(node.m_name.c_str());
    printSuite(node.m_body);
    printIndent();
    printToken("end");
    printNewline();
}

void RubyFormatter::printIfChain(IfStmtNode& node)
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
    printSuite(node.m_thenBranch);
    if (node.m_elseBranch != NULL)
    {
        printIndent();
        if (node.m_elseBranch->isIfStmt())
        {
            // ponytail: chained elsif shares the single closing end below
            IfStmtNode* elsif = static_cast<IfStmtNode*>(node.m_elseBranch);
            printIfChain(*elsif);
        }
        else
        {
            printToken("else");
            printSuite(node.m_elseBranch);
        }
    }
}

void RubyFormatter::visitIfStmt(IfStmtNode& node)
{
    printIfChain(node);
    printIndent();
    printToken("end");
    printNewline();
}

void RubyFormatter::visitForStmt(ForStmtNode& node)
{
    // ponytail: Ruby for is rare but real; it closes like everything else
    printToken("for");
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    printSuite(node.m_body);
    printIndent();
    printToken("end");
    printNewline();
}

void RubyFormatter::visitWhileStmt(WhileStmtNode& node)
{
    if (!node.m_keyword.empty())
    {
        printToken(node.m_keyword.c_str());
    }
    else
    {
        printToken("while");
    }
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    printSuite(node.m_body);
    printIndent();
    printToken("end");
    printNewline();
}

void RubyFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    printToken("case");
    if (node.m_discriminant != NULL)
    {
        printSpace();
        node.m_discriminant->accept(*this);
    }
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
    printIndent();
    printToken("end");
    printNewline();
}

void RubyFormatter::visitTryCatchStmt(TryCatchStmtNode& node)
{
    // ponytail: extras (further rescues) carry no begin body, so no header
    // and no end; the statement's first line always rides the caller slot,
    // later parts start fresh lines with their own indent
    bool bare = (node.m_body == NULL);
    if (node.m_body != NULL)
    {
        printToken("begin");
        printSuite(node.m_body);
    }
    if (node.m_catchClause != NULL || node.m_catchBody != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("rescue");
        if (node.m_catchClause != NULL)
        {
            printSpace();
            node.m_catchClause->accept(*this);
        }
        if (node.m_catchBody != NULL) { printSuite(node.m_catchBody); }
        else { printNewline(); }
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
        printToken("else");
        printSuite(node.m_elseBlock);
    }
    if (node.m_finallyBlock != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("ensure");
        printSuite(node.m_finallyBlock);
    }
    // ponytail: body-less chains (method rescue/ensure) close through the
    // def's own end, so only headed begins print one here
    if (node.m_body != NULL)
    {
        printIndent();
        printToken("end");
        printNewline();
    }
}

void RubyFormatter::visitImportDecl(ImportDeclNode& node)
{
    printToken("require");
    if (!node.m_source.empty())
    {
        printSpace();
        printToken("\"");
        printToken(node.m_source.c_str());
        printToken("\"");
    }
    printNewline();
}

void RubyFormatter::visitBlock(BlockNode& node)
{
    // ponytail: standalone blocks are do-blocks here (hashes never arrive
    // as blocks); do-blocks close their own end
    if (node.m_kind == "do" && !node.m_statements.empty())
    {
        // ponytail: statements[0] is the verbatim opener and owns the slot
        // line (already indented by the caller); only the body indents
        node.m_statements[0]->accept(*this);
        indent();
        size_t sc = node.m_statements.size();
        for (size_t i = 1; i < sc; i++)
        {
            printIndent();
            if (node.m_statements[i] != NULL) { node.m_statements[i]->accept(*this); }
            if (i < sc - 1) { printNewline(); }
        }
        dedent();
        // ponytail: no blank line in an empty block (foo do / end adjacent)
        if (sc > 1) { printNewline(); }
        printIndent();
        printToken("end");
        printNewline();
        return;
    }
    printSuite(&node);
}

void RubyFormatter::visitLiteral(LiteralNode& node)
{
    switch (node.m_type)
    {
    case LiteralNode::TYPE_STRING:
        printStringLiteral(node);
        break;
    case LiteralNode::TYPE_BOOLEAN:
    case LiteralNode::TYPE_NUMBER:
        printToken(node.m_value.c_str());
        break;
    case LiteralNode::TYPE_NULL:
        printToken("nil");
        break;
    case LiteralNode::TYPE_REGEX:
    default:
        printToken(node.m_value.c_str());
        break;
    }
}

// Easter egg: RubyFormatter complete - end of formatting, beginning of joy
