// Easter egg: V formatter tests - v is for victory over ugly code
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/VFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: mut binding keeps mut
    {
        OutputBuffer out(4096);
        Config config;
        VFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "mut";
        varDecl->m_name = "count";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "0";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "mut count := 0") == NULL) { printf("  FAIL: missing 'mut count := 0'\n"); ok = false; }
        if (ok) { printf("Test 1 (mut): PASSED\n"); passed++; }
        else { printf("Test 1 (mut): FAILED\n"); failed++; }
    }

    // Test 2: real enum, not iota
    {
        OutputBuffer out(4096);
        Config config;
        VFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "Color";
        IdentifierNode* member = ASTNodeFactory::createIdentifierNode();
        member->m_name = "red";
        enm->m_members.push_back(member);
        prog->m_statements.push_back(enm);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "enum Color") == NULL) { printf("  FAIL: missing 'enum Color'\n"); ok = false; }
        if (strstr(result, "red") == NULL) { printf("  FAIL: missing 'red'\n"); ok = false; }
        if (strstr(result, "iota") != NULL) { printf("  FAIL: unexpected 'iota'\n"); ok = false; }
        if (ok) { printf("Test 2 (enum): PASSED\n"); passed++; }
        else { printf("Test 2 (enum): FAILED\n"); failed++; }
    }

    // Test 3: module, unquoted import, verbatim flag
    {
        OutputBuffer out(4096);
        Config config;
        VFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "main";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "os";
        prog->m_statements.push_back(imp);
        PreprocessorDirectiveNode* flag = ASTNodeFactory::createPreprocessorDirectiveNode();
        flag->m_directive = "#flag -lm";
        prog->m_statements.push_back(flag);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "module main") == NULL) { printf("  FAIL: missing 'module main'\n"); ok = false; }
        if (strstr(result, "import os") == NULL) { printf("  FAIL: missing 'import os'\n"); ok = false; }
        if (strstr(result, "\"os\"") != NULL) { printf("  FAIL: import must not be quoted\n"); ok = false; }
        if (strstr(result, "#flag -lm") == NULL) { printf("  FAIL: missing '#flag -lm'\n"); ok = false; }
        if (strstr(result, "go:build") != NULL) { printf("  FAIL: unexpected 'go:build'\n"); ok = false; }
        if (ok) { printf("Test 3 (module/import/flag): PASSED\n"); passed++; }
        else { printf("Test 3 (module/import/flag): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("VFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
