#include "fmtpp/Parser.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include <cstdio>

// Easter egg: parser initialization - tree-sitter not linked yet, using fallback

Parser::Parser()
    : m_parser(NULL)
    , m_language(NULL)
    , m_isValid(false)
{
    /* TODO: When tree-sitter is properly linked via CMake FetchContent,  */
    /* initialize TSParser and load tree_sitter_javascript() grammar     */
}

Parser::~Parser()
{
    /* Cleanup tree-sitter resources when they become available */
}

ProgramNode* Parser::parse(const char* source, size_t len)
{
    VariableDeclNode* varDecl = NULL;
    IdentifierNode* name = NULL;
    LiteralNode* init = NULL;

    if (source == NULL || len == 0)
    {
        return NULL;
    }

    /* Fallback: create a minimal AST for testing                     */
    /* This will be replaced with real tree-sitter parsing when the   */
    /* library is linked                                              */
    ProgramNode* program = ASTNodeFactory::createProgramNode();
    if (program == NULL)
    {
        return NULL;
    }

    /* Create a placeholder variable declaration */
    varDecl = ASTNodeFactory::createVariableDeclNode();
    if (varDecl == NULL)
    {
        goto cleanup;
    }

    varDecl->m_kind = "const";

    name = ASTNodeFactory::createIdentifierNode();
    if (name == NULL)
    {
        goto cleanup;
    }
    name->m_name = "placeholder";

    init = ASTNodeFactory::createLiteralNode();
    if (init == NULL)
    {
        goto cleanup;
    }
    init->m_value = "0";
    init->m_type = LiteralNode::TYPE_NUMBER;

    varDecl->m_name = name->m_name;
    varDecl->m_initializer = init;
    program->m_statements.push_back(varDecl);

    return program;

cleanup:
    /* On allocation failure, the pool will clean up. */
    /* Return NULL to indicate parse error.           */
    return NULL;
}

ProgramNode* Parser::parse(const SourceBuffer& source)
{
    return parse(source.getData(), source.getSize());
}

bool Parser::isValid() const
{
    return m_isValid;
}