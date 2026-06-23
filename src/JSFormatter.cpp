#include "fmtpp/JSFormatter.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include <cstdio>

// Easter egg: the JS formatter awakens - ready to handle the chaos of JavaScript

JSFormatter::JSFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

JSFormatter::~JSFormatter()
{
}

// --- Base class overrides (delegate to Node-suffixed methods) ---

void JSFormatter::visitFunctionDecl(FunctionDeclNode& node)    { visitFunctionDeclNode(node); }
void JSFormatter::visitVariableDecl(VariableDeclNode& node)    { visitVariableDeclNode(node); }
void JSFormatter::visitClassDecl(ClassDeclNode& node)          { visitClassDeclNode(node); }
void JSFormatter::visitImportDecl(ImportDeclNode& node)        { visitImportDeclNode(node); }
void JSFormatter::visitExportDecl(ExportDeclNode& node)        { visitExportDeclNode(node); }
void JSFormatter::visitArrowFunction(ArrowFunctionNode& node)  { visitArrowFunctionNode(node); }
void JSFormatter::visitTemplateLiteral(TemplateLiteralNode& node) { visitTemplateLiteralNode(node); }
void JSFormatter::visitInterfaceDecl(InterfaceDeclNode& node)  { visitInterfaceDeclNode(node); }
void JSFormatter::visitEnumDecl(EnumDeclNode& node)            { visitEnumDeclNode(node); }
void JSFormatter::visitTypeAlias(TypeAliasNode& node)          { visitTypeAliasNode(node); }

// --- JS-specific ASI handling ---

bool JSFormatter::requiresSemicolon(ASTNode* node) const
{
    (void)node;
    return m_config.semi;
}

// ================================================================
// JS-specific visitor implementations
// ================================================================

void JSFormatter::visitFunctionDeclNode(FunctionDeclNode& node)
{
    printToken("function");
    if (spaceAfterKeywords()) { printSpace(); }
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
    else { printSemicolon(); printNewline(); }
}

void JSFormatter::visitVariableDeclNode(VariableDeclNode& node)
{
    printToken(node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_initializer != NULL) { printToken(" = "); node.m_initializer->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void JSFormatter::visitClassDeclNode(ClassDeclNode& node)
{
    printToken("class");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL) { printToken(" extends "); node.m_superClass->accept(*this); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void JSFormatter::visitImportDeclNode(ImportDeclNode& node)
{
    printToken("import");
    if (node.m_defaultImport != NULL) { printSpace(); node.m_defaultImport->accept(*this); }
    if (node.m_specifiers.size() > 0)
    {
        if (node.m_defaultImport != NULL) { printComma(); }
        else { printSpace(); }
        printToken("{");
        if (m_config.bracketSpacing) { printSpace(); }
        size_t sc = node.m_specifiers.size();
        for (size_t i = 0; i < sc; i++)
        {
            if (node.m_specifiers[i] != NULL) { node.m_specifiers[i]->accept(*this); }
            if (i < sc - 1) { printComma(); }
        }
        if (m_config.bracketSpacing) { printSpace(); }
        printToken("}");
    }
    if (!node.m_source.empty())
    {
        if (node.m_specifiers.size() > 0 || node.m_defaultImport != NULL) { printToken(" from"); }
        printSpace();
        m_out.printf("'%s'", node.m_source.c_str());
        m_currentColumn += static_cast<int>(node.m_source.size()) + 2;
    }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void JSFormatter::visitExportDeclNode(ExportDeclNode& node)
{
    printToken("export");
    if (node.m_declaration != NULL) { printSpace(); node.m_declaration->accept(*this); }
    if (!node.m_source.empty())
    {
        printToken(" from ");
        m_out.printf("'%s'", node.m_source.c_str());
        m_currentColumn += static_cast<int>(node.m_source.size()) + 2;
        printSemicolonIfNeeded(&node);
        printNewline();
    }
}

void JSFormatter::visitArrowFunctionNode(ArrowFunctionNode& node)
{
    if (node.m_params.size() == 1 && m_config.arrowParens == "as-needed")
    {
        if (node.m_params[0] != NULL) { node.m_params[0]->accept(*this); }
    }
    else
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
    printToken(" => ");
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void JSFormatter::visitTemplateLiteralNode(TemplateLiteralNode& node)
{
    printToken("`");
    size_t qc = node.m_quasis.size();
    for (size_t i = 0; i < qc; i++)
    {
        if (node.m_quasis[i] != NULL) { node.m_quasis[i]->accept(*this); }
        if (i < node.m_expressions.size() && node.m_expressions[i] != NULL)
        { printToken("${"); node.m_expressions[i]->accept(*this); printToken("}"); }
    }
    printToken("`");
}

void JSFormatter::visitInterfaceDeclNode(InterfaceDeclNode& node)
{
    printToken("interface");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_extends.size() > 0)
    {
        printToken(" extends ");
        size_t ec = node.m_extends.size();
        for (size_t i = 0; i < ec; i++)
        {
            if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
            if (i < ec - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void JSFormatter::visitEnumDeclNode(EnumDeclNode& node)
{
    printToken("enum");
    printSpace();
    printToken(node.m_name.c_str());
    printOpeningBrace();
    printNewline();
    indent();
    size_t mc = node.m_members.size();
    for (size_t i = 0; i < mc; i++)
    {
        printIndent();
        if (node.m_members[i] != NULL) { node.m_members[i]->accept(*this); }
        if (i < mc - 1) { printToken(","); }
        printNewline();
    }
    dedent();
    printIndent();
    printClosingBrace();
}

void JSFormatter::visitTypeAliasNode(TypeAliasNode& node)
{
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    printToken(" = ");
    if (node.m_typeAnnotation != NULL) { node.m_typeAnnotation->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

// Easter egg: JSFormatter complete - JavaScript: the language that runs the internet, for better or worse