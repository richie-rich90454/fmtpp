// Easter egg: Objective-C formatter tests - messages sent, tests received
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/ObjectiveCFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: @interface with superclass and protocol, ends with @end
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        cls->m_kind = "@interface";
        IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
        sup->m_name = "NSObject";
        cls->m_superClass = sup;
        IdentifierNode* proto = ASTNodeFactory::createIdentifierNode();
        proto->m_name = "Pet";
        cls->m_implements.push_back(proto);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "@interface Dog : NSObject <Pet>") == NULL)
        {
            printf("  FAIL: missing '@interface Dog : NSObject <Pet>'\n");
            ok = false;
        }
        if (strstr(result, "@end") == NULL) { printf("  FAIL: missing '@end'\n"); ok = false; }
        if (ok) { printf("Test 1 (@interface): PASSED\n"); passed++; }
        else { printf("Test 1 (@interface): FAILED\n"); failed++; }
    }

    // Test 2: method keeps its raw signature
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "- (void)bark:(int)volume";
        func->m_body = NULL;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "- (void)bark:(int)volume;") == NULL)
        {
            printf("  FAIL: missing method prototype\n");
            ok = false;
        }
        if (ok) { printf("Test 2 (method): PASSED\n"); passed++; }
        else { printf("Test 2 (method): FAILED\n"); failed++; }
    }

    // Test 3: @protocol with @end
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "Pet";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "NSObject";
        iface->m_extends.push_back(base);
        iface->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "@protocol Pet <NSObject>") == NULL)
        {
            printf("  FAIL: missing '@protocol Pet <NSObject>'\n");
            ok = false;
        }
        if (strstr(result, "@end") == NULL) { printf("  FAIL: missing '@end'\n"); ok = false; }
        if (ok) { printf("Test 3 (@protocol): PASSED\n"); passed++; }
        else { printf("Test 3 (@protocol): FAILED\n"); failed++; }
    }

    // Test 4: plain C function still works
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "helper";
        func->m_returnType = "void";
        func->m_body = NULL;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "void helper();") == NULL) { printf("  FAIL: missing 'void helper();'\n"); ok = false; }
        if (ok) { printf("Test 4 (C function): PASSED\n"); passed++; }
        else { printf("Test 4 (C function): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("ObjectiveCFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
