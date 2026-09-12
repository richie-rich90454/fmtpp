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
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"

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

    // Test 10: table-driven var decls (all scalar types/modifiers)
    {
        struct Case { const char* kind; const char* name; const char* init; };
        static const Case kCases[] = {
            {"int", "tv00", "0"}, {"unsigned int", "tv01", "1"},
            {"long", "tv02", "2"}, {"long long", "tv03", "3"},
            {"short", "tv04", "4"}, {"char", "tv05", "5"},
            {"signed char", "tv06", "6"}, {"unsigned char", "tv07", "7"},
            {"float", "tv08", "8"}, {"double", "tv09", "9"},
            {"long double", "tv10", "10"}, {"const int", "tv11", "11"},
            {"static int", "tv12", "12"}, {"volatile int", "tv13", "13"},
            {"register int", "tv14", "14"}, {"extern int", "tv15", "15"},
            {"size_t", "tv16", "16"}, {"int32_t", "tv17", "17"},
            {"uint64_t", "tv18", "18"}, {"bool", "tv19", "19"},
            {"char *", "tv20", "20"}, {"const char *", "tv21", "21"},
            {"int *", "tv22", "22"}, {"unsigned long", "tv23", "23"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            CFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
            varDecl->m_kind = kCases[i].kind;
            varDecl->m_name = kCases[i].name;
            LiteralNode* lit = ASTNodeFactory::createLiteralNode();
            lit->m_type = LiteralNode::TYPE_NUMBER;
            lit->m_value = kCases[i].init;
            varDecl->m_initializer = lit;
            prog->m_statements.push_back(varDecl);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].kind) == NULL) { printf("  FAIL row %u: missing kind\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].init) == NULL) { printf("  FAIL row %u: missing init\n", (unsigned)i); ok = false; }
            if (strstr(result, ";") == NULL) { printf("  FAIL row %u: missing ';'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 10 (var table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 11: table-driven prototypes (modifiers x return types)
    {
        struct Case { const char* mods; const char* ret; const char* name; };
        static const Case kCases[] = {
            {"", "int", "cfn00"}, {"static", "int", "cfn01"},
            {"inline", "int", "cfn02"}, {"extern", "void", "cfn03"},
            {"static", "void", "cfn04"}, {"", "char *", "cfn05"},
            {"", "unsigned long", "cfn06"}, {"static", "double", "cfn07"},
            {"", "size_t", "cfn08"}, {"inline", "void", "cfn09"},
            {"extern", "int", "cfn10"}, {"", "const char *", "cfn11"},
            {"static", "long", "cfn12"}, {"", "float", "cfn13"},
            {"inline", "short", "cfn14"}, {"", "bool", "cfn15"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            CFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            func->m_name = kCases[i].name;
            func->m_modifiers = kCases[i].mods;
            func->m_returnType = kCases[i].ret;
            func->m_body = NULL;
            prog->m_statements.push_back(func);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].ret) == NULL) { printf("  FAIL row %u: missing ret\n", (unsigned)i); ok = false; }
            if (strstr(result, "(") == NULL) { printf("  FAIL row %u: missing '('\n", (unsigned)i); ok = false; }
            if (strstr(result, ")") == NULL) { printf("  FAIL row %u: missing ')'\n", (unsigned)i); ok = false; }
            if (strstr(result, ";") == NULL) { printf("  FAIL row %u: missing ';'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 11 (prototype table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 12: C function body uses same-line brace ") {"
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "samebrace";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        formatter.format(prog);
        const char* result = out.getData();
        printf("Test 12 output: [%s]\n", result);
        bool ok = true;
        if (strstr(result, "void samebrace()") == NULL) { printf("  FAIL: missing signature\n"); ok = false; }
        if (strstr(result, ") {") == NULL) { printf("  FAIL: brace not on same line\n"); ok = false; }
        if (strstr(result, "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 12 (same-line brace): PASSED\n"); passed++; }
        else { printf("Test 12 (same-line brace): FAILED\n"); failed++; }
    }

    // Test 13: empty params print as "()"
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "noparams";
        func->m_returnType = "int";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "noparams()") == NULL) { printf("  FAIL: missing 'noparams()'\n"); ok = false; }
        if (strstr(result, "int") == NULL) { printf("  FAIL: missing 'int'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (ok) { printf("Test 13 (empty params): PASSED\n"); passed++; }
        else { printf("Test 13 (empty params): FAILED\n"); failed++; }
    }

    // Test 14: many params (5) all survive
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "manyparams";
        func->m_returnType = "int";
        static const char* kParams[] = {"int pa", "int pb", "int pc", "int pd", "int pe"};
        for (size_t i = 0; i < 5; i++)
        {
            IdentifierNode* p = ASTNodeFactory::createIdentifierNode();
            p->m_name = kParams[i];
            func->m_params.push_back(p);
        }
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        for (size_t i = 0; i < 5; i++)
        {
            if (strstr(result, kParams[i]) == NULL) { printf("  FAIL: missing '%s'\n", kParams[i]); ok = false; }
        }
        if (strstr(result, "manyparams") == NULL) { printf("  FAIL: missing name\n"); ok = false; }
        if (ok) { printf("Test 14 (many params): PASSED\n"); passed++; }
        else { printf("Test 14 (many params): FAILED\n"); failed++; }
    }

    // Test 15: if/else both branches
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "cflag > 1";
        ifStmt->m_condition = cond;
        BlockNode* thenBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret1 = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* one = ASTNodeFactory::createLiteralNode();
        one->m_type = LiteralNode::TYPE_NUMBER;
        one->m_value = "11";
        ret1->m_argument = one;
        thenBody->m_statements.push_back(ret1);
        ifStmt->m_thenBranch = thenBody;
        BlockNode* elseBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret2 = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* two = ASTNodeFactory::createLiteralNode();
        two->m_type = LiteralNode::TYPE_NUMBER;
        two->m_value = "22";
        ret2->m_argument = two;
        elseBody->m_statements.push_back(ret2);
        ifStmt->m_elseBranch = elseBody;
        prog->m_statements.push_back(ifStmt);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "if") == NULL) { printf("  FAIL: missing 'if'\n"); ok = false; }
        if (strstr(result, "(cflag > 1)") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing 'else'\n"); ok = false; }
        if (strstr(result, "return 11;") == NULL) { printf("  FAIL: missing then ret\n"); ok = false; }
        if (strstr(result, "return 22;") == NULL) { printf("  FAIL: missing else ret\n"); ok = false; }
        if (ok) { printf("Test 15 (if/else): PASSED\n"); passed++; }
        else { printf("Test 15 (if/else): FAILED\n"); failed++; }
    }

    // Test 16: do-while passthrough stays verbatim
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "do { tick(); } while (alive);";
        prog->m_statements.push_back(raw);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "do { tick(); } while (alive);") == NULL) { printf("  FAIL: do-while altered\n"); ok = false; }
        if (strstr(result, "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 16 (do-while): PASSED\n"); passed++; }
        else { printf("Test 16 (do-while): FAILED\n"); failed++; }
    }

    // Test 17: switch with two cases and default
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "mode";
        sw->m_discriminant = disc;
        IdentifierNode* c1 = ASTNodeFactory::createIdentifierNode();
        c1->m_name = "case 9: run9(); break;";
        sw->m_cases.push_back(c1);
        IdentifierNode* c2 = ASTNodeFactory::createIdentifierNode();
        c2->m_name = "default: idle(); break;";
        sw->m_cases.push_back(c2);
        prog->m_statements.push_back(sw);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "switch") == NULL) { printf("  FAIL: missing 'switch'\n"); ok = false; }
        if (strstr(result, "(mode)") == NULL) { printf("  FAIL: missing '(mode)'\n"); ok = false; }
        if (strstr(result, "case 9: run9(); break;") == NULL) { printf("  FAIL: case altered\n"); ok = false; }
        if (strstr(result, "default: idle(); break;") == NULL) { printf("  FAIL: default altered\n"); ok = false; }
        if (ok) { printf("Test 17 (switch): PASSED\n"); passed++; }
        else { printf("Test 17 (switch): FAILED\n"); failed++; }
    }

    // Test 18: block comment keeps delimiters
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        CommentNode* cmt = ASTNodeFactory::createCommentNode();
        cmt->m_text = " block-note-42 ";
        cmt->m_isBlock = true;
        prog->m_statements.push_back(cmt);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "/*") == NULL) { printf("  FAIL: missing '/*'\n"); ok = false; }
        if (strstr(result, "block-note-42") == NULL) { printf("  FAIL: missing text\n"); ok = false; }
        if (strstr(result, "*/") == NULL) { printf("  FAIL: missing '*/'\n"); ok = false; }
        if (ok) { printf("Test 18 (block comment): PASSED\n"); passed++; }
        else { printf("Test 18 (block comment): FAILED\n"); failed++; }
    }

    // Test 19: trailing comment after a declaration
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "int";
        varDecl->m_name = "trailvar";
        prog->m_statements.push_back(varDecl);
        CommentNode* cmt = ASTNodeFactory::createCommentNode();
        cmt->m_text = " trailing-note-7";
        cmt->m_isBlock = false;
        prog->m_statements.push_back(cmt);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "trailvar") == NULL) { printf("  FAIL: missing var\n"); ok = false; }
        if (strstr(result, "trailing-note-7") == NULL) { printf("  FAIL: missing comment\n"); ok = false; }
        if (strstr(result, "//") == NULL) { printf("  FAIL: missing '//'\n"); ok = false; }
        if (ok) { printf("Test 19 (trailing comment): PASSED\n"); passed++; }
        else { printf("Test 19 (trailing comment): FAILED\n"); failed++; }
    }

    // Test 20: struct/union/typedef verbatim + enum members
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* st = ASTNodeFactory::createRawTextNode();
        st->m_text = "struct Node { int val; };";
        prog->m_statements.push_back(st);
        RawTextNode* un = ASTNodeFactory::createRawTextNode();
        un->m_text = "union Word { int i; };";
        prog->m_statements.push_back(un);
        RawTextNode* td = ASTNodeFactory::createRawTextNode();
        td->m_text = "typedef unsigned int u32;";
        prog->m_statements.push_back(td);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "BigColor";
        static const char* kMembers[] = {"BC_RED", "BC_GREEN", "BC_BLUE", "BC_ALPHA"};
        for (size_t i = 0; i < 4; i++)
        {
            IdentifierNode* m = ASTNodeFactory::createIdentifierNode();
            m->m_name = kMembers[i];
            enm->m_members.push_back(m);
        }
        prog->m_statements.push_back(enm);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "struct Node { int val; };") == NULL) { printf("  FAIL: struct altered\n"); ok = false; }
        if (strstr(result, "union Word { int i; };") == NULL) { printf("  FAIL: union altered\n"); ok = false; }
        if (strstr(result, "typedef unsigned int u32;") == NULL) { printf("  FAIL: typedef altered\n"); ok = false; }
        if (strstr(result, "enum BigColor") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        for (size_t i = 0; i < 4; i++)
        {
            if (strstr(result, kMembers[i]) == NULL) { printf("  FAIL: missing '%s'\n", kMembers[i]); ok = false; }
        }
        if (strstr(result, "};") == NULL) { printf("  FAIL: missing enum '};'\n"); ok = false; }
        if (ok) { printf("Test 20 (struct/union/typedef/enum): PASSED\n"); passed++; }
        else { printf("Test 20 (struct/union/typedef/enum): FAILED\n"); failed++; }
    }

    // Test 21: array declarator + string literal init
    {
        OutputBuffer out(4096);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* arr = ASTNodeFactory::createVariableDeclNode();
        arr->m_kind = "int";
        arr->m_name = "arrbuf[16]";
        prog->m_statements.push_back(arr);
        VariableDeclNode* str = ASTNodeFactory::createVariableDeclNode();
        str->m_kind = "const char *";
        str->m_name = "greeting";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_STRING;
        lit->m_value = "hello-c";
        str->m_initializer = lit;
        prog->m_statements.push_back(str);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "arrbuf[16]") == NULL) { printf("  FAIL: missing array decl\n"); ok = false; }
        if (strstr(result, "greeting") == NULL) { printf("  FAIL: missing string name\n"); ok = false; }
        if (strstr(result, "hello-c") == NULL) { printf("  FAIL: missing string value\n"); ok = false; }
        if (strstr(result, ";") == NULL) { printf("  FAIL: missing ';'\n"); ok = false; }
        if (ok) { printf("Test 21 (array/string): PASSED\n"); passed++; }
        else { printf("Test 21 (array/string): FAILED\n"); failed++; }
    }

    // Test 22: directives table (#include/#define/#ifdef/#pragma)
    {
        struct Case { const char* dir; };
        static const Case kCases[] = {
            {"#include <stdlib.h>"}, {"#include \"local.h\""},
            {"#define BUF 256"}, {"#ifdef DEBUG"},
            {"#pragma once"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            CFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            PreprocessorDirectiveNode* pp = ASTNodeFactory::createPreprocessorDirectiveNode();
            pp->m_directive = kCases[i].dir;
            prog->m_statements.push_back(pp);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].dir) == NULL) { printf("  FAIL row %u: directive altered\n", (unsigned)i); ok = false; }
            if (strstr(result, "placeholder") != NULL) { printf("  FAIL row %u: placeholder leak\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 22 (directives table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 23: empty program emits nothing
    {
        OutputBuffer out(256);
        Config config;
        CFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (out.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        if (result[0] != '\0') { printf("  FAIL: expected empty string\n"); ok = false; }
        if (strstr(result, "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 23 (empty program): PASSED\n"); passed++; }
        else { printf("Test 23 (empty program): FAILED\n"); failed++; }
    }

    // Test 24: invalid input kept verbatim, idempotent format
    {
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "??? not C !!! {{{";
        prog->m_statements.push_back(raw);
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "int";
        varDecl->m_name = "idemvar";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "3";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);
        OutputBuffer out1(4096);
        Config c1;
        CFormatter f1(out1, c1);
        f1.format(prog);
        OutputBuffer out2(4096);
        Config c2;
        CFormatter f2(out2, c2);
        f2.format(prog);
        const char* r1 = out1.getData();
        const char* r2 = out2.getData();
        bool ok = true;
        if (strstr(r1, "??? not C !!! {{{") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(r1, "idemvar") == NULL) { printf("  FAIL: missing var\n"); ok = false; }
        if (strcmp(r1, r2) != 0) { printf("  FAIL: format not idempotent\n"); ok = false; }
        if (strstr(r1, "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 24 (verbatim+idempotent): PASSED\n"); passed++; }
        else { printf("Test 24 (verbatim+idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("CFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
