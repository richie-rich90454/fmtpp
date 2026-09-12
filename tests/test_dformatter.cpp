// Easter egg: D formatter tests - unittest { assert(passes); }
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/DFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: class with colon bases, struct kind
    {
        OutputBuffer out(4096);
        Config config;
        DFormatter formatter(out, config);

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
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class Dog : Animal, Pet") == NULL)
        {
            printf("  FAIL: missing 'class Dog : Animal, Pet'\n");
            ok = false;
        }
        if (strstr(result, "public") != NULL) { printf("  FAIL: unexpected 'public'\n"); ok = false; }
        if (ok) { printf("Test 1 (class): PASSED\n"); passed++; }
        else { printf("Test 1 (class): FAILED\n"); failed++; }
    }

    // Test 2: interface with colon bases
    {
        OutputBuffer out(4096);
        Config config;
        DFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "IList";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "ICollection";
        iface->m_extends.push_back(base);
        iface->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "interface IList : ICollection") == NULL)
        {
            printf("  FAIL: missing 'interface IList : ICollection'\n");
            ok = false;
        }
        if (strstr(result, "extends") != NULL) { printf("  FAIL: unexpected 'extends'\n"); ok = false; }
        if (ok) { printf("Test 2 (interface): PASSED\n"); passed++; }
        else { printf("Test 2 (interface): FAILED\n"); failed++; }
    }

    // Test 3: dotted import and module with semicolons
    {
        OutputBuffer out(4096);
        Config config;
        DFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "myapp";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "std.stdio";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "module myapp;") == NULL) { printf("  FAIL: missing 'module myapp;'\n"); ok = false; }
        if (strstr(result, "import std.stdio;") == NULL) { printf("  FAIL: missing 'import std.stdio;'\n"); ok = false; }
        if (ok) { printf("Test 3 (module/import): PASSED\n"); passed++; }
        else { printf("Test 3 (module/import): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("DFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
