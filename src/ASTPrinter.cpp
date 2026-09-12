#include "fmtpp/ASTPrinter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/HtmlElementNode.hpp"
#include "fmtpp/ast/WithNode.hpp"
#include "fmtpp/ast/SwitchCaseNode.hpp"
#include <cstdio>
#include <vector>

// Easter egg: the printer that brings the tree to life, one printf at a time

ASTPrinter::ASTPrinter(const std::string& indent)
    : m_indent(indent)
    , m_indentLevel(0)
{
}

ASTPrinter::~ASTPrinter()
{
}

void ASTPrinter::printIndent()
{
    int i;
    for (i = 0; i < m_indentLevel; i++)
    {
        printf("%s", m_indent.c_str());
    }
}

void ASTPrinter::visitProgram(ProgramNode& node)
{
    printIndent();
    printf("ProgramNode\n");
    m_indentLevel++;
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_statements.size(); i++)
        {
            if (node.m_statements[i] != NULL)
            {
                node.m_statements[i]->accept(*this);
            }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitFunctionDecl(FunctionDeclNode& node)
{
    printIndent();
    printf("FunctionDeclNode\n");
    m_indentLevel++;
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_params.size(); i++)
        {
            if (node.m_params[i] != NULL)
            {
                node.m_params[i]->accept(*this);
            }
        }
    }
    if (node.m_body != NULL)
    {
        node.m_body->accept(*this);
    }
    m_indentLevel--;
}

void ASTPrinter::visitVariableDecl(VariableDeclNode& node)
{
    printIndent();
    printf("VariableDeclNode\n");
    m_indentLevel++;
    printIndent();
    printf("kind: %s\n", node.m_kind.c_str());
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    if (node.m_initializer != NULL)
    {
        node.m_initializer->accept(*this);
    }
    m_indentLevel--;
}

void ASTPrinter::visitIfStmt(IfStmtNode& node)
{
    printIndent();
    printf("IfStmtNode\n");
    m_indentLevel++;
    if (!node.m_keyword.empty())
    {
        printIndent();
        printf("keyword: %s\n", node.m_keyword.c_str());
    }
    if (node.m_condition != NULL)
    {
        node.m_condition->accept(*this);
    }
    if (node.m_thenBranch != NULL)
    {
        node.m_thenBranch->accept(*this);
    }
    if (node.m_elseBranch != NULL)
    {
        node.m_elseBranch->accept(*this);
    }
    m_indentLevel--;
}

void ASTPrinter::visitForStmt(ForStmtNode& node)
{
    printIndent();
    printf("ForStmtNode\n");
    m_indentLevel++;
    if (node.m_init != NULL) { node.m_init->accept(*this); }
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_update != NULL) { node.m_update->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    if (node.m_elseBlock != NULL) { node.m_elseBlock->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitWhileStmt(WhileStmtNode& node)
{
    printIndent();
    printf("WhileStmtNode\n");
    m_indentLevel++;
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    if (node.m_elseBlock != NULL) { node.m_elseBlock->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitSwitchStmt(SwitchStmtNode& node)
{
    printIndent();
    printf("SwitchStmtNode\n");
    m_indentLevel++;
    if (node.m_discriminant != NULL) { node.m_discriminant->accept(*this); }
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_cases.size(); i++)
        {
            if (node.m_cases[i] != NULL) { node.m_cases[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitBlock(BlockNode& node)
{
    printIndent();
    printf("BlockNode\n");
    m_indentLevel++;
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_statements.size(); i++)
        {
            if (node.m_statements[i] != NULL) { node.m_statements[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitReturnStmt(ReturnStmtNode& node)
{
    printIndent();
    printf("ReturnStmtNode\n");
    if (node.m_argument != NULL)
    {
        m_indentLevel++;
        node.m_argument->accept(*this);
        m_indentLevel--;
    }
}

void ASTPrinter::visitThrowStmt(ThrowStmtNode& node)
{
    printIndent();
    printf("ThrowStmtNode\n");
    if (node.m_expression != NULL)
    {
        m_indentLevel++;
        node.m_expression->accept(*this);
        m_indentLevel--;
    }
}

void ASTPrinter::visitTryCatchStmt(TryCatchStmtNode& node)
{
    printIndent();
    printf("TryCatchStmtNode\n");
    m_indentLevel++;
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    if (node.m_catchClause != NULL) { node.m_catchClause->accept(*this); }
    if (node.m_catchBody != NULL) { node.m_catchBody->accept(*this); }
    if (node.m_elseBlock != NULL) { node.m_elseBlock->accept(*this); }
    {
        std::vector<ASTNode*>::size_type eci;
        for (eci = 0; eci < node.m_extraCatches.size(); eci++)
        {
            if (node.m_extraCatches[eci] != NULL) { node.m_extraCatches[eci]->accept(*this); }
        }
    }
    if (node.m_finallyBlock != NULL) { node.m_finallyBlock->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitComment(CommentNode& node)
{
    printIndent();
    printf("CommentNode\n");
    m_indentLevel++;
    printIndent();
    printf("text: %s\n", node.m_text.c_str());
    printIndent();
    printf("isBlock: %s\n", node.m_isBlock ? "true" : "false");
    m_indentLevel--;
}

void ASTPrinter::visitLiteral(LiteralNode& node)
{
    printIndent();
    printf("LiteralNode\n");
    m_indentLevel++;
    printIndent();
    printf("value: %s\n", node.m_value.c_str());
    printIndent();
    printf("type: %d\n", (int)node.m_type);
    m_indentLevel--;
}

void ASTPrinter::visitIdentifier(IdentifierNode& node)
{
    printIndent();
    printf("IdentifierNode\n");
    m_indentLevel++;
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    m_indentLevel--;
}

void ASTPrinter::visitArrayLiteral(ArrayLiteralNode& node)
{
    printIndent();
    printf("ArrayLiteralNode\n");
    m_indentLevel++;
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_elements.size(); i++)
        {
            if (node.m_elements[i] != NULL) { node.m_elements[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitObjectLiteral(ObjectLiteralNode& node)
{
    printIndent();
    printf("ObjectLiteralNode\n");
    m_indentLevel++;
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_properties.size(); i++)
        {
            if (node.m_properties[i] != NULL) { node.m_properties[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitTemplateLiteral(TemplateLiteralNode& node)
{
    printIndent();
    printf("TemplateLiteralNode\n");
    m_indentLevel++;
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_quasis.size(); i++)
        {
            if (node.m_quasis[i] != NULL) { node.m_quasis[i]->accept(*this); }
        }
        for (i = 0; i < node.m_expressions.size(); i++)
        {
            if (node.m_expressions[i] != NULL) { node.m_expressions[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitArrowFunction(ArrowFunctionNode& node)
{
    printIndent();
    printf("ArrowFunctionNode\n");
    m_indentLevel++;
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_params.size(); i++)
        {
            if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        }
    }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitClassDecl(ClassDeclNode& node)
{
    printIndent();
    printf("ClassDeclNode\n");
    m_indentLevel++;
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    if (node.m_superClass != NULL) { node.m_superClass->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitInterfaceDecl(InterfaceDeclNode& node)
{
    printIndent();
    printf("InterfaceDeclNode\n");
    m_indentLevel++;
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitEnumDecl(EnumDeclNode& node)
{
    printIndent();
    printf("EnumDeclNode\n");
    m_indentLevel++;
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_members.size(); i++)
        {
            if (node.m_members[i] != NULL) { node.m_members[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitTypeAlias(TypeAliasNode& node)
{
    printIndent();
    printf("TypeAliasNode\n");
    m_indentLevel++;
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    if (node.m_typeAnnotation != NULL) { node.m_typeAnnotation->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitImportDecl(ImportDeclNode& node)
{
    printIndent();
    printf("ImportDeclNode\n");
    m_indentLevel++;
    printIndent();
    printf("source: %s\n", node.m_source.c_str());
    if (node.m_defaultImport != NULL) { node.m_defaultImport->accept(*this); }
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_specifiers.size(); i++)
        {
            if (node.m_specifiers[i] != NULL) { node.m_specifiers[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitExportDecl(ExportDeclNode& node)
{
    printIndent();
    printf("ExportDeclNode\n");
    m_indentLevel++;
    if (node.m_declaration != NULL) { node.m_declaration->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitPreprocessorDirective(PreprocessorDirectiveNode& node)
{
    printIndent();
    printf("PreprocessorDirectiveNode\n");
    m_indentLevel++;
    printIndent();
    printf("directive: %s\n", node.m_directive.c_str());
    m_indentLevel--;
}

void ASTPrinter::visitNamespace(NamespaceNode& node)
{
    printIndent();
    printf("NamespaceNode\n");
    m_indentLevel++;
    printIndent();
    printf("name: %s\n", node.m_name.c_str());
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitProperty(PropertyNode& node)
{
    printIndent();
    printf("PropertyNode\n");
    m_indentLevel++;
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    if (node.m_value != NULL) { node.m_value->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitRawText(RawTextNode& node)
{
    printIndent();
    printf("RawTextNode\n");
    m_indentLevel++;
    printIndent();
    printf("text: %s\n", node.m_text.c_str());
    m_indentLevel--;
}

void ASTPrinter::visitHtmlElement(HtmlElementNode& node)
{
    printIndent();
    printf("HtmlElementNode\n");
    m_indentLevel++;
    printIndent();
    printf("tag: %s\n", node.m_tag.c_str());
    {
        std::vector<std::string>::size_type i;
        for (i = 0; i < node.m_attrs.size(); i++)
        {
            printIndent();
            printf("attr: %s\n", node.m_attrs[i].c_str());
        }
    }
    {
        std::vector<ASTNode*>::size_type i;
        for (i = 0; i < node.m_children.size(); i++)
        {
            if (node.m_children[i] != NULL) { node.m_children[i]->accept(*this); }
        }
    }
    m_indentLevel--;
}

void ASTPrinter::visitWith(WithNode& node)
{
    printIndent();
    printf("WithNode\n");
    m_indentLevel++;
    if (node.m_context != NULL) { node.m_context->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    m_indentLevel--;
}

void ASTPrinter::visitSwitchCase(SwitchCaseNode& node)
{
    printIndent();
    printf("SwitchCaseNode\n");
    m_indentLevel++;
    if (node.m_pattern != NULL) { node.m_pattern->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    m_indentLevel--;
}
