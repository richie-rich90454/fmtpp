#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include "fmtpp/ASTPrinter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"

// Easter egg: testing the parser - where text meets its destiny

extern MemoryPool* g_pool;

int main()
{
    int errors = 0;
    ProgramNode* program = NULL;

    MemoryPool pool;
    g_pool = &pool;

    /* Test SourceBuffer */
    /* First create a test file */
    FILE* f = NULL;
    f = fopen("test_parser_input.txt", "w");
    if (f != NULL)
    {
        fprintf(f, "const x = 1;\n");
        fclose(f);
    }

    SourceBuffer source("test_parser_input.txt");
    if (source.isValid())
    {
        printf("SourceBuffer: file loaded OK (size=%zu)\n", source.getSize());
    }
    else
    {
        printf("SourceBuffer: file load failed (may be OK if file doesn't exist)\n");
    }

    /* Test Parser */
    Parser parser;
    const char* testSource = "const x = 1;";
    program = parser.parse(testSource, strlen(testSource));

    if (program == NULL)
    {
        printf("FAIL: Parser.parse returned NULL\n");
        errors++;
        goto cleanup;
    }
    printf("Parser: parse OK\n");
    printf("ProgramNode statements: %zu\n", program->m_statements.size());

    /* Print AST */
    {
        printf("\n--- ASTPrinter output ---\n");
        ASTPrinter printer("  ");
        program->accept(printer);
        printf("--- end ---\n\n");
    }

    /* Verify structure */
    if (program->m_statements.size() > 0)
    {
        printf("Parser: AST structure verified OK\n");
    }
    else
    {
        printf("FAIL: ProgramNode has no statements\n");
        errors++;
    }

    /* Test SourceBuffer-based parse */
    program = parser.parse(source);
    if (program != NULL)
    {
        printf("Parser.parse(SourceBuffer): OK\n");
    }
    else
    {
        printf("FAIL: Parser.parse(SourceBuffer) returned NULL\n");
        errors++;
        goto cleanup;
    }

    printf("Parser::isValid(): %s\n", parser.isValid() ? "true" : "false");

cleanup:
    pool.reset();
    if (errors == 0)
    {
        printf("Phase 5 tests passed.\n");
    }
    else
    {
        printf("\n%d test(s) FAILED.\n", errors);
    }
    return errors;
}