#include "fmtpp/DFormatter.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include <cstdio>

// Easter egg: D formatter - unittest { assert(formatting == beautiful); }

DFormatter::DFormatter(OutputBuffer& out, const Config& config)
    : CFormatter(out, config)
{
}

DFormatter::~DFormatter()
{
}

void DFormatter::visitClassDecl(ClassDeclNode& node)
{
    // D: class|struct|union Name [: Bases] { body }
    // (abstract/final ride along inside m_kind)
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
    else { printToken(";"); printNewline(); }
}

void DFormatter::visitInterfaceDecl(InterfaceDeclNode& node)
{
    // D: interface Name [: Bases] { body }
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
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

void DFormatter::visitImportDecl(ImportDeclNode& node)
{
    // D: import std.stdio [: names];
    printToken("import");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printToken(";");
    printNewline();
}

void DFormatter::visitNamespace(NamespaceNode& node)
{
    // D: module name;
    printToken("module");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// Easter egg: DFormatter complete - scope(exit) ugly_code;
