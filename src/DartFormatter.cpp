#include "fmtpp/DartFormatter.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: Dart formatter - every format is a triple twenty

DartFormatter::DartFormatter(OutputBuffer& out, const Config& config)
    : CFormatter(out, config)
{
}

DartFormatter::~DartFormatter()
{
}

void DartFormatter::visitClassDecl(ClassDeclNode& node)
{
    // Dart: class Name [extends Super] [implements Ifaces]
    // (abstract rides along inside m_kind, e.g. "abstract class")
    printToken("class");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL)
    {
        printToken(" extends ");
        node.m_superClass->accept(*this);
    }
    size_t ic = node.m_implements.size();
    if (ic > 0)
    {
        printToken(" implements ");
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void DartFormatter::visitImportDecl(ImportDeclNode& node)
{
    // Dart: import 'uri' [as prefix] [show/hide ...];
    printToken("import");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printToken(";");
    printNewline();
}

// Easter egg: DartFormatter complete - game, set, match on formatting
