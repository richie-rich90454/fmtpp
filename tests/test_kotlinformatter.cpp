// Easter egg: Kotlin formatter tests - null safety, even in tests
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/KotlinFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: fun with return type, no semicolons
    {
        OutputBuffer out(4096);
        Config config;
        KotlinFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "add";
        func->m_returnType = "Int";
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "a: Int";
        func->m_params.push_back(paramA);
        BlockNode* body = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        IdentifierNode* arg = ASTNodeFactory::createIdentifierNode();
        arg->m_name = "a";
        ret->m_argument = arg;
        body->m_statements.push_back(ret);
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "fun add(a: Int): Int") == NULL) { printf("  FAIL: missing 'fun add(a: Int): Int'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function'\n"); ok = false; }
        if (strstr(result, "return a;") != NULL) { printf("  FAIL: unexpected semicolon\n"); ok = false; }
        if (ok) { printf("Test 1 (fun decl): PASSED\n"); passed++; }
        else { printf("Test 1 (fun decl): FAILED\n"); failed++; }
    }

    // Test 2: class with supertype and trait
    {
        OutputBuffer out(4096);
        Config config;
        KotlinFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
        sup->m_name = "Animal()";
        cls->m_superClass = sup;
        IdentifierNode* trait = ASTNodeFactory::createIdentifierNode();
        trait->m_name = "Pet";
        cls->m_implements.push_back(trait);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class Dog : Animal(), Pet") == NULL) { printf("  FAIL: missing 'class Dog : Animal(), Pet'\n"); ok = false; }
        if (strstr(result, "public") != NULL) { printf("  FAIL: unexpected 'public'\n"); ok = false; }
        if (strstr(result, "extends") != NULL) { printf("  FAIL: unexpected 'extends'\n"); ok = false; }
        if (ok) { printf("Test 2 (class): PASSED\n"); passed++; }
        else { printf("Test 2 (class): FAILED\n"); failed++; }
    }

    // Test 3: import without semicolon, package without semicolon
    {
        OutputBuffer out(4096);
        Config config;
        KotlinFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "com.example";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "kotlin.math.PI";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "package com.example") == NULL) { printf("  FAIL: missing package\n"); ok = false; }
        if (strstr(result, "import kotlin.math.PI") == NULL) { printf("  FAIL: missing import\n"); ok = false; }
        if (strstr(result, ";") != NULL) { printf("  FAIL: unexpected semicolon\n"); ok = false; }
        if (ok) { printf("Test 3 (package/import): PASSED\n"); passed++; }
        else { printf("Test 3 (package/import): FAILED\n"); failed++; }
    }

    // Test 4: when expression, arms verbatim
    {
        OutputBuffer out(4096);
        Config config;
        KotlinFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "x";
        sw->m_discriminant = disc;
        IdentifierNode* arm = ASTNodeFactory::createIdentifierNode();
        arm->m_name = "1 -> one()";
        sw->m_cases.push_back(arm);
        prog->m_statements.push_back(sw);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "when (x)") == NULL) { printf("  FAIL: missing 'when (x)'\n"); ok = false; }
        if (strstr(result, "1 -> one()") == NULL) { printf("  FAIL: missing arm\n"); ok = false; }
        if (strstr(result, "switch") != NULL) { printf("  FAIL: unexpected 'switch'\n"); ok = false; }
        if (ok) { printf("Test 4 (when): PASSED\n"); passed++; }
        else { printf("Test 4 (when): FAILED\n"); failed++; }
    }

    // Test 5: val declaration, object kind
    {
        OutputBuffer out(4096);
        Config config;
        KotlinFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "val";
        varDecl->m_name = "pi";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "3.14";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);
        ClassDeclNode* obj = ASTNodeFactory::createClassDeclNode();
        obj->m_name = "Singleton";
        obj->m_kind = "object";
        obj->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(obj);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "val pi = 3.14") == NULL) { printf("  FAIL: missing 'val pi = 3.14'\n"); ok = false; }
        if (strstr(result, "object Singleton") == NULL) { printf("  FAIL: missing 'object Singleton'\n"); ok = false; }
        if (ok) { printf("Test 5 (val/object): PASSED\n"); passed++; }
        else { printf("Test 5 (val/object): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("KotlinFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
