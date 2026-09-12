// Easter egg: Swift formatter tests - optional semicolons, non-optional safety
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/SwiftFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
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

    // Test 1: func with arrow return, no semicolons
    {
        OutputBuffer out(4096);
        Config config;
        SwiftFormatter formatter(out, config);

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
        if (strstr(result, "func add(a: Int) -> Int") == NULL) { printf("  FAIL: missing 'func add(a: Int) -> Int'\n"); ok = false; }
        if (strstr(result, ";") != NULL) { printf("  FAIL: unexpected semicolon\n"); ok = false; }
        if (ok) { printf("Test 1 (func decl): PASSED\n"); passed++; }
        else { printf("Test 1 (func decl): FAILED\n"); failed++; }
    }

    // Test 2: struct with bases
    {
        OutputBuffer out(4096);
        Config config;
        SwiftFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Point";
        cls->m_kind = "struct";
        IdentifierNode* proto = ASTNodeFactory::createIdentifierNode();
        proto->m_name = "Equatable";
        cls->m_implements.push_back(proto);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "struct Point: Equatable") == NULL) { printf("  FAIL: missing 'struct Point: Equatable'\n"); ok = false; }
        if (ok) { printf("Test 2 (struct): PASSED\n"); passed++; }
        else { printf("Test 2 (struct): FAILED\n"); failed++; }
    }

    // Test 3: import, let/var without semicolons
    {
        OutputBuffer out(4096);
        Config config;
        SwiftFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "Foundation";
        prog->m_statements.push_back(imp);
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "pi";
        varDecl->m_type = "Double";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "3.14";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "import Foundation") == NULL) { printf("  FAIL: missing import\n"); ok = false; }
        if (strstr(result, "let pi: Double = 3.14") == NULL) { printf("  FAIL: missing 'let pi: Double = 3.14'\n"); ok = false; }
        if (strstr(result, ";") != NULL) { printf("  FAIL: unexpected semicolon\n"); ok = false; }
        if (ok) { printf("Test 3 (import/let): PASSED\n"); passed++; }
        else { printf("Test 3 (import/let): FAILED\n"); failed++; }
    }

    // Test 4: for-in without parentheses
    {
        OutputBuffer out(4096);
        Config config;
        SwiftFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = false;
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "x in 0..<10";
        forStmt->m_condition = cond;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "for x in 0..<10") == NULL) { printf("  FAIL: missing 'for x in 0..<10'\n"); ok = false; }
        if (strstr(result, "(") != NULL) { printf("  FAIL: unexpected parentheses\n"); ok = false; }
        if (ok) { printf("Test 4 (for-in): PASSED\n"); passed++; }
        else { printf("Test 4 (for-in): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("SwiftFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
