#include "fmtpp/CSharpFormatter.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: C# formatter - Microsoft's answer to Java, with extra sharpness and LINQ

CSharpFormatter::CSharpFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

CSharpFormatter::~CSharpFormatter()
{
}

void CSharpFormatter::visitClassDecl(ClassDeclNode& node)
{
    // C# class: public class Name : BaseClass, IInterface { }
    printToken("public");
    printSpace();
    printToken("class");
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
    if (node.m_body != NULL) { printNewline(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void CSharpFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // C# method: public ReturnType Name(params) { body }
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
    if (node.m_body != NULL) { printNewline(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void CSharpFormatter::visitNamespace(NamespaceNode& node)
{
    // C# namespace: namespace Name { body }
    printToken("namespace");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printNewline(); node.m_body->accept(*this); }
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
    if (node.m_body != NULL) { printNewline(); node.m_body->accept(*this); }
    else { printNewline(); }
}

// Easter egg: CSharpFormatter complete - C#: bringing sharpness to the .NET world