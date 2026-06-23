#pragma once
#include "fmtpp/FormatterVisitor.hpp"

/**
 * Master base class for all C-style language formatters.
 * Provides default formatting behavior for common constructs:
 * blocks, if/for/while/switch statements, function/variable declarations,
 * literals, identifiers, and more.
 *
 * Subclasses (JSFormatter, JavaFormatter, CFormatter, CSharpFormatter)
 * override specific virtual methods to customize behavior per language.
 *
 * Easter egg: the master formatter - 800 lines of pure formatting glory
 */
class CstyleFormatterBase : public FormatterVisitor
{
public:
    CstyleFormatterBase(OutputBuffer& out, const Config& config);
    virtual ~CstyleFormatterBase();

    virtual void format(ASTNode* root);

    // --- Visitor methods (implemented in base) ---
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

protected:
    /** Called before formatting begins. Override for setup. */
    virtual void preFormat();

    /** Called after formatting ends. Adds trailing newline. */
    virtual void postFormat();

    /** Whether braces go on the same line as the declaration */
    virtual bool braceOnSameLine() const;

    /** Whether to add a space before the opening brace */
    virtual bool spaceBeforeOpeningBrace() const;

    /** Whether to add a space after keywords like if, for, while */
    virtual bool spaceAfterKeywords() const;

    /** Whether a semicolon is required after this node */
    virtual bool requiresSemicolon(ASTNode* node) const;

    /** Print an opening brace with optional preceding space */
    void printOpeningBrace();

    /** Print a closing brace */
    void printClosingBrace();

    /** Wrap lines that exceed print width */
    void wrapLines();

    /** Visit a child node, skipping NULL */
    void visitNode(ASTNode* node);

    /** Print a semicolon if one is needed for this node */
    void printSemicolonIfNeeded(ASTNode* node);
};