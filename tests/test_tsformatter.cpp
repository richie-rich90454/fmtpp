#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/TSFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"

int main()
{
    // 1. Create MemoryPool and set g_pool
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // ============================================================
    // Test 1: Function declaration with return type annotation
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        TSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "add";
        func->m_returnType = "number";
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "a";
        func->m_params.push_back(paramA);
        IdentifierNode* paramB = ASTNodeFactory::createIdentifierNode();
        paramB->m_name = "b";
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
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "function") == NULL) { printf("  FAIL: missing 'function'\n"); ok = false; }
        if (strstr(result, "add") == NULL) { printf("  FAIL: missing 'add'\n"); ok = false; }
        if (strstr(result, "number") == NULL) { printf("  FAIL: missing return type 'number'\n"); ok = false; }
        if (strstr(result, ": ") == NULL) { printf("  FAIL: missing ': '\n"); ok = false; }
        if (strstr(result, "(") == NULL) { printf("  FAIL: missing '('\n"); ok = false; }
        if (strstr(result, ")") == NULL) { printf("  FAIL: missing ')'\n"); ok = false; }
        if (ok) { printf("Test 1 (function with return type): PASSED\n"); passed++; }
        else { printf("Test 1 (function with return type): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 2: Interface declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        TSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "Person";
        BlockNode* ifaceBody = ASTNodeFactory::createBlockNode();
        PropertyNode* prop = ASTNodeFactory::createPropertyNode();
        IdentifierNode* key = ASTNodeFactory::createIdentifierNode();
        key->m_name = "name";
        prop->m_key = key;
        IdentifierNode* val = ASTNodeFactory::createIdentifierNode();
        val->m_name = "string";
        prop->m_value = val;
        ifaceBody->m_statements.push_back(prop);
        iface->m_body = ifaceBody;
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "interface") == NULL) { printf("  FAIL: missing 'interface'\n"); ok = false; }
        if (strstr(result, "Person") == NULL) { printf("  FAIL: missing 'Person'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (strstr(result, "}") == NULL) { printf("  FAIL: missing '}'\n"); ok = false; }
        if (strstr(result, "name") == NULL) { printf("  FAIL: missing 'name'\n"); ok = false; }
        if (strstr(result, "string") == NULL) { printf("  FAIL: missing 'string'\n"); ok = false; }
        if (ok) { printf("Test 2 (interface decl): PASSED\n"); passed++; }
        else { printf("Test 2 (interface decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 3: Enum declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        TSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "Color";
        IdentifierNode* member1 = ASTNodeFactory::createIdentifierNode();
        member1->m_name = "Red";
        enm->m_members.push_back(member1);
        IdentifierNode* member2 = ASTNodeFactory::createIdentifierNode();
        member2->m_name = "Green";
        enm->m_members.push_back(member2);
        IdentifierNode* member3 = ASTNodeFactory::createIdentifierNode();
        member3->m_name = "Blue";
        enm->m_members.push_back(member3);
        prog->m_statements.push_back(enm);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "enum") == NULL) { printf("  FAIL: missing 'enum'\n"); ok = false; }
        if (strstr(result, "Color") == NULL) { printf("  FAIL: missing 'Color'\n"); ok = false; }
        if (strstr(result, "Red") == NULL) { printf("  FAIL: missing 'Red'\n"); ok = false; }
        if (strstr(result, "Green") == NULL) { printf("  FAIL: missing 'Green'\n"); ok = false; }
        if (strstr(result, "Blue") == NULL) { printf("  FAIL: missing 'Blue'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (strstr(result, "}") == NULL) { printf("  FAIL: missing '}'\n"); ok = false; }
        if (ok) { printf("Test 3 (enum decl): PASSED\n"); passed++; }
        else { printf("Test 3 (enum decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 4: Type alias declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        TSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        TypeAliasNode* alias = ASTNodeFactory::createTypeAliasNode();
        alias->m_name = "StringOrNumber";
        IdentifierNode* typeAnn = ASTNodeFactory::createIdentifierNode();
        typeAnn->m_name = "string | number";
        alias->m_typeAnnotation = typeAnn;
        prog->m_statements.push_back(alias);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "type") == NULL) { printf("  FAIL: missing 'type'\n"); ok = false; }
        if (strstr(result, "StringOrNumber") == NULL) { printf("  FAIL: missing 'StringOrNumber'\n"); ok = false; }
        if (strstr(result, " = ") == NULL) { printf("  FAIL: missing ' = '\n"); ok = false; }
        if (strstr(result, "string | number") == NULL) { printf("  FAIL: missing 'string | number'\n"); ok = false; }
        if (ok) { printf("Test 4 (type alias): PASSED\n"); passed++; }
        else { printf("Test 4 (type alias): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 5: Interface with extends
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        TSFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "Employee";
        IdentifierNode* extended = ASTNodeFactory::createIdentifierNode();
        extended->m_name = "Person";
        iface->m_extends.push_back(extended);
        BlockNode* ifaceBody = ASTNodeFactory::createBlockNode();
        iface->m_body = ifaceBody;
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "interface") == NULL) { printf("  FAIL: missing 'interface'\n"); ok = false; }
        if (strstr(result, "Employee") == NULL) { printf("  FAIL: missing 'Employee'\n"); ok = false; }
        if (strstr(result, "extends") == NULL) { printf("  FAIL: missing 'extends'\n"); ok = false; }
        if (strstr(result, "Person") == NULL) { printf("  FAIL: missing 'Person'\n"); ok = false; }
        if (ok) { printf("Test 5 (interface extends): PASSED\n"); passed++; }
        else { printf("Test 5 (interface extends): FAILED\n"); failed++; }
    }

    // ============================================================
    // Summary
    // ============================================================
    printf("\n========================================\n");
    printf("TSFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
