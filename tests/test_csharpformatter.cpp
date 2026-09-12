// Easter egg: C# formatter tests - Microsoft's answer to Java, with extra sharpness
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CSharpFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
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

    // Test 1: C# class with base class + interface
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* superCls = ASTNodeFactory::createIdentifierNode();
        superCls->m_name = "Animal";
        cls->m_superClass = superCls;
        IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
        iface->m_name = "IDisposable";
        cls->m_implements.push_back(iface);
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public") == NULL) { printf("  FAIL: missing 'public'\n"); ok = false; }
        if (strstr(result, "class") == NULL) { printf("  FAIL: missing 'class'\n"); ok = false; }
        if (strstr(result, "Dog") == NULL) { printf("  FAIL: missing 'Dog'\n"); ok = false; }
        if (strstr(result, ":") == NULL) { printf("  FAIL: missing ':'\n"); ok = false; }
        if (strstr(result, "Animal") == NULL) { printf("  FAIL: missing 'Animal'\n"); ok = false; }
        if (strstr(result, "IDisposable") == NULL) { printf("  FAIL: missing 'IDisposable'\n"); ok = false; }
        if (ok) { printf("Test 1 (class bases): PASSED\n"); passed++; }
        else { printf("Test 1 (class bases): FAILED\n"); failed++; }
    }

    // Test 2: C# method declaration (public ReturnType Name)
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Main";
        func->m_returnType = "void";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public void Main(") == NULL) { printf("  FAIL: missing 'public void Main('\n"); ok = false; }
        if (ok) { printf("Test 2 (method decl): PASSED\n"); passed++; }
        else { printf("Test 2 (method decl): FAILED\n"); failed++; }
    }

    // Test 3: C# namespace declaration
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "MyApp";
        BlockNode* nsBody = ASTNodeFactory::createBlockNode();
        ns->m_body = nsBody;
        prog->m_statements.push_back(ns);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "namespace") == NULL) { printf("  FAIL: missing 'namespace'\n"); ok = false; }
        if (strstr(result, "MyApp") == NULL) { printf("  FAIL: missing 'MyApp'\n"); ok = false; }
        if (ok) { printf("Test 3 (namespace): PASSED\n"); passed++; }
        else { printf("Test 3 (namespace): FAILED\n"); failed++; }
    }

    // Test 4: C# interface with extended interface
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "IList";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "ICollection";
        iface->m_extends.push_back(base);
        BlockNode* ifaceBody = ASTNodeFactory::createBlockNode();
        iface->m_body = ifaceBody;
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public interface IList") == NULL) { printf("  FAIL: missing 'public interface IList'\n"); ok = false; }
        if (strstr(result, "ICollection") == NULL) { printf("  FAIL: missing 'ICollection'\n"); ok = false; }
        if (ok) { printf("Test 4 (interface): PASSED\n"); passed++; }
        else { printf("Test 4 (interface): FAILED\n"); failed++; }
    }

    // Test 5: C# method with params and return
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Add";
        func->m_returnType = "int";
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "int a";
        func->m_params.push_back(paramA);
        IdentifierNode* paramB = ASTNodeFactory::createIdentifierNode();
        paramB->m_name = "int b";
        func->m_params.push_back(paramB);
        BlockNode* funcBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        IdentifierNode* retExpr = ASTNodeFactory::createIdentifierNode();
        retExpr->m_name = "a";
        ret->m_argument = retExpr;
        funcBody->m_statements.push_back(ret);
        func->m_body = funcBody;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public int Add(") == NULL) { printf("  FAIL: missing 'public int Add('\n"); ok = false; }
        if (strstr(result, "int a") == NULL) { printf("  FAIL: missing 'int a'\n"); ok = false; }
        if (strstr(result, "int b") == NULL) { printf("  FAIL: missing 'int b'\n"); ok = false; }
        if (strstr(result, "return") == NULL) { printf("  FAIL: missing 'return'\n"); ok = false; }
        if (ok) { printf("Test 5 (method with params): PASSED\n"); passed++; }
        else { printf("Test 5 (method with params): FAILED\n"); failed++; }
    }

    // Test 6: C# method prototype without body
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Dispose";
        func->m_returnType = "void";
        func->m_body = NULL;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public void Dispose();") == NULL) { printf("  FAIL: missing 'public void Dispose();'\n"); ok = false; }
        if (ok) { printf("Test 6 (method prototype): PASSED\n"); passed++; }
        else { printf("Test 6 (method prototype): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("CSharpFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
