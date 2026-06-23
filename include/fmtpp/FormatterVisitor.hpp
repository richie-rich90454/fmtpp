#pragma once
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTVisitor.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"

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

protected:
    /** Print indentation based on current indent level */
    void printIndent();

    /** Print a newline and reset column counter */
    void printNewline();

    /** Print a single space */
    void printSpace();

    /** Print a token string, tracking column position */
    void printToken(const char* token);

    /** Print a comma followed by a space */
    void printComma();

    /** Print a semicolon if semicolons are enabled */
    void printSemicolon();

    /** Increase indentation level by one */
    void indent();

    /** Decrease indentation level by one (clamped at 0) */
    void dedent();

    OutputBuffer& m_out;
    const Config& m_config;
    int m_indentLevel;
    int m_currentColumn;
};