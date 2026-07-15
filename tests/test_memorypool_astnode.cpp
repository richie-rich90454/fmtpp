#include <cstdio>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"

// Easter egg: the memory pool test - where a thousand nodes are born and none are leaked

extern MemoryPool* g_pool;

int main()
{
    int failures = 0;

    // Test 1: MemoryPool default size is 10MB
    {
        MemoryPool pool;
        if (pool.getCapacity() != 10 * 1024 * 1024)
        {
            printf("FAIL: default pool capacity should be 10MB, got %zu\n", pool.getCapacity());
            failures++;
        }
        else
        {
            printf("MemoryPool default capacity: PASSED\n");
        }
    }

    // Test 2: Bump-pointer allocation
    {
        MemoryPool pool(1024);
        g_pool = &pool;

        void* p1 = pool.allocate(100);
        void* p2 = pool.allocate(100);
        if (p1 == NULL || p2 == NULL)
        {
            printf("FAIL: allocation from pool returned NULL\n");
            failures++;
        }
        else if (p2 <= p1)
        {
            printf("FAIL: bump pointer did not advance\n");
            failures++;
        }
        else
        {
            printf("MemoryPool bump allocation: PASSED\n");
        }

        g_pool = NULL;
    }

    // Test 3: Pool exhaustion returns NULL
    {
        MemoryPool pool(64);
        void* p = pool.allocate(128);
        if (p != NULL)
        {
            printf("FAIL: over-allocation should return NULL\n");
            failures++;
        }
        else
        {
            printf("MemoryPool exhaustion: PASSED\n");
        }
    }

    // Test 4: Bulk AST node allocation
    {
        MemoryPool pool;
        g_pool = &pool;

        ProgramNode* program = ASTNodeFactory::createProgramNode();
        if (program == NULL)
        {
            printf("FAIL: createProgramNode returned NULL\n");
            failures++;
            g_pool = NULL;
            return failures;
        }

        for (int i = 0; i < 1000; i++)
        {
            VariableDeclNode* var = ASTNodeFactory::createVariableDeclNode();
            LiteralNode* lit = ASTNodeFactory::createLiteralNode();
            if (var == NULL || lit == NULL)
            {
                printf("FAIL: bulk allocation failed at iteration %d\n", i);
                failures++;
                break;
            }
            var->m_name = "x";
            var->m_initializer = lit;
            program->m_statements.push_back(var);
        }

        if (program->m_statements.size() == 1000)
        {
            printf("Bulk AST node allocation (1000 nodes): PASSED\n");
        }
        else
        {
            printf("FAIL: expected 1000 nodes, got %zu\n", program->m_statements.size());
            failures++;
        }

        g_pool = NULL;
    }

    // Test 5: Pool reset
    {
        MemoryPool pool(1024);
        g_pool = &pool;

        void* p1 = pool.allocate(100);
        if (p1 == NULL)
        {
            printf("FAIL: allocation before reset failed\n");
            failures++;
        }
        else
        {
            pool.reset();
            if (pool.getUsed() != 0)
            {
                printf("FAIL: pool used should be 0 after reset, got %zu\n", pool.getUsed());
                failures++;
            }
            else
            {
                printf("MemoryPool reset: PASSED\n");
            }
        }

        g_pool = NULL;
    }

    if (failures == 0)
    {
        printf("All MemoryPool and ASTNode tests passed.\n");
    }
    else
    {
        printf("MemoryPool and ASTNode tests FAILED: %d failure(s)\n", failures);
    }

    return failures;
}
