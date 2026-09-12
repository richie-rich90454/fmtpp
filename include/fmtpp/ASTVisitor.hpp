#pragma once

// Forward declarations of all AST node types
class ProgramNode;
class FunctionDeclNode;
class VariableDeclNode;
class IfStmtNode;
class ForStmtNode;
class WhileStmtNode;
class SwitchStmtNode;
class BlockNode;
class ReturnStmtNode;
class ThrowStmtNode;
class TryCatchStmtNode;
class CommentNode;
class LiteralNode;
class IdentifierNode;
class ArrayLiteralNode;
class ObjectLiteralNode;
class TemplateLiteralNode;
class ArrowFunctionNode;
class ClassDeclNode;
class InterfaceDeclNode;
class EnumDeclNode;
class TypeAliasNode;
class ImportDeclNode;
class ExportDeclNode;
class PreprocessorDirectiveNode;
class NamespaceNode;
class PropertyNode;
class RawTextNode;
class HtmlElementNode;
class WithNode;
class SwitchCaseNode;

/**
 * Abstract visitor interface for AST traversal.
 * Implementations override visit methods to perform operations on the AST.
 * 
 * Easter egg: the visitor that walks the tree, spreading format wherever it goes
 */
class ASTVisitor
{
public:
    virtual ~ASTVisitor();

    virtual void visitProgram(ProgramNode& node) = 0;
    virtual void visitFunctionDecl(FunctionDeclNode& node) = 0;
    virtual void visitVariableDecl(VariableDeclNode& node) = 0;
    virtual void visitIfStmt(IfStmtNode& node) = 0;
    virtual void visitForStmt(ForStmtNode& node) = 0;
    virtual void visitWhileStmt(WhileStmtNode& node) = 0;
    virtual void visitSwitchStmt(SwitchStmtNode& node) = 0;
    virtual void visitBlock(BlockNode& node) = 0;
    virtual void visitReturnStmt(ReturnStmtNode& node) = 0;
    virtual void visitThrowStmt(ThrowStmtNode& node) = 0;
    virtual void visitTryCatchStmt(TryCatchStmtNode& node) = 0;
    virtual void visitComment(CommentNode& node) = 0;
    virtual void visitLiteral(LiteralNode& node) = 0;
    virtual void visitIdentifier(IdentifierNode& node) = 0;
    virtual void visitArrayLiteral(ArrayLiteralNode& node) = 0;
    virtual void visitObjectLiteral(ObjectLiteralNode& node) = 0;
    virtual void visitTemplateLiteral(TemplateLiteralNode& node) = 0;
    virtual void visitArrowFunction(ArrowFunctionNode& node) = 0;
    virtual void visitClassDecl(ClassDeclNode& node) = 0;
    virtual void visitInterfaceDecl(InterfaceDeclNode& node) = 0;
    virtual void visitEnumDecl(EnumDeclNode& node) = 0;
    virtual void visitTypeAlias(TypeAliasNode& node) = 0;
    virtual void visitImportDecl(ImportDeclNode& node) = 0;
    virtual void visitExportDecl(ExportDeclNode& node) = 0;
    virtual void visitPreprocessorDirective(PreprocessorDirectiveNode& node) = 0;
    virtual void visitNamespace(NamespaceNode& node) = 0;
    virtual void visitProperty(PropertyNode& node) = 0;
    virtual void visitRawText(RawTextNode& node) = 0;
    virtual void visitHtmlElement(HtmlElementNode& node) = 0;
    virtual void visitWith(WithNode& node) = 0;
    virtual void visitSwitchCase(SwitchCaseNode& node) = 0;
};
