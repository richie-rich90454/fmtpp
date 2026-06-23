#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/ASTPrinter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"

// Easter egg: the test that proves the tree works - if it prints, it lives!

int main()
{
    int errors = 0;
    ProgramNode* program = NULL;
    VariableDeclNode* varDecl = NULL;
    LiteralNode* literal = NULL;

    // 1. Create a MemoryPool
    MemoryPool pool;
    printf("MemoryPool created: OK\n");

    // 2. Set g_pool
    g_pool = &pool;
    printf("g_pool set: OK\n");

    // 3. Use ASTNodeFactory to create nodes
    program = ASTNodeFactory::createProgramNode();
    if (program == NULL)
    {
        printf("FAIL: createProgramNode returned NULL\n");
        errors++;
        goto cleanup;
    }
    printf("ProgramNode created: OK\n");

    varDecl = ASTNodeFactory::createVariableDeclNode();
    if (varDecl == NULL)
    {
        printf("FAIL: createVariableDeclNode returned NULL\n");
        errors++;
        goto cleanup;
    }
    varDecl->m_kind = "let";
    varDecl->m_name = "x";
    varDecl->m_type = "number";
    varDecl->m_initializer = NULL;
    printf("VariableDeclNode created: OK\n");

    literal = ASTNodeFactory::createLiteralNode();
    if (literal == NULL)
    {
        printf("FAIL: createLiteralNode returned NULL\n");
        errors++;
        goto cleanup;
    }
    literal->m_value = "42";
    literal->m_type = LiteralNode::TYPE_NUMBER;
    printf("LiteralNode created: OK\n");

    varDecl->m_initializer = literal;
    program->m_statements.push_back(varDecl);

    // 4. Run ASTPrinter to print the tree
    {
        printf("\n--- ASTPrinter output ---\n");
        ASTPrinter printer("  ");
        program->accept(printer);
        printf("--- end ---\n\n");
    }

    // 5. Verify output contains expected node names
    if (program->m_statements.size() == 1)
    {
        printf("Verification: ProgramNode has 1 statement: OK\n");
    }
    else
    {
        printf("FAIL: ProgramNode has %d statements, expected 1\n",
               (int)program->m_statements.size());
        errors++;
    }

    if (varDecl->m_name == "x")
    {
        printf("Verification: VariableDeclNode name is 'x': OK\n");
    }
    else
    {
        printf("FAIL: VariableDeclNode name mismatch\n");
        errors++;
    }

    if (varDecl->m_initializer != NULL)
    {
        printf("Verification: VariableDeclNode has initializer: OK\n");
    }
    else
    {
        printf("FAIL: VariableDeclNode has no initializer\n");
        errors++;
    }

cleanup:
    if (errors == 0)
    {
        printf("\nAll Phase 3 and 4 tests passed.\n");
    }
    else
    {
        printf("\n%d test(s) FAILED.\n", errors);
    }
    return errors;
}
