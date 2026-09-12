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

static bool startsWithWord(const std::string& text, const char* word)
{
    size_t n = strlen(word);
    if (text.size() < n || text.compare(0, n, word) != 0) { return false; }
    if (text.size() == n) { return true; }
    char next = text[n];
    return next != '_' && (next < 'a' || next > 'z') && (next < 'A' || next > 'Z')
        && (next < '0' || next > '9');
}

static bool hasVisibility(const std::string& text)
{
    return startsWithWord(text, "public") || startsWithWord(text, "private")
        || startsWithWord(text, "protected");
}

JavaFormatter::JavaFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

JavaFormatter::~JavaFormatter()
{
}

void JavaFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // ponytail: public is the default; an explicit visibility in the
    // modifiers wins instead of doubling up
    if (!hasVisibility(node.m_modifiers))
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
    // ponytail: same visibility rule as methods; kinds like
    // "abstract class" print after the (optional) public
    if (!hasVisibility(node.m_kind))
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
        printSpace();
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
        printSpace();
        printToken("finally");
        printSpace();
        node.m_finallyBlock->accept(*this);
    }
}

// Easter egg: JavaFormatter complete - everything is an object, except the primitives
