#pragma once
#include "fmtpp/FormatterVisitor.hpp"

/**
 * Rust code formatter.
 * Extends FormatterVisitor directly with Rust-specific formatting behavior.
 * Handles fn with modifiers and -> returns, let/let mut bindings, match
 * expressions, struct/impl/trait/enum/type/mod declarations, use paths,
 * vec! and panic! macros, closures and expression-based returns.
 * Output matches rustfmt default style.
 */
class RustFormatter : public FormatterVisitor
{
public:
    RustFormatter(OutputBuffer& out, const Config& config);
    virtual ~RustFormatter();
    virtual void format(ASTNode* root);

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

    // Easter egg: Rust - where the borrow checker is always watching
};