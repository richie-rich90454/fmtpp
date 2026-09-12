// Easter egg: CSS formatter tests - !importantly passing assertions
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CssFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: rule with declarations keeps semicolons
    {
        OutputBuffer out(4096);
        Config config;
        CssFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* rule = ASTNodeFactory::createClassDeclNode();
        rule->m_name = ".app";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        PropertyNode* prop = ASTNodeFactory::createPropertyNode();
        IdentifierNode* key = ASTNodeFactory::createIdentifierNode();
        key->m_name = "color";
        prop->m_key = key;
        IdentifierNode* val = ASTNodeFactory::createIdentifierNode();
        val->m_name = "red";
        prop->m_value = val;
        body->m_statements.push_back(prop);
        rule->m_body = body;
        prog->m_statements.push_back(rule);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, ".app") == NULL) { printf("  FAIL: missing selector\n"); ok = false; }
        if (strstr(result, "color: red;") == NULL) { printf("  FAIL: missing 'color: red;'\n"); ok = false; }
        if (ok) { printf("Test 1 (rule): PASSED\n"); passed++; }
        else { printf("Test 1 (rule): FAILED\n"); failed++; }
    }

    // Test 2: @import with quotes, @media rule name raw
    {
        OutputBuffer out(4096);
        Config config;
        CssFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "reset.css";
        prog->m_statements.push_back(imp);
        ClassDeclNode* media = ASTNodeFactory::createClassDeclNode();
        media->m_name = "@media screen";
        media->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(media);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "@import \"reset.css\";") == NULL) { printf("  FAIL: missing '@import'\n"); ok = false; }
        if (strstr(result, "@media screen") == NULL) { printf("  FAIL: missing '@media screen'\n"); ok = false; }
        if (ok) { printf("Test 2 (at-rules): PASSED\n"); passed++; }
        else { printf("Test 2 (at-rules): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("CssFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
