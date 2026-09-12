// Easter egg: Java formatter tests - everything is an object, except the primitives
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/JavaFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: Java method declaration (public + return type)
    {
        OutputBuffer out(4096);
        Config config;
        JavaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_returnType = "void";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public") == NULL) { printf("  FAIL: missing 'public'\n"); ok = false; }
        if (strstr(result, "void") == NULL) { printf("  FAIL: missing 'void'\n"); ok = false; }
        if (strstr(result, "main") == NULL) { printf("  FAIL: missing 'main'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function' keyword\n"); ok = false; }
        if (ok) { printf("Test 1 (method decl): PASSED\n"); passed++; }
        else { printf("Test 1 (method decl): FAILED\n"); failed++; }
    }

    // Test 2: Java class with extends + implements
    {
        OutputBuffer out(4096);
        Config config;
        JavaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* superCls = ASTNodeFactory::createIdentifierNode();
        superCls->m_name = "Animal";
        cls->m_superClass = superCls;
        IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
        iface->m_name = "Pet";
        cls->m_implements.push_back(iface);
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public") == NULL) { printf("  FAIL: missing 'public'\n"); ok = false; }
        if (strstr(result, "class") == NULL) { printf("  FAIL: missing 'class'\n"); ok = false; }
        if (strstr(result, "Dog") == NULL) { printf("  FAIL: missing 'Dog'\n"); ok = false; }
        if (strstr(result, "extends") == NULL) { printf("  FAIL: missing 'extends'\n"); ok = false; }
        if (strstr(result, "Animal") == NULL) { printf("  FAIL: missing 'Animal'\n"); ok = false; }
        if (strstr(result, "implements") == NULL) { printf("  FAIL: missing 'implements'\n"); ok = false; }
        if (strstr(result, "Pet") == NULL) { printf("  FAIL: missing 'Pet'\n"); ok = false; }
        if (ok) { printf("Test 2 (class extends/implements): PASSED\n"); passed++; }
        else { printf("Test 2 (class extends/implements): FAILED\n"); failed++; }
    }

    // Test 3: Java import statement
    {
        OutputBuffer out(4096);
        Config config;
        JavaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "java.util.List";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "import") == NULL) { printf("  FAIL: missing 'import'\n"); ok = false; }
        if (strstr(result, "java.util.List") == NULL) { printf("  FAIL: missing 'java.util.List'\n"); ok = false; }
        if (strstr(result, ";") == NULL) { printf("  FAIL: missing ';'\n"); ok = false; }
        if (ok) { printf("Test 3 (import decl): PASSED\n"); passed++; }
        else { printf("Test 3 (import decl): FAILED\n"); failed++; }
    }

    // Test 4: Java switch with break after each case
    {
        OutputBuffer out(4096);
        Config config;
        JavaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "day";
        sw->m_discriminant = disc;
        IdentifierNode* caseNode = ASTNodeFactory::createIdentifierNode();
        caseNode->m_name = "case 1: one()";
        sw->m_cases.push_back(caseNode);
        prog->m_statements.push_back(sw);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "switch") == NULL) { printf("  FAIL: missing 'switch'\n"); ok = false; }
        if (strstr(result, "day") == NULL) { printf("  FAIL: missing 'day'\n"); ok = false; }
        if (strstr(result, "break;") == NULL) { printf("  FAIL: missing 'break;'\n"); ok = false; }
        if (ok) { printf("Test 4 (switch/break): PASSED\n"); passed++; }
        else { printf("Test 4 (switch/break): FAILED\n"); failed++; }
    }

    // Test 5: Java try/catch/finally
    {
        OutputBuffer out(4096);
        Config config;
        JavaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        TryCatchStmtNode* tc = ASTNodeFactory::createTryCatchStmtNode();
        BlockNode* tryBody = ASTNodeFactory::createBlockNode();
        tc->m_body = tryBody;
        IdentifierNode* catchClause = ASTNodeFactory::createIdentifierNode();
        catchClause->m_name = "Exception e";
        tc->m_catchClause = catchClause;
        BlockNode* finallyBody = ASTNodeFactory::createBlockNode();
        tc->m_finallyBlock = finallyBody;
        prog->m_statements.push_back(tc);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "try") == NULL) { printf("  FAIL: missing 'try'\n"); ok = false; }
        if (strstr(result, "catch") == NULL) { printf("  FAIL: missing 'catch'\n"); ok = false; }
        if (strstr(result, "Exception e") == NULL) { printf("  FAIL: missing 'Exception e'\n"); ok = false; }
        if (strstr(result, "finally") == NULL) { printf("  FAIL: missing 'finally'\n"); ok = false; }
        if (ok) { printf("Test 5 (try/catch/finally): PASSED\n"); passed++; }
        else { printf("Test 5 (try/catch/finally): FAILED\n"); failed++; }
    }

    // Test 6: Java typed variable declaration
    {
        OutputBuffer out(4096);
        Config config;
        JavaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "int";
        varDecl->m_name = "count";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "0";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "int") == NULL) { printf("  FAIL: missing 'int'\n"); ok = false; }
        if (strstr(result, "count") == NULL) { printf("  FAIL: missing 'count'\n"); ok = false; }
        if (strstr(result, "0") == NULL) { printf("  FAIL: missing '0'\n"); ok = false; }
        if (strstr(result, ";") == NULL) { printf("  FAIL: missing ';'\n"); ok = false; }
        if (ok) { printf("Test 6 (typed var decl): PASSED\n"); passed++; }
        else { printf("Test 6 (typed var decl): FAILED\n"); failed++; }
    }

    // Test 7: Java method with params and return statement
    {
        OutputBuffer out(4096);
        Config config;
        JavaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "add";
        func->m_returnType = "int";
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "int a";
        func->m_params.push_back(paramA);
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
        printf("Test 7 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public int add") == NULL) { printf("  FAIL: missing 'public int add'\n"); ok = false; }
        if (strstr(result, "int a") == NULL) { printf("  FAIL: missing 'int a'\n"); ok = false; }
        if (strstr(result, "return") == NULL) { printf("  FAIL: missing 'return'\n"); ok = false; }
        if (ok) { printf("Test 7 (method with params): PASSED\n"); passed++; }
        else { printf("Test 7 (method with params): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("JavaFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
