#include "fmtpp/JavaFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>
#include <cstring>

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
    // ponytail: hand-built nodes carry no modifiers, and those default to
    // public (locked by tests); real modifiers (final, static, explicit
    // visibility) print verbatim instead of gaining an invented public
    if (node.m_modifiers.empty())
    {
        printToken("public");
        printSpace();
    }
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    if (!node.m_returnType.empty())
    {
        printToken(node.m_returnType.c_str());
        printSpace();
    }
    printToken(node.m_name.c_str());
    if (m_config.spaceBeforeFunctionParen) { printSpace(); }
    printToken("(");
    if (m_config.spaceInParens && node.m_params.size() > 0) { printSpace(); }
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    if (m_config.spaceInParens && node.m_params.size() > 0) { printSpace(); }
    printToken(")");
    if (node.m_body != NULL) { printFunctionBraceGap(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void JavaFormatter::visitClassDecl(ClassDeclNode& node)
{
    // ponytail: hand-built nodes carry no kind, and those default to
    // public class (locked by tests); real kinds ("final class",
    // "public class") print verbatim instead of gaining another public
    if (node.m_kind.empty() || node.m_kind == "class")
    {
        printToken("public");
        printSpace();
    }
    if (node.m_kind.empty() || node.m_kind == "class")
    {
        printToken("class");
    }
    else
    {
        printToken(node.m_kind.c_str());
    }
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
    if (node.m_body != NULL) { printTypeBraceGap(); node.m_body->accept(*this); }
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

void JavaFormatter::visitNamespace(NamespaceNode& node)
{
    printToken("package");
    printSpace();
    printToken(node.m_name.c_str());
    printToken(";");
    printNewline();
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void JavaFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    printToken("switch");
    if (spaceAfterKeywords()) { printSpace(); }
    if (node.m_discriminant != NULL) { printToken("("); node.m_discriminant->accept(*this); printToken(")"); }
    printOpeningBrace();
    printNewline();
    indent();
    // ponytail: cases print verbatim, breaks included. Auto-adding break
    // would duplicate real breaks and destroy intentional fallthrough.
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

void JavaFormatter::visitTryCatchStmt(TryCatchStmtNode& node)
{
    printToken("try");
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    if (node.m_catchClause != NULL || node.m_catchBody != NULL)
    {
        bool catchNewLine = (m_config.catchOnNewLine == "always")
            || (m_config.catchOnNewLine == "auto" && node.m_catchOnNewLine);
        if (catchNewLine) { printNewline(); printIndent(); }
        else { printSpace(); }
        printToken("catch");
        if (node.m_catchClause != NULL)
        {
            printSpace();
            printToken("(");
            node.m_catchClause->accept(*this);
            printToken(")");
        }
        if (node.m_catchBody != NULL) { printSpace(); node.m_catchBody->accept(*this); }
    }
    for (size_t eci = 0; eci < node.m_extraCatches.size(); eci++)
    {
        if (node.m_extraCatches[eci] == NULL) { continue; }
        printSpace();
        node.m_extraCatches[eci]->accept(*this);
    }
    if (node.m_finallyBlock != NULL)
    {
        bool finallyNewLine = (m_config.finallyOnNewLine == "always")
            || (m_config.finallyOnNewLine == "auto" && node.m_finallyOnNewLine);
        if (finallyNewLine) { printNewline(); printIndent(); }
        else { printSpace(); }
        printToken("finally");
        printSpace();
        node.m_finallyBlock->accept(*this);
    }
}

// Easter egg: JavaFormatter complete - everything is an object, except the primitives
