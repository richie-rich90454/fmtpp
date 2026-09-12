// Easter egg: C++ formatter tests - because sometimes you need more pluses
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CppFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: C++ function with return type prefix (int main())
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_returnType = "int";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* zero = ASTNodeFactory::createLiteralNode();
        zero->m_type = LiteralNode::TYPE_NUMBER;
        zero->m_value = "0";
        ret->m_argument = zero;
        body->m_statements.push_back(ret);
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "int") == NULL) { printf("  FAIL: missing 'int'\n"); ok = false; }
        if (strstr(result, "main") == NULL) { printf("  FAIL: missing 'main'\n"); ok = false; }
        if (strstr(result, "(") == NULL) { printf("  FAIL: missing '('\n"); ok = false; }
        if (strstr(result, ")") == NULL) { printf("  FAIL: missing ')'\n"); ok = false; }
        if (strstr(result, "return 0;") == NULL) { printf("  FAIL: missing 'return 0;'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function' keyword\n"); ok = false; }
        if (ok) { printf("Test 1 (function decl): PASSED\n"); passed++; }
        else { printf("Test 1 (function decl): FAILED\n"); failed++; }
    }

    // Test 2: C++ class with public inheritance
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* superCls = ASTNodeFactory::createIdentifierNode();
        superCls->m_name = "Animal";
        cls->m_superClass = superCls;
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class") == NULL) { printf("  FAIL: missing 'class'\n"); ok = false; }
        if (strstr(result, "Dog") == NULL) { printf("  FAIL: missing 'Dog'\n"); ok = false; }
        if (strstr(result, ": public") == NULL) { printf("  FAIL: missing ': public'\n"); ok = false; }
        if (strstr(result, "Animal") == NULL) { printf("  FAIL: missing 'Animal'\n"); ok = false; }
        if (ok) { printf("Test 2 (class inheritance): PASSED\n"); passed++; }
        else { printf("Test 2 (class inheritance): FAILED\n"); failed++; }
    }

    // Test 3: C++ namespace declaration
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "std";
        BlockNode* nsBody = ASTNodeFactory::createBlockNode();
        ns->m_body = nsBody;
        prog->m_statements.push_back(ns);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "namespace") == NULL) { printf("  FAIL: missing 'namespace'\n"); ok = false; }
        if (strstr(result, "std") == NULL) { printf("  FAIL: missing 'std'\n"); ok = false; }
        if (ok) { printf("Test 3 (namespace): PASSED\n"); passed++; }
        else { printf("Test 3 (namespace): FAILED\n"); failed++; }
    }

    // Test 4: C++ function declaration without body (forward declaration)
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

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
        if (ok) { printf("Test 4 (forward decl): PASSED\n"); passed++; }
        else { printf("Test 4 (forward decl): FAILED\n"); failed++; }
    }

    // Test 5: C++ #include passthrough
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* pp = ASTNodeFactory::createPreprocessorDirectiveNode();
        pp->m_directive = "#include <iostream>";
        prog->m_statements.push_back(pp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "#include <iostream>") == NULL) { printf("  FAIL: missing '#include <iostream>'\n"); ok = false; }
        if (ok) { printf("Test 5 (include): PASSED\n"); passed++; }
        else { printf("Test 5 (include): FAILED\n"); failed++; }
    }

    // Test 6: C++ class implementing interface (comma-separated bases)
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Widget";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "Drawable";
        cls->m_implements.push_back(base);
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class Widget") == NULL) { printf("  FAIL: missing 'class Widget'\n"); ok = false; }
        if (strstr(result, "Drawable") == NULL) { printf("  FAIL: missing 'Drawable'\n"); ok = false; }
        if (ok) { printf("Test 6 (class implements): PASSED\n"); passed++; }
        else { printf("Test 6 (class implements): FAILED\n"); failed++; }
    }

    // Test 7: C++ member variable with type
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "std::string";
        varDecl->m_name = "name";
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 7 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "std::string") == NULL) { printf("  FAIL: missing 'std::string'\n"); ok = false; }
        if (strstr(result, "name") == NULL) { printf("  FAIL: missing 'name'\n"); ok = false; }
        if (ok) { printf("Test 7 (member var): PASSED\n"); passed++; }
        else { printf("Test 7 (member var): FAILED\n"); failed++; }
    }

    // Test 8: C++ struct keeps its kind (not rewritten to class)
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Point";
        cls->m_kind = "struct";
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 8 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "struct Point") == NULL) { printf("  FAIL: missing 'struct Point'\n"); ok = false; }
        if (strstr(result, "class Point") != NULL) { printf("  FAIL: struct rewritten to class\n"); ok = false; }
        if (ok) { printf("Test 8 (struct kind): PASSED\n"); passed++; }
        else { printf("Test 8 (struct kind): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("CppFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
