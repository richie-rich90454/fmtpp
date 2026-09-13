#include "fmtpp/CSharpFormatter.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>
#include <cstring>

// Easter egg: C# formatter - Microsoft's answer to Java, with extra sharpness and LINQ

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
        || startsWithWord(text, "protected") || startsWithWord(text, "internal");
}

CSharpFormatter::CSharpFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

CSharpFormatter::~CSharpFormatter()
{
}

bool CSharpFormatter::defaultFunctionBraceSameLine() const
{
    // ponytail: C# house style is Allman; braceStyleFunctions overrides it
    return false;
}

bool CSharpFormatter::defaultTypeBraceSameLine() const
{
    return false;
}

void CSharpFormatter::visitClassDecl(ClassDeclNode& node)
{
    // C# class/struct: public class Name : BaseClass, IInterface { }
    // (an explicit visibility in the kind wins instead of doubling up)
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
    if (node.m_superClass != NULL || node.m_implements.size() > 0)
    {
        printToken(" : ");
        bool first = true;
        if (node.m_superClass != NULL)
        {
            node.m_superClass->accept(*this);
            first = false;
        }
        size_t ic = node.m_implements.size();
        for (size_t i = 0; i < ic; i++)
        {
            if (!first) { printComma(); }
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            first = false;
        }
    }
    if (node.m_body != NULL) { printTypeBraceGap(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void CSharpFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // C# method: public [modifiers] ReturnType Name(params) { body }
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

void CSharpFormatter::visitNamespace(NamespaceNode& node)
{
    // C# namespace: namespace Name { body }
    printToken("namespace");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printNewline(); printIndent(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void CSharpFormatter::visitInterfaceDecl(InterfaceDeclNode& node)
{
    // C# interface: public interface IName : IBase { }
    printToken("public");
    printSpace();
    printToken("interface");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_extends.size() > 0)
    {
        printToken(" : ");
        size_t ec = node.m_extends.size();
        for (size_t i = 0; i < ec; i++)
        {
            if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
            if (i < ec - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printTypeBraceGap(); node.m_body->accept(*this); }
    else { printNewline(); }
}

// Easter egg: CSharpFormatter complete - C#: bringing sharpness to the .NET world