// Easter egg: PHP formatter tests - echoing beautiful code since 1995
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/PhpFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
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

    // Test 1: function with visibility modifier
    {
        OutputBuffer out(4096);
        Config config;
        PhpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "bark";
        func->m_modifiers = "public";
        IdentifierNode* param = ASTNodeFactory::createIdentifierNode();
        param->m_name = "$volume";
        func->m_params.push_back(param);
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public function bark($volume)") == NULL)
        {
            printf("  FAIL: missing 'public function bark($volume)'\n");
            ok = false;
        }
        if (ok) { printf("Test 1 (method): PASSED\n"); passed++; }
        else { printf("Test 1 (method): FAILED\n"); failed++; }
    }

    // Test 2: $ variables keep their sigil exactly once
    {
        OutputBuffer out(4096);
        Config config;
        PhpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_name = "$name";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_STRING;
        lit->m_value = "Rex";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "$name") == NULL) { printf("  FAIL: missing '$name'\n"); ok = false; }
        if (strstr(result, "$$name") != NULL) { printf("  FAIL: doubled sigil\n"); ok = false; }
        if (strstr(result, ";") == NULL) { printf("  FAIL: missing ';'\n"); ok = false; }
        if (ok) { printf("Test 2 (variable): PASSED\n"); passed++; }
        else { printf("Test 2 (variable): FAILED\n"); failed++; }
    }

    // Test 3: class with interface, use, namespace, open tag
    {
        OutputBuffer out(4096);
        Config config;
        PhpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* tag = ASTNodeFactory::createPreprocessorDirectiveNode();
        tag->m_directive = "<?php";
        prog->m_statements.push_back(tag);
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "App\\Models";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "App\\Contracts\\Pet";
        prog->m_statements.push_back(imp);
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
        iface->m_name = "Pet";
        cls->m_implements.push_back(iface);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "<?php") == NULL) { printf("  FAIL: missing '<?php'\n"); ok = false; }
        if (strstr(result, "namespace App\\Models;") == NULL) { printf("  FAIL: missing namespace\n"); ok = false; }
        if (strstr(result, "use App\\Contracts\\Pet;") == NULL) { printf("  FAIL: missing use\n"); ok = false; }
        if (strstr(result, "class Dog implements Pet") == NULL) { printf("  FAIL: missing class line\n"); ok = false; }
        if (ok) { printf("Test 3 (class/use/namespace): PASSED\n"); passed++; }
        else { printf("Test 3 (class/use/namespace): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("PhpFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
