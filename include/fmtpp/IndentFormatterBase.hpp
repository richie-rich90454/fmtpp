#pragma once
#include "fmtpp/FormatterVisitor.hpp"

/**
 * Master base class for indentation-delimited languages (Python, Ruby).
 * No braces: suites print as newline + deeper indent. Provides the
 * shared machinery both languages reuse: program iteration, suite
 * printing, # comments, literals with verbatim strings, identifiers,
 * containers, verbatim nodes and graceful fallbacks.
 *
 * Easter egg: the indent master - where whitespace finally means something
 */
class IndentFormatterBase : public FormatterVisitor
{
public:
    IndentFormatterBase(OutputBuffer& out, const Config& config);
    virtual ~IndentFormatterBase();

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
    virtual void visitWith(WithNode& node);
    virtual void visitSwitchCase(SwitchCaseNode& node);

protected:
    /** Prints a suite: newline, deeper indent, statements, dedent.
        Accepts blocks, single statements (printed inline) or NULL. */
    void printSuite(ASTNode* body);

    /** Prints an empty suite body. Base prints nothing (Ruby-valid);
        Python prints an indented pass. No trailing newline (shared one). */
    virtual void printEmptySuite();

    /** Prints a value node (literal or identifier), never NULL-checked by callers. */
    void printValue(ASTNode* node);

    /** String literal, verbatim when already quoted. */
    void printStringLiteral(LiteralNode& node);
};
