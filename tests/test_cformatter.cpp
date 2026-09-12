// Easter egg: C formatter tests - the language that started it all, still going strong
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: C variable declaration (int x = 42;)
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "int";
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
        if (strstr(result, "int") == NULL) { printf("  FAIL: missing 'int'\n"); ok = false; }
        if (strstr(result, "x") == NULL) { printf("  FAIL: missing 'x'\n"); ok = false; }
        if (strstr(result, "42") == NULL) { printf("  FAIL: missing '42'\n"); ok = false; }
        if (strstr(result, ";") == NULL) { printf("  FAIL: missing ';'\n"); ok = false; }
        if (ok) { printf("Test 1 (var decl): PASSED\n"); passed++; }
        else { printf("Test 1 (var decl): FAILED\n"); failed++; }
    }

    // Test 2: C pointer declaration (char *msg;)
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "char *";
        varDecl->m_name = "msg";
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "char *") == NULL) { printf("  FAIL: missing 'char *'\n"); ok = false; }
        if (strstr(result, "msg") == NULL) { printf("  FAIL: missing 'msg'\n"); ok = false; }
        if (ok) { printf("Test 2 (pointer decl): PASSED\n"); passed++; }
        else { printf("Test 2 (pointer decl): FAILED\n"); failed++; }
    }

    // Test 3: Preprocessor directive passthrough (#include)
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* pp = ASTNodeFactory::createPreprocessorDirectiveNode();
        pp->m_directive = "#include <stdio.h>";
        prog->m_statements.push_back(pp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "#include <stdio.h>") == NULL) { printf("  FAIL: missing '#include <stdio.h>'\n"); ok = false; }
        if (ok) { printf("Test 3 (preprocessor): PASSED\n"); passed++; }
        else { printf("Test 3 (preprocessor): FAILED\n"); failed++; }
    }

    // Test 4: #define directive passthrough
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* pp = ASTNodeFactory::createPreprocessorDirectiveNode();
        pp->m_directive = "#define MAX 100";
        prog->m_statements.push_back(pp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "#define MAX 100") == NULL) { printf("  FAIL: missing '#define MAX 100'\n"); ok = false; }
        if (ok) { printf("Test 4 (define): PASSED\n"); passed++; }
        else { printf("Test 4 (define): FAILED\n"); failed++; }
    }

    // Test 5: C if statement with parenthesized condition
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "x > 0";
        ifStmt->m_condition = cond;
        BlockNode* thenBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* one = ASTNodeFactory::createLiteralNode();
        one->m_type = LiteralNode::TYPE_NUMBER;
        one->m_value = "1";
        ret->m_argument = one;
        thenBody->m_statements.push_back(ret);
        ifStmt->m_thenBranch = thenBody;
        prog->m_statements.push_back(ifStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "if") == NULL) { printf("  FAIL: missing 'if'\n"); ok = false; }
        if (strstr(result, "(x > 0)") == NULL) { printf("  FAIL: missing '(x > 0)'\n"); ok = false; }
        if (strstr(result, "return 1;") == NULL) { printf("  FAIL: missing 'return 1;'\n"); ok = false; }
        if (ok) { printf("Test 5 (if stmt): PASSED\n"); passed++; }
        else { printf("Test 5 (if stmt): FAILED\n"); failed++; }
    }

    // Test 6: C for loop with semicolon-separated header
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "int";
        init->m_name = "i";
        LiteralNode* initVal = ASTNodeFactory::createLiteralNode();
        initVal->m_type = LiteralNode::TYPE_NUMBER;
        initVal->m_value = "0";
        init->m_initializer = initVal;
        forStmt->m_init = init;
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "i < 10";
        forStmt->m_condition = cond;
        IdentifierNode* update = ASTNodeFactory::createIdentifierNode();
        update->m_name = "i++";
        forStmt->m_update = update;
        BlockNode* body = ASTNodeFactory::createBlockNode();
        forStmt->m_body = body;
        prog->m_statements.push_back(forStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "for") == NULL) { printf("  FAIL: missing 'for'\n"); ok = false; }
        if (strstr(result, "int i = 0") == NULL) { printf("  FAIL: missing 'int i = 0'\n"); ok = false; }
        if (strstr(result, "i < 10") == NULL) { printf("  FAIL: missing 'i < 10'\n"); ok = false; }
        if (strstr(result, "i++") == NULL) { printf("  FAIL: missing 'i++'\n"); ok = false; }
        if (ok) { printf("Test 6 (for loop): PASSED\n"); passed++; }
        else { printf("Test 6 (for loop): FAILED\n"); failed++; }
    }

    // Test 7: C while loop
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        WhileStmtNode* whileStmt = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "running";
        whileStmt->m_condition = cond;
        BlockNode* body = ASTNodeFactory::createBlockNode();
        whileStmt->m_body = body;
        prog->m_statements.push_back(whileStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 7 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "while") == NULL) { printf("  FAIL: missing 'while'\n"); ok = false; }
        if (strstr(result, "(running)") == NULL) { printf("  FAIL: missing '(running)'\n"); ok = false; }
        if (ok) { printf("Test 7 (while loop): PASSED\n"); passed++; }
        else { printf("Test 7 (while loop): FAILED\n"); failed++; }
    }

    // Test 8: C comment preserved
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        CommentNode* cmt = ASTNodeFactory::createCommentNode();
        cmt->m_text = " entry point";
        cmt->m_isBlock = false;
        prog->m_statements.push_back(cmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 8 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "//") == NULL) { printf("  FAIL: missing '//'\n"); ok = false; }
        if (strstr(result, "entry point") == NULL) { printf("  FAIL: missing comment text\n"); ok = false; }
        if (ok) { printf("Test 8 (comment): PASSED\n"); passed++; }
        else { printf("Test 8 (comment): FAILED\n"); failed++; }
    }

    // Test 9: C function uses return-type prefix, not the function keyword
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "add";
        func->m_returnType = "int";
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "int a";
        func->m_params.push_back(paramA);
        IdentifierNode* paramB = ASTNodeFactory::createIdentifierNode();
        paramB->m_name = "int b";
        func->m_params.push_back(paramB);
        BlockNode* body = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        IdentifierNode* retExpr = ASTNodeFactory::createIdentifierNode();
        retExpr->m_name = "a";
        ret->m_argument = retExpr;
        body->m_statements.push_back(ret);
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 9 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "int add(int a, int b)") == NULL) { printf("  FAIL: missing 'int add(int a, int b)'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function' keyword\n"); ok = false; }
        if (ok) { printf("Test 9 (function decl): PASSED\n"); passed++; }
        else { printf("Test 9 (function decl): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("CFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
