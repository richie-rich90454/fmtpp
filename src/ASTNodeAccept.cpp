#include "fmtpp/ASTVisitor.hpp"
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

// Easter egg: the accept() implementations - the bridge between nodes and visitors

ASTVisitor::~ASTVisitor()
{
}

// Constructor/destructor implementations for all nodes
ProgramNode::ProgramNode() {}
ProgramNode::~ProgramNode() {}
void ProgramNode::accept(ASTVisitor& visitor) { visitor.visitProgram(*this); }

FunctionDeclNode::FunctionDeclNode() : m_body(NULL) {}
FunctionDeclNode::~FunctionDeclNode() {}
void FunctionDeclNode::accept(ASTVisitor& visitor) { visitor.visitFunctionDecl(*this); }

VariableDeclNode::VariableDeclNode() : m_initializer(NULL) {}
VariableDeclNode::~VariableDeclNode() {}
void VariableDeclNode::accept(ASTVisitor& visitor) { visitor.visitVariableDecl(*this); }

IfStmtNode::IfStmtNode() : m_condition(NULL), m_thenBranch(NULL), m_elseBranch(NULL) {}
IfStmtNode::~IfStmtNode() {}
void IfStmtNode::accept(ASTVisitor& visitor) { visitor.visitIfStmt(*this); }

ForStmtNode::ForStmtNode() : m_init(NULL), m_condition(NULL), m_update(NULL), m_body(NULL) {}
ForStmtNode::~ForStmtNode() {}
void ForStmtNode::accept(ASTVisitor& visitor) { visitor.visitForStmt(*this); }

WhileStmtNode::WhileStmtNode() : m_condition(NULL), m_body(NULL) {}
WhileStmtNode::~WhileStmtNode() {}
void WhileStmtNode::accept(ASTVisitor& visitor) { visitor.visitWhileStmt(*this); }

SwitchStmtNode::SwitchStmtNode() : m_discriminant(NULL) {}
SwitchStmtNode::~SwitchStmtNode() {}
void SwitchStmtNode::accept(ASTVisitor& visitor) { visitor.visitSwitchStmt(*this); }

BlockNode::BlockNode() {}
BlockNode::~BlockNode() {}
void BlockNode::accept(ASTVisitor& visitor) { visitor.visitBlock(*this); }

ReturnStmtNode::ReturnStmtNode() : m_argument(NULL) {}
ReturnStmtNode::~ReturnStmtNode() {}
void ReturnStmtNode::accept(ASTVisitor& visitor) { visitor.visitReturnStmt(*this); }

ThrowStmtNode::ThrowStmtNode() : m_expression(NULL) {}
ThrowStmtNode::~ThrowStmtNode() {}
void ThrowStmtNode::accept(ASTVisitor& visitor) { visitor.visitThrowStmt(*this); }

TryCatchStmtNode::TryCatchStmtNode() : m_body(NULL), m_catchClause(NULL), m_finallyBlock(NULL) {}
TryCatchStmtNode::~TryCatchStmtNode() {}
void TryCatchStmtNode::accept(ASTVisitor& visitor) { visitor.visitTryCatchStmt(*this); }

CommentNode::CommentNode() : m_isBlock(false) {}
CommentNode::~CommentNode() {}
void CommentNode::accept(ASTVisitor& visitor) { visitor.visitComment(*this); }

LiteralNode::LiteralNode() : m_type(TYPE_STRING) {}
LiteralNode::~LiteralNode() {}
void LiteralNode::accept(ASTVisitor& visitor) { visitor.visitLiteral(*this); }

IdentifierNode::IdentifierNode() {}
IdentifierNode::~IdentifierNode() {}
void IdentifierNode::accept(ASTVisitor& visitor) { visitor.visitIdentifier(*this); }

ArrayLiteralNode::ArrayLiteralNode() {}
ArrayLiteralNode::~ArrayLiteralNode() {}
void ArrayLiteralNode::accept(ASTVisitor& visitor) { visitor.visitArrayLiteral(*this); }

ObjectLiteralNode::ObjectLiteralNode() {}
ObjectLiteralNode::~ObjectLiteralNode() {}
void ObjectLiteralNode::accept(ASTVisitor& visitor) { visitor.visitObjectLiteral(*this); }

TemplateLiteralNode::TemplateLiteralNode() {}
TemplateLiteralNode::~TemplateLiteralNode() {}
void TemplateLiteralNode::accept(ASTVisitor& visitor) { visitor.visitTemplateLiteral(*this); }

ArrowFunctionNode::ArrowFunctionNode() : m_body(NULL) {}
ArrowFunctionNode::~ArrowFunctionNode() {}
void ArrowFunctionNode::accept(ASTVisitor& visitor) { visitor.visitArrowFunction(*this); }

ClassDeclNode::ClassDeclNode() : m_superClass(NULL), m_body(NULL) {}
ClassDeclNode::~ClassDeclNode() {}
void ClassDeclNode::accept(ASTVisitor& visitor) { visitor.visitClassDecl(*this); }

InterfaceDeclNode::InterfaceDeclNode() : m_body(NULL) {}
InterfaceDeclNode::~InterfaceDeclNode() {}
void InterfaceDeclNode::accept(ASTVisitor& visitor) { visitor.visitInterfaceDecl(*this); }

EnumDeclNode::EnumDeclNode() {}
EnumDeclNode::~EnumDeclNode() {}
void EnumDeclNode::accept(ASTVisitor& visitor) { visitor.visitEnumDecl(*this); }

TypeAliasNode::TypeAliasNode() : m_typeAnnotation(NULL) {}
TypeAliasNode::~TypeAliasNode() {}
void TypeAliasNode::accept(ASTVisitor& visitor) { visitor.visitTypeAlias(*this); }

ImportDeclNode::ImportDeclNode() : m_defaultImport(NULL) {}
ImportDeclNode::~ImportDeclNode() {}
void ImportDeclNode::accept(ASTVisitor& visitor) { visitor.visitImportDecl(*this); }

ExportDeclNode::ExportDeclNode() : m_declaration(NULL) {}
ExportDeclNode::~ExportDeclNode() {}
void ExportDeclNode::accept(ASTVisitor& visitor) { visitor.visitExportDecl(*this); }

PreprocessorDirectiveNode::PreprocessorDirectiveNode() {}
PreprocessorDirectiveNode::~PreprocessorDirectiveNode() {}
void PreprocessorDirectiveNode::accept(ASTVisitor& visitor) { visitor.visitPreprocessorDirective(*this); }

NamespaceNode::NamespaceNode() : m_body(NULL) {}
NamespaceNode::~NamespaceNode() {}
void NamespaceNode::accept(ASTVisitor& visitor) { visitor.visitNamespace(*this); }

PropertyNode::PropertyNode() : m_key(NULL), m_value(NULL) {}
PropertyNode::~PropertyNode() {}
void PropertyNode::accept(ASTVisitor& visitor) { visitor.visitProperty(*this); }

