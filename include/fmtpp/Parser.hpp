#pragma once
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include <cstddef>

/**
 * Parser wrapper around tree-sitter.
 * Converts source code into an AST (Abstract Syntax Tree)
 * using tree-sitter for parsing and ASTNodeFactory for node creation.
 *
 * Easter egg: the parser - turning text into trees, one token at a time
 */
class Parser
{
public:
    /** Constructs a parser. Tree-sitter initialization happens here. */
    Parser();

    /** Destroys the parser and cleans up tree-sitter resources. */
    ~Parser();

    /**
     * Parses source code into an AST.
     * @param source Pointer to the source text.
     * @param len Length of the source text in bytes.
     * @return ProgramNode* root of the AST, or NULL on error.
     */
    ProgramNode* parse(const char* source, size_t len);

    /**
     * Parses source code from a SourceBuffer.
     * @param source The source buffer containing the code.
     * @return ProgramNode* root of the AST, or NULL on error.
     */
    ProgramNode* parse(const SourceBuffer& source);

    /** Returns true if the parser is in a valid state. */
    bool isValid() const;

private:
    void* m_parser;      /* TSParser* - opaque until tree-sitter is linked */
    void* m_language;    /* TSLanguage* - opaque */
    bool m_isValid;
};