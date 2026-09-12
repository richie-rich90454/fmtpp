// Easter egg: Dart formatter tests - triple twenty first throw
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/DartFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
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

    // Test 1: C-style main function
    {
        OutputBuffer out(4096);
        Config config;
        DartFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "void main()") == NULL) { printf("  FAIL: missing 'void main()'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function'\n"); ok = false; }
        if (ok) { printf("Test 1 (main): PASSED\n"); passed++; }
        else { printf("Test 1 (main): FAILED\n"); failed++; }
    }

    // Test 2: class with extends and implements
    {
        OutputBuffer out(4096);
        Config config;
        DartFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
        sup->m_name = "Animal";
        cls->m_superClass = sup;
        IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
        iface->m_name = "Pet";
        cls->m_implements.push_back(iface);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class Dog extends Animal implements Pet") == NULL)
        {
            printf("  FAIL: missing 'class Dog extends Animal implements Pet'\n");
            ok = false;
        }
        if (strstr(result, "public") != NULL) { printf("  FAIL: unexpected 'public'\n"); ok = false; }
        if (ok) { printf("Test 2 (class): PASSED\n"); passed++; }
        else { printf("Test 2 (class): FAILED\n"); failed++; }
    }

    // Test 3: quoted import with semicolon
    {
        OutputBuffer out(4096);
        Config config;
        DartFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "'dart:math'";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "import 'dart:math';") == NULL) { printf("  FAIL: missing \"import 'dart:math';\"\n"); ok = false; }
        if (ok) { printf("Test 3 (import): PASSED\n"); passed++; }
        else { printf("Test 3 (import): FAILED\n"); failed++; }
    }

    // Test 4: method with params and return
    {
        OutputBuffer out(4096);
        Config config;
        DartFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "area";
        func->m_returnType = "double";
        IdentifierNode* param = ASTNodeFactory::createIdentifierNode();
        param->m_name = "double radius";
        func->m_params.push_back(param);
        BlockNode* body = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* val = ASTNodeFactory::createLiteralNode();
        val->m_type = LiteralNode::TYPE_NUMBER;
        val->m_value = "0.0";
        ret->m_argument = val;
        body->m_statements.push_back(ret);
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "double area(double radius)") == NULL) { printf("  FAIL: missing signature\n"); ok = false; }
        if (strstr(result, "return 0.0;") == NULL) { printf("  FAIL: missing 'return 0.0;'\n"); ok = false; }
        if (ok) { printf("Test 4 (method): PASSED\n"); passed++; }
        else { printf("Test 4 (method): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("DartFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
