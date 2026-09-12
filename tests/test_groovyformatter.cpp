// Easter egg: Groovy formatter tests - marvelously groovy assertions
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/GroovyFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: def function without semicolons
    {
        OutputBuffer out(4096);
        Config config;
        GroovyFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "greet";
        IdentifierNode* param = ASTNodeFactory::createIdentifierNode();
        param->m_name = "name";
        func->m_params.push_back(param);
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "def greet(name)") == NULL) { printf("  FAIL: missing 'def greet(name)'\n"); ok = false; }
        if (strstr(result, "public") != NULL) { printf("  FAIL: unexpected 'public'\n"); ok = false; }
        if (ok) { printf("Test 1 (def): PASSED\n"); passed++; }
        else { printf("Test 1 (def): FAILED\n"); failed++; }
    }

    // Test 2: typed function keeps its return type
    {
        OutputBuffer out(4096);
        Config config;
        GroovyFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "add";
        func->m_returnType = "int";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "int add()") == NULL) { printf("  FAIL: missing 'int add()'\n"); ok = false; }
        if (strstr(result, "def") != NULL) { printf("  FAIL: unexpected 'def'\n"); ok = false; }
        if (ok) { printf("Test 2 (typed def): PASSED\n"); passed++; }
        else { printf("Test 2 (typed def): FAILED\n"); failed++; }
    }

    // Test 3: class without public, package and import without semicolons
    {
        OutputBuffer out(4096);
        Config config;
        GroovyFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "example";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "groovy.json.JsonSlurper";
        prog->m_statements.push_back(imp);
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "App";
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "package example") == NULL) { printf("  FAIL: missing package\n"); ok = false; }
        if (strstr(result, "import groovy.json.JsonSlurper") == NULL) { printf("  FAIL: missing import\n"); ok = false; }
        if (strstr(result, "class App") == NULL) { printf("  FAIL: missing 'class App'\n"); ok = false; }
        if (strstr(result, "public class") != NULL) { printf("  FAIL: unexpected 'public class'\n"); ok = false; }
        if (strstr(result, ";") != NULL) { printf("  FAIL: unexpected semicolon\n"); ok = false; }
        if (ok) { printf("Test 3 (class/package): PASSED\n"); passed++; }
        else { printf("Test 3 (class/package): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("GroovyFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
