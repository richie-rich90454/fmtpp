#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/MemoryPool.hpp"
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
#include <new>

// Easter egg: the factory floor - where raw memory becomes meaningful AST nodes

ASTNodeFactory::ASTNodeFactory() {}
ASTNodeFactory::~ASTNodeFactory() {}

ProgramNode* ASTNodeFactory::createProgramNode()
{
    return new (g_pool->allocate(sizeof(ProgramNode))) ProgramNode();
}

FunctionDeclNode* ASTNodeFactory::createFunctionDeclNode()
{
    return new (g_pool->allocate(sizeof(FunctionDeclNode))) FunctionDeclNode();
}

VariableDeclNode* ASTNodeFactory::createVariableDeclNode()
{
    return new (g_pool->allocate(sizeof(VariableDeclNode))) VariableDeclNode();
}

IfStmtNode* ASTNodeFactory::createIfStmtNode()
{
    return new (g_pool->allocate(sizeof(IfStmtNode))) IfStmtNode();
}

ForStmtNode* ASTNodeFactory::createForStmtNode()
{
    return new (g_pool->allocate(sizeof(ForStmtNode))) ForStmtNode();
}

WhileStmtNode* ASTNodeFactory::createWhileStmtNode()
{
    return new (g_pool->allocate(sizeof(WhileStmtNode))) WhileStmtNode();
}

SwitchStmtNode* ASTNodeFactory::createSwitchStmtNode()
{
    return new (g_pool->allocate(sizeof(SwitchStmtNode))) SwitchStmtNode();
}

BlockNode* ASTNodeFactory::createBlockNode()
{
    return new (g_pool->allocate(sizeof(BlockNode))) BlockNode();
}

ReturnStmtNode* ASTNodeFactory::createReturnStmtNode()
{
    return new (g_pool->allocate(sizeof(ReturnStmtNode))) ReturnStmtNode();
}

ThrowStmtNode* ASTNodeFactory::createThrowStmtNode()
{
    return new (g_pool->allocate(sizeof(ThrowStmtNode))) ThrowStmtNode();
}

TryCatchStmtNode* ASTNodeFactory::createTryCatchStmtNode()
{
    return new (g_pool->allocate(sizeof(TryCatchStmtNode))) TryCatchStmtNode();
}

CommentNode* ASTNodeFactory::createCommentNode()
{
    return new (g_pool->allocate(sizeof(CommentNode))) CommentNode();
}

LiteralNode* ASTNodeFactory::createLiteralNode()
{
    return new (g_pool->allocate(sizeof(LiteralNode))) LiteralNode();
}

IdentifierNode* ASTNodeFactory::createIdentifierNode()
{
    return new (g_pool->allocate(sizeof(IdentifierNode))) IdentifierNode();
}

ArrayLiteralNode* ASTNodeFactory::createArrayLiteralNode()
{
    return new (g_pool->allocate(sizeof(ArrayLiteralNode))) ArrayLiteralNode();
}

ObjectLiteralNode* ASTNodeFactory::createObjectLiteralNode()
{
    return new (g_pool->allocate(sizeof(ObjectLiteralNode))) ObjectLiteralNode();
}

TemplateLiteralNode* ASTNodeFactory::createTemplateLiteralNode()
{
    return new (g_pool->allocate(sizeof(TemplateLiteralNode))) TemplateLiteralNode();
}

ArrowFunctionNode* ASTNodeFactory::createArrowFunctionNode()
{
    return new (g_pool->allocate(sizeof(ArrowFunctionNode))) ArrowFunctionNode();
}

ClassDeclNode* ASTNodeFactory::createClassDeclNode()
{
    return new (g_pool->allocate(sizeof(ClassDeclNode))) ClassDeclNode();
}

InterfaceDeclNode* ASTNodeFactory::createInterfaceDeclNode()
{
    return new (g_pool->allocate(sizeof(InterfaceDeclNode))) InterfaceDeclNode();
}

EnumDeclNode* ASTNodeFactory::createEnumDeclNode()
{
    return new (g_pool->allocate(sizeof(EnumDeclNode))) EnumDeclNode();
}

TypeAliasNode* ASTNodeFactory::createTypeAliasNode()
{
    return new (g_pool->allocate(sizeof(TypeAliasNode))) TypeAliasNode();
}

ImportDeclNode* ASTNodeFactory::createImportDeclNode()
{
    return new (g_pool->allocate(sizeof(ImportDeclNode))) ImportDeclNode();
}

ExportDeclNode* ASTNodeFactory::createExportDeclNode()
{
    return new (g_pool->allocate(sizeof(ExportDeclNode))) ExportDeclNode();
}

PreprocessorDirectiveNode* ASTNodeFactory::createPreprocessorDirectiveNode()
{
    return new (g_pool->allocate(sizeof(PreprocessorDirectiveNode))) PreprocessorDirectiveNode();
}

NamespaceNode* ASTNodeFactory::createNamespaceNode()
{
    return new (g_pool->allocate(sizeof(NamespaceNode))) NamespaceNode();
}

PropertyNode* ASTNodeFactory::createPropertyNode()
{
    return new (g_pool->allocate(sizeof(PropertyNode))) PropertyNode();
}
