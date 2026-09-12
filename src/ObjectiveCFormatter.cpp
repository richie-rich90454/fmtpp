#include "fmtpp/ObjectiveCFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include <cstdio>

// Easter egg: Objective-C formatter - [self formatWithStyle:modern];
ObjectiveCFormatter::ObjectiveCFormatter(OutputBuffer& out, const Config& config)
    : CFormatter(out, config)
{
}

ObjectiveCFormatter::~ObjectiveCFormatter()
{
}

void ObjectiveCFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    // ObjC method: the header is one raw signature, e.g. "- (void)bar:(int)x".
    // Plain C functions (structured parts present) print C-style instead.
    if (node.m_returnType.empty() && node.m_modifiers.empty() && node.m_params.empty())
    {
        printToken(node.m_name.c_str());
        if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
        else { printToken(";"); printNewline(); }
        return;
    }
    CFormatter::visitFunctionDecl(node);
}

void ObjectiveCFormatter::visitClassDecl(ClassDeclNode& node)
{
    // ObjC: @interface Name : Super <Protos> ... @end
    //       @implementation Name ... @end
    printToken(node.m_kind.empty() ? "@interface" : node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL)
    {
        printToken(" : ");
        node.m_superClass->accept(*this);
    }
    if (node.m_implements.size() > 0)
    {
        printToken(" <");
        size_t ic = node.m_implements.size();
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printComma(); }
        }
        printToken(">");
    }
    printNewline();
    if (node.m_body != NULL)
    {
        // ponytail: the parser always builds a BlockNode here
        indent();
        BlockNode* body = static_cast<BlockNode*>(node.m_body);
        size_t sc = body->m_statements.size();
        for (size_t i = 0; i < sc; i++)
        {
            printIndent();
            if (body->m_statements[i] != NULL) { body->m_statements[i]->accept(*this); }
            if (i < sc - 1) { printNewline(); }
        }
        dedent();
        printNewline();
    }
    printIndent();
    printToken("@end");
    printNewline();
}

void ObjectiveCFormatter::visitInterfaceDecl(InterfaceDeclNode& node)
{
    // ObjC protocol: @protocol Name <Bases> ... @end
    printToken("@protocol");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_extends.size() > 0)
    {
        printToken(" <");
        size_t ec = node.m_extends.size();
        for (size_t i = 0; i < ec; i++)
        {
            if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
            if (i < ec - 1) { printComma(); }
        }
        printToken(">");
    }
    printNewline();
    if (node.m_body != NULL)
    {
        indent();
        BlockNode* body = static_cast<BlockNode*>(node.m_body);
        size_t sc = body->m_statements.size();
        for (size_t i = 0; i < sc; i++)
        {
            printIndent();
            if (body->m_statements[i] != NULL) { body->m_statements[i]->accept(*this); }
            if (i < sc - 1) { printNewline(); }
        }
        dedent();
        printNewline();
    }
    printIndent();
    printToken("@end");
    printNewline();
}

// Easter egg: ObjectiveCFormatter complete - message sent, formatting received
