#pragma once
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTVisitor.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include <vector>

/**
 * Base formatter visitor that provides formatting primitives.
 * All formatters extend this class to get indentation, spacing,
 * newline, semicolon, and token printing functionality.
 *
 * Easter egg: the formatter visitor - where indentation dreams come true
 */
class FormatterVisitor : public ASTVisitor
{
public:
    FormatterVisitor(OutputBuffer& out, const Config& config);
    virtual ~FormatterVisitor();

    virtual void format(ASTNode* root) = 0;

    /**
     * Convert a line-feed buffer to the configured endOfLine for output
     * (lf/auto keep newlines, crlf uses carriage pairs, cr uses bare
     * carriage returns). Applied once at the CLI edge, never in tests.
     */
    std::string applyEndOfLine(const std::string& text) const;

protected:
    /** Print indentation based on current indent level */
    void printIndent();

    /** Print a newline and reset column counter */
    void printNewline();

    /**
     * Ensure `blanks` blank lines (buffer ends with blanks+1 newlines).
     * Re-emits input separators stamped as m_blankLinesBefore and backs
     * the blank-line options. Capped by maxBlankLines, never leads the
     * output, never stacks beyond the cap.
     */
    void printBlankLine(int blanks);

    /** Print a single space */
    void printSpace();

    /** Print a token string, tracking column position */
    void printToken(const char* token);

    /** Print a comma followed by a space */
    void printComma();

    /** Print a semicolon if semicolons are enabled */
    void printSemicolon();

    /** Print `=` with optional surrounding spaces */
    void printAssign();

    /** Print `:=` with optional surrounding spaces */
    void printShortAssign();

    /** Increase indentation level by one */
    void indent();

    /**
     * Emit blank separators before statement `stmts[i]`: preserved input
     * gaps (m_blankLinesBefore, capped by maxBlankLines), a blank after
     * a trailing import block, and a blank before returns, each gated
     * by its own option. No-ops stay silent, so loops call it blindly.
     */
    void printStatementBlanks(const std::vector<ASTNode*>& stmts, size_t i);

    /** Decrease indentation level by one (clamped at 0) */
    void dedent();

    OutputBuffer& m_out;
    const Config& m_config;
    int m_indentLevel;
    int m_currentColumn;
};