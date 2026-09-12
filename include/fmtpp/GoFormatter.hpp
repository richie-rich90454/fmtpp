#pragma once
#include "fmtpp/FormatterVisitor.hpp"

/**
 * Go code formatter.
 * Extends FormatterVisitor directly (not CstyleFormatterBase)
 * because Go has unique syntax: no semicolons, func keyword,
 * struct instead of class, implicit interfaces, packages.
 *
 * Easter egg: Go - where simplicity meets concurrency, and semicolons are ghosts
 */
class GoFormatter : public FormatterVisitor
{
public:
    GoFormatter(OutputBuffer& out, const Config& config);
    virtual ~GoFormatter();

    virtual void format(ASTNode* root);

    // --- Base class visitor overrides (delegate to Node-suffixed) ---
    virtual void visitProgram(ProgramNode& node);
    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitVariableDecl(VariableDeclNode& node);
    virtual void visitIfStmt(IfStmtNode& node);
    virtual void visitForStmt(ForStmtNode& node);
    virtual void visitWhileStmt(WhileStmtNode& node);
    virtual void visitSwitchStmt(SwitchStmtNode& node);
    virtual void visitBlock(BlockNode& node);
    virtual void visitReturnStmt(ReturnStmtNode& node);
    virtual void visitThrowStmt(ThrowStmtNode& node);
    virtual void visitTryCatchStmt(TryCatchStmtNode& node);
    virtual void visitComment(CommentNode& node);
    virtual void visitLiteral(LiteralNode& node);
    virtual void visitIdentifier(IdentifierNode& node);
    virtual void visitArrayLiteral(ArrayLiteralNode& node);
    virtual void visitObjectLiteral(ObjectLiteralNode& node);
    virtual void visitTemplateLiteral(TemplateLiteralNode& node);
    virtual void visitArrowFunction(ArrowFunctionNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitInterfaceDecl(InterfaceDeclNode& node);
    virtual void visitEnumDecl(EnumDeclNode& node);
    virtual void visitTypeAlias(TypeAliasNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitExportDecl(ExportDeclNode& node);
    virtual void visitPreprocessorDirective(PreprocessorDirectiveNode& node);
    virtual void visitNamespace(NamespaceNode& node);
    virtual void visitProperty(PropertyNode& node);
    virtual void visitRawText(RawTextNode& node);
    virtual void visitHtmlElement(HtmlElementNode& node);

protected:
    /** Visit a child node, skipping NULL */
    void visitNode(ASTNode* node);

    /** Print an opening brace in Go style (space before brace) */
    void printGoOpeningBrace();

    /** Print a closing brace */
    void printGoClosingBrace();

    /** Print a semicolon only in for-loop headers */
    void printGoSemicolon(bool inForLoop);

    // --- Go-specific node implementations ---
    virtual void visitProgramNode(ProgramNode& node);
    virtual void visitFunctionDeclNode(FunctionDeclNode& node);
    virtual void visitVariableDeclNode(VariableDeclNode& node);
    virtual void visitImportDeclNode(ImportDeclNode& node);
    virtual void visitBlockNode(BlockNode& node);
    virtual void visitReturnStmtNode(ReturnStmtNode& node);
    virtual void visitIfStmtNode(IfStmtNode& node);
    virtual void visitForStmtNode(ForStmtNode& node);
    virtual void visitWhileStmtNode(WhileStmtNode& node);
    virtual void visitSwitchStmtNode(SwitchStmtNode& node);
    virtual void visitTryCatchStmtNode(TryCatchStmtNode& node);
    virtual void visitCommentNode(CommentNode& node);
    virtual void visitLiteralNode(LiteralNode& node);
    virtual void visitIdentifierNode(IdentifierNode& node);
    virtual void visitArrayLiteralNode(ArrayLiteralNode& node);
    virtual void visitObjectLiteralNode(ObjectLiteralNode& node);
    virtual void visitTemplateLiteralNode(TemplateLiteralNode& node);
    virtual void visitArrowFunctionNode(ArrowFunctionNode& node);
    virtual void visitClassDeclNode(ClassDeclNode& node);
    virtual void visitInterfaceDeclNode(InterfaceDeclNode& node);
    virtual void visitEnumDeclNode(EnumDeclNode& node);
    virtual void visitTypeAliasNode(TypeAliasNode& node);
    virtual void visitExportDeclNode(ExportDeclNode& node);
    virtual void visitPreprocessorDirectiveNode(PreprocessorDirectiveNode& node);
    virtual void visitNamespaceNode(NamespaceNode& node);
    virtual void visitPropertyNode(PropertyNode& node);
    virtual void visitRawTextNode(RawTextNode& node);
    virtual void visitHtmlElementNode(HtmlElementNode& node);
    virtual void visitThrowStmtNode(ThrowStmtNode& node);
};