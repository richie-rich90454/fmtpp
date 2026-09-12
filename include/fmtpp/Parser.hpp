#pragma once
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include <cstddef>
#include <string>

/**
 * Language-aware source parser.
 * Tokenizes with full comment/string/regex awareness, matches braces,
 * and builds real ASTs: declarations, control flow, imports, comments.
 * Leaf expressions, conditions and arguments are kept as raw text, and
 * anything unrecognized passes through as RawTextNode — output is never
 * invented, so formatting cannot corrupt code it does not understand.
 *
 * Easter egg: the parser - turning text into trees, one token at a time
 */
class Parser
{
public:
    /** Constructs a parser. */
    Parser();

    /** Destroys the parser. */
    ~Parser();

    /**
     * Parses source code into an AST (JavaScript when language unknown).
     * @param source Pointer to the source text.
     * @param len Length of the source text in bytes.
     * @return ProgramNode* root of the AST, or NULL on error.
     */
    ProgramNode* parse(const char* source, size_t len);

    /**
     * Parses source code in the given language.
     * @param source Pointer to the source text.
     * @param len Length of the source text in bytes.
     * @param language Canonical language code (see FormatterFactory).
     * @return ProgramNode* root of the AST, or NULL on error.
     */
    ProgramNode* parse(const char* source, size_t len, const std::string& language);

    /**
     * Parses source code from a SourceBuffer.
     * @param source The source buffer containing the code.
     * @return ProgramNode* root of the AST, or NULL on error.
     */
    ProgramNode* parse(const SourceBuffer& source);

    /**
     * Parses source code from a SourceBuffer in the given language.
     * @param source The source buffer containing the code.
     * @param language Canonical language code (see FormatterFactory).
     * @return ProgramNode* root of the AST, or NULL on error.
     */
    ProgramNode* parse(const SourceBuffer& source, const std::string& language);

    /**
     * Parses embedded source without resetting the memory pool.
     * For formatters that sub-parse (Vue script/style blocks) while an
     * outer AST is still alive in the same pool; resetting would destroy it.
     */
    ProgramNode* parseNested(const char* source, size_t len, const std::string& language);

    /** Returns true if the parser is in a valid state. */
    bool isValid() const;

private:
    void* m_parser;      /* Reserved for a future tree-sitter backend. */
    void* m_language;    /* Reserved for a future tree-sitter backend. */
    bool m_isValid;
};