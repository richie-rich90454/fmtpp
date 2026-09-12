// Easter egg: GLSL formatter tests - vertex-tested, fragment-approved
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/GlslFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
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

    // Test 1: struct keeps kind and trailing semicolon
    {
        OutputBuffer out(4096);
        Config config;
        GlslFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Light";
        cls->m_kind = "struct";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        VariableDeclNode* field = ASTNodeFactory::createVariableDeclNode();
        field->m_kind = "vec3";
        field->m_name = "position";
        body->m_statements.push_back(field);
        cls->m_body = body;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "struct Light") == NULL) { printf("  FAIL: missing 'struct Light'\n"); ok = false; }
        if (strstr(result, "vec3 position;") == NULL) { printf("  FAIL: missing 'vec3 position;'\n"); ok = false; }
        if (strstr(result, "};") == NULL) { printf("  FAIL: missing trailing '};'\n"); ok = false; }
        if (ok) { printf("Test 1 (struct): PASSED\n"); passed++; }
        else { printf("Test 1 (struct): FAILED\n"); failed++; }
    }

    // Test 2: version, precision, uniform, main
    {
        OutputBuffer out(4096);
        Config config;
        GlslFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* ver = ASTNodeFactory::createPreprocessorDirectiveNode();
        ver->m_directive = "#version 330 core";
        prog->m_statements.push_back(ver);
        VariableDeclNode* uni = ASTNodeFactory::createVariableDeclNode();
        uni->m_kind = "uniform vec3";
        uni->m_name = "color";
        prog->m_statements.push_back(uni);
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "#version 330 core") == NULL) { printf("  FAIL: missing version\n"); ok = false; }
        if (strstr(result, "uniform vec3 color;") == NULL) { printf("  FAIL: missing uniform\n"); ok = false; }
        if (strstr(result, "void main()") == NULL) { printf("  FAIL: missing 'void main()'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function'\n"); ok = false; }
        if (ok) { printf("Test 2 (shader prelude): PASSED\n"); passed++; }
        else { printf("Test 2 (shader prelude): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("GlslFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
