#pragma once
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTVisitor.hpp"
#include "fmtpp/Config.hpp"
#include <string>

/**
 * Style detector that analyzes source code to infer formatting preferences.
 * Real detection happens in detectFromSource(), which measures the raw
 * bytes. The ASTVisitor walk below only takes a node census: AST nodes
 * carry no style evidence (positions, quote characters, brace placement),
 * so the AST path assumes nothing and detect() returns defaults.
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
     * Walks an AST and returns the default configuration.
     * AST nodes carry no style evidence, so nothing is inferred here;
     * use detectFromSource() for real detection.
     * @param root The root AST node to analyze.
     * @return Default Config.
     */
    Config detect(ASTNode* root);

    /**
     * Analyzes raw source text and returns detected style configuration.
     * Measures indentation (tabs vs spaces, width), line widths, statement
     * semicolons, quote style, brace placement and line endings directly
     * from the bytes, so it works without a parsed AST.
     * Anything with no clear majority keeps the default value.
     * @param data Source bytes (need not be NUL-terminated).
     * @param len Number of bytes in data.
     * @return Config with detected style preferences.
     */
    Config detectFromSource(const char* data, size_t len);

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
    virtual void visitRawText(RawTextNode& node);
    virtual void visitHtmlElement(HtmlElementNode& node);
    virtual void visitWith(WithNode& node);
    virtual void visitSwitchCase(SwitchCaseNode& node);

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

    /** `{x` / `x}` tight curlies vs `{ x` / `x }` spaced. */
    int m_curlyTightCount;
    int m_curlySpacedCount;

    /** `[x` / `x]` tight squares vs `[ x` / `x ]` spaced. */
    int m_squareTightCount;
    int m_squareSpacedCount;

    /** Count of same-line braces. */
    int m_sameLineBraceCount;

    /** Count of next-line braces. */
    int m_nextLineBraceCount;

    /** Count of tabs used. */
    int m_tabCount;

    /** Count of spaces used for indentation. */
    int m_spaceIndentCount;

    /** Scan one trimmed code line for spacing signals. */
    void scanCodeLine(const char* d, size_t start, size_t end);

    /** `,x` vs `, x` after commas. */
    int m_commaTightCount;
    int m_commaSpacedCount;

    /** `(x` / `x)` tight parens vs `( x` / `x )` spaced. */
    int m_parenTightCount;
    int m_parenSpacedCount;

    /** Control parens `if(x` vs `if( x` (if/for/while/switch/catch). */
    int m_condParenTightCount;
    int m_condParenSpacedCount;

    /** Single arrow params with parens `(x) =>` vs bare `x =>`. */
    int m_arrowParenCount;
    int m_bareArrowParamCount;

    /** `=>` tight vs spaced. */
    int m_arrowTightCount;
    int m_arrowSpacedCount;

    /** `${x` vs `${ x` in template literals. */
    int m_templateTightCount;
    int m_templateSpacedCount;

    /** Non-empty lines ending with a comma. */
    int m_trailingCommaLineCount;
    /** Non-empty lines not ending with a comma. */
    int m_noTrailingCommaLineCount;

    /** Tag opens split across lines (`<tag` ... attrs). */
    int m_multilineTagCount;
    /** Tag opens closed on one line. */
    int m_singleLineTagCount;

    /** Longest blank-line run seen (in newlines). */
    int m_maxBlankRun;

    /** Blank line right after an import-ish line or not. */
    int m_blankAfterImportCount;
    int m_noBlankAfterImportCount;

    /** `=` with spaces both sides vs tight. */
    int m_assignSpacedCount;
    int m_assignTightCount;

    /** `:` followed by space vs glued (skips `::`). */
    int m_colonSpacedCount;
    int m_colonTightCount;

    /** `name (` vs `name(` before parens (non-keywords). */
    int m_funcSpaceCount;
    int m_funcTightCount;

    /** `){` vs `) {` before braces. */
    int m_braceTightCount;
    int m_braceSpacedCount;

    /** `if(` vs `if (` after control keywords. */
    int m_keywordTightCount;
    int m_keywordSpacedCount;

    /** `} else` / `} catch` / `} finally` joined vs split. */
    int m_elseJoinedCount;
    int m_elseSplitCount;
    int m_catchJoinedCount;
    int m_catchSplitCount;
    int m_finallyJoinedCount;
    int m_finallySplitCount;

    /** `elseif` occurrences (joining would rewrite the keyword). */
    int m_elseifCount;
    /** `else if` occurrences. */
    int m_elseIfCount;

    /** Blank line before `return` lines or not. */
    int m_blankBeforeReturnCount;
    int m_noBlankBeforeReturnCount;

    /** First line empty or not. */
    int m_leadingBlankCount;

    /** File ends with a newline or not. */
    int m_finalNewlineCount;
    int m_noFinalNewlineCount;

    /** File starts with a UTF-8 BOM. */
    int m_bomCount;

    /** Lines holding exactly `{}`. */
    int m_inlineEmptyCount;

    /** Split empty blocks (`{` line then `}` line). */
    int m_splitEmptyCount;

    /** Detected indentation width. */
    int m_detectedTabWidth;

    /** Maximum line width encountered. */
    int m_maxLineWidth;

    /** Visited node count for statistics. */
    int m_nodeCount;

    /** Detected config. */
    Config m_config;
};