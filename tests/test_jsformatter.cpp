#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/JSFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"

int main()
{
    // 1. Create MemoryPool and set g_pool
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // ============================================================
    // Test 1: Variable declaration (let x = 42)
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "x";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "42";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "let") == NULL) { printf("  FAIL: missing 'let'\n"); ok = false; }
        if (strstr(result, "x") == NULL) { printf("  FAIL: missing 'x'\n"); ok = false; }
        if (strstr(result, "42") == NULL) { printf("  FAIL: missing '42'\n"); ok = false; }
        if (strstr(result, "=") == NULL) { printf("  FAIL: missing '='\n"); ok = false; }
        if (ok) { printf("Test 1 (variable decl): PASSED\n"); passed++; }
        else { printf("Test 1 (variable decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 2: Function declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        BlockNode* funcBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* zero = ASTNodeFactory::createLiteralNode();
        zero->m_type = LiteralNode::TYPE_NUMBER;
        zero->m_value = "0";
        ret->m_argument = zero;
        funcBody->m_statements.push_back(ret);
        func->m_body = funcBody;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "function") == NULL) { printf("  FAIL: missing 'function'\n"); ok = false; }
        if (strstr(result, "main") == NULL) { printf("  FAIL: missing 'main'\n"); ok = false; }
        if (strstr(result, "(") == NULL) { printf("  FAIL: missing '('\n"); ok = false; }
        if (strstr(result, ")") == NULL) { printf("  FAIL: missing ')'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (strstr(result, "}") == NULL) { printf("  FAIL: missing '}'\n"); ok = false; }
        if (ok) { printf("Test 2 (function decl): PASSED\n"); passed++; }
        else { printf("Test 2 (function decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 3: Import declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "react";
        IdentifierNode* spec = ASTNodeFactory::createIdentifierNode();
        spec->m_name = "useState";
        imp->m_specifiers.push_back(spec);
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "import") == NULL) { printf("  FAIL: missing 'import'\n"); ok = false; }
        if (strstr(result, "useState") == NULL) { printf("  FAIL: missing 'useState'\n"); ok = false; }
        if (strstr(result, "react") == NULL) { printf("  FAIL: missing 'react'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (strstr(result, "}") == NULL) { printf("  FAIL: missing '}'\n"); ok = false; }
        if (ok) { printf("Test 3 (import decl): PASSED\n"); passed++; }
        else { printf("Test 3 (import decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 4: Class declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Foo";
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class") == NULL) { printf("  FAIL: missing 'class'\n"); ok = false; }
        if (strstr(result, "Foo") == NULL) { printf("  FAIL: missing 'Foo'\n"); ok = false; }
        if (ok) { printf("Test 4 (class decl): PASSED\n"); passed++; }
        else { printf("Test 4 (class decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 5: Arrow function
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "const";
        varDecl->m_name = "add";
        ArrowFunctionNode* arrow = ASTNodeFactory::createArrowFunctionNode();
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "a";
        arrow->m_params.push_back(paramA);
        IdentifierNode* paramB = ASTNodeFactory::createIdentifierNode();
        paramB->m_name = "b";
        arrow->m_params.push_back(paramB);
        BlockNode* arrowBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        IdentifierNode* retExpr = ASTNodeFactory::createIdentifierNode();
        retExpr->m_name = "a";
        ret->m_argument = retExpr;
        arrowBody->m_statements.push_back(ret);
        arrow->m_body = arrowBody;
        varDecl->m_initializer = arrow;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "const") == NULL) { printf("  FAIL: missing 'const'\n"); ok = false; }
        if (strstr(result, "add") == NULL) { printf("  FAIL: missing 'add'\n"); ok = false; }
        if (strstr(result, "=>") == NULL) { printf("  FAIL: missing '=>'\n"); ok = false; }
        if (ok) { printf("Test 5 (arrow function): PASSED\n"); passed++; }
        else { printf("Test 5 (arrow function): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 6: Array literal
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "const";
        varDecl->m_name = "arr";
        ArrayLiteralNode* arr = ASTNodeFactory::createArrayLiteralNode();
        LiteralNode* elem1 = ASTNodeFactory::createLiteralNode();
        elem1->m_type = LiteralNode::TYPE_NUMBER;
        elem1->m_value = "1";
        arr->m_elements.push_back(elem1);
        LiteralNode* elem2 = ASTNodeFactory::createLiteralNode();
        elem2->m_type = LiteralNode::TYPE_NUMBER;
        elem2->m_value = "2";
        arr->m_elements.push_back(elem2);
        LiteralNode* elem3 = ASTNodeFactory::createLiteralNode();
        elem3->m_type = LiteralNode::TYPE_NUMBER;
        elem3->m_value = "3";
        arr->m_elements.push_back(elem3);
        varDecl->m_initializer = arr;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "const") == NULL) { printf("  FAIL: missing 'const'\n"); ok = false; }
        if (strstr(result, "arr") == NULL) { printf("  FAIL: missing 'arr'\n"); ok = false; }
        if (strstr(result, "[") == NULL) { printf("  FAIL: missing '['\n"); ok = false; }
        if (strstr(result, "]") == NULL) { printf("  FAIL: missing ']'\n"); ok = false; }
        if (strstr(result, "1") == NULL) { printf("  FAIL: missing '1'\n"); ok = false; }
        if (strstr(result, "2") == NULL) { printf("  FAIL: missing '2'\n"); ok = false; }
        if (strstr(result, "3") == NULL) { printf("  FAIL: missing '3'\n"); ok = false; }
        if (ok) { printf("Test 6 (array literal): PASSED\n"); passed++; }
        else { printf("Test 6 (array literal): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 7: Object literal
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "const";
        varDecl->m_name = "obj";
        ObjectLiteralNode* obj = ASTNodeFactory::createObjectLiteralNode();
        PropertyNode* prop = ASTNodeFactory::createPropertyNode();
        IdentifierNode* key = ASTNodeFactory::createIdentifierNode();
        key->m_name = "key";
        prop->m_key = key;
        LiteralNode* val = ASTNodeFactory::createLiteralNode();
        val->m_type = LiteralNode::TYPE_STRING;
        val->m_value = "value";
        prop->m_value = val;
        obj->m_properties.push_back(prop);
        varDecl->m_initializer = obj;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 7 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "const") == NULL) { printf("  FAIL: missing 'const'\n"); ok = false; }
        if (strstr(result, "obj") == NULL) { printf("  FAIL: missing 'obj'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (strstr(result, "}") == NULL) { printf("  FAIL: missing '}'\n"); ok = false; }
        if (strstr(result, "key") == NULL) { printf("  FAIL: missing 'key'\n"); ok = false; }
        if (strstr(result, "value") == NULL) { printf("  FAIL: missing 'value'\n"); ok = false; }
        if (ok) { printf("Test 7 (object literal): PASSED\n"); passed++; }
        else { printf("Test 7 (object literal): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 8: Export declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        JSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ExportDeclNode* exp = ASTNodeFactory::createExportDeclNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "hello";
        BlockNode* funcBody = ASTNodeFactory::createBlockNode();
        func->m_body = funcBody;
        exp->m_declaration = func;
        prog->m_statements.push_back(exp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 8 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "export") == NULL) { printf("  FAIL: missing 'export'\n"); ok = false; }
        if (strstr(result, "function") == NULL) { printf("  FAIL: missing 'function'\n"); ok = false; }
        if (strstr(result, "hello") == NULL) { printf("  FAIL: missing 'hello'\n"); ok = false; }
        if (ok) { printf("Test 8 (export decl): PASSED\n"); passed++; }
        else { printf("Test 8 (export decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Summary
    // ============================================================
    printf("\n========================================\n");
    printf("JSFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}