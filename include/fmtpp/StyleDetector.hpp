#pragma once
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTVisitor.hpp"
#include "fmtpp/Config.hpp"
#include <string>

/**
 * Style detector that analyzes source code to infer formatting preferences.
 * Extends ASTVisitor to walk the AST and collect style statistics.
 * Used by the "fmtpp learn" command to generate a .fmtpp.toml file.
 *
 * Easter egg: the style detective - reading code to understand your formatting soul
 */
class StyleDetector : public ASTVisitor
{
public:
    StyleDetector();
    virtual ~StyleDetector();

    /**
     * Analyzes source code and returns detected style configuration.
     * @param root The root AST node to analyze.
     * @return Config with detected style preferences.
     */
    Config detect(ASTNode* root);

    // --- Visitor methods ---
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

    /**
     * Writes the detected configuration to a .fmtpp.toml file.
     * @param filepath Path to write the config file.
     * @return true on success.
     */
    bool writeConfig(const std::string& filepath) const;

private:
    /** Count of semicolons found. */
    int m_semicolonCount;

    /** Count of statements without semicolons. */
    int m_noSemicolonCount;

    /** Count of single-quoted strings. */
    int m_singleQuoteCount;

    /** Count of double-quoted strings. */
    int m_doubleQuoteCount;

    /** Count of same-line braces. */
    int m_sameLineBraceCount;

    /** Count of next-line braces. */
    int m_nextLineBraceCount;

    /** Count of tabs used. */
    int m_tabCount;

    /** Count of spaces used for indentation. */
    int m_spaceIndentCount;

    /** Detected indentation width. */
    int m_detectedTabWidth;

    /** Maximum line width encountered. */
    int m_maxLineWidth;

    /** Visited node count for statistics. */
    int m_nodeCount;

    /** Detected config. */
    Config m_config;
};