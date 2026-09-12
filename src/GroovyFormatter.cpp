#include "fmtpp/GroovyFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: Groovy formatter - defying verbosity, one def at a time

GroovyFormatter::GroovyFormatter(OutputBuffer& out, const Config& config)
    : JavaFormatter(out, config)
{
}

GroovyFormatter::~GroovyFormatter()
{
}

bool GroovyFormatter::requiresSemicolon(ASTNode*) const
{
    return false;
}

void GroovyFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // Groovy: [modifiers] (def | ReturnType) name(params) { body }.
    // Capitalized names are constructors and take no def keyword.
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    if (node.m_returnType.empty() && (node.m_name.empty()
        || node.m_name[0] < 'A' || node.m_name[0] > 'Z'))
    {
        printToken("def");
    }
    else if (!node.m_returnType.empty())
    {
        printToken(node.m_returnType.c_str());
    }
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
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void GroovyFormatter::visitClassDecl(ClassDeclNode& node)
{
    // Groovy: class Name [extends Super] [implements Traits]
    // (abstract/final ride along inside m_kind, e.g. "abstract class")
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

void GroovyFormatter::visitImportDecl(ImportDeclNode& node)
{
    printToken("import");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
    }
    printNewline();
}

void GroovyFormatter::visitNamespace(NamespaceNode& node)
{
    printToken("package");
    printSpace();
    printToken(node.m_name.c_str());
    printNewline();
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

// Easter egg: GroovyFormatter complete - marvelously groovy output
