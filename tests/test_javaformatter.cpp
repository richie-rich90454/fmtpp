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
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"

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

    // Test 4: Java switch prints cases verbatim (no invented breaks,
    // so intentional fallthrough survives formatting)
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
        caseNode->m_name = "case 1: one(); break;";
        sw->m_cases.push_back(caseNode);
        prog->m_statements.push_back(sw);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "switch") == NULL) { printf("  FAIL: missing 'switch'\n"); ok = false; }
        if (strstr(result, "day") == NULL) { printf("  FAIL: missing 'day'\n"); ok = false; }
        if (strstr(result, "case 1: one(); break;") == NULL) { printf("  FAIL: case text altered\n"); ok = false; }
        // exactly one break: first occurrence must be the last one
        const char* first = strstr(result, "break;");
        if (first == NULL) { printf("  FAIL: missing 'break;'\n"); ok = false; }
        else if (strstr(first + 1, "break;") != NULL) { printf("  FAIL: duplicated 'break;'\n"); ok = false; }
        if (ok) { printf("Test 4 (switch verbatim): PASSED\n"); passed++; }
        else { printf("Test 4 (switch verbatim): FAILED\n"); failed++; }
    }

    // Test 5: Java try/catch/finally with a real catch body
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
        BlockNode* catchBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* catchRet = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* negOne = ASTNodeFactory::createLiteralNode();
        negOne->m_type = LiteralNode::TYPE_NUMBER;
        negOne->m_value = "-1";
        catchRet->m_argument = negOne;
        catchBody->m_statements.push_back(catchRet);
        tc->m_catchBody = catchBody;
        BlockNode* finallyBody = ASTNodeFactory::createBlockNode();
        tc->m_finallyBlock = finallyBody;
        prog->m_statements.push_back(tc);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "try") == NULL) { printf("  FAIL: missing 'try'\n"); ok = false; }
        if (strstr(result, "catch (Exception e)") == NULL) { printf("  FAIL: missing 'catch (Exception e)'\n"); ok = false; }
        if (strstr(result, "return -1;") == NULL) { printf("  FAIL: catch body dropped\n"); ok = false; }
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

    // Test 8: table-driven typed var decls
    {
        struct Case { const char* kind; const char* name; const char* init; };
        static const Case kCases[] = {
            {"int", "jv00", "0"}, {"long", "jv01", "1"},
            {"double", "jv02", "2"}, {"boolean", "jv03", "3"},
            {"char", "jv04", "4"}, {"String", "jv05", "5"},
            {"final int", "jv06", "6"}, {"private String", "jv07", "7"},
            {"List<String>", "jv08", "8"}, {"int[]", "jv09", "9"},
            {"float", "jv10", "10"}, {"short", "jv11", "11"},
            {"byte", "jv12", "12"}, {"Object", "jv13", "13"},
            {"static int", "jv14", "14"}, {"volatile long", "jv15", "15"},
            {"Map<String,Integer>", "jv16", "16"}, {"double[]", "jv17", "17"},
            {"var", "jv18", "18"}, {"final String", "jv19", "19"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            JavaFormatter formatter(out, config);
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
        printf("Test 8 (var table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 9: table-driven methods (visibility x return types)
    {
        struct Case { const char* mods; const char* ret; const char* name; };
        static const Case kCases[] = {
            {"", "void", "jvm00"}, {"private", "void", "jvm01"},
            {"protected", "int", "jvm02"}, {"static", "String", "jvm03"},
            {"final", "boolean", "jvm04"}, {"private static", "int", "jvm05"},
            {"", "long", "jvm06"}, {"public static", "void", "jvm07"},
            {"", "double", "jvm08"}, {"protected", "Object", "jvm09"},
            {"static", "long", "jvm10"}, {"", "char", "jvm11"},
            {"private", "int", "jvm12"}, {"final", "String", "jvm13"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            JavaFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            func->m_name = kCases[i].name;
            func->m_modifiers = kCases[i].mods;
            func->m_returnType = kCases[i].ret;
            func->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(func);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].ret) == NULL) { printf("  FAIL row %u: missing ret\n", (unsigned)i); ok = false; }
            if (strstr(result, "(") == NULL) { printf("  FAIL row %u: missing '('\n", (unsigned)i); ok = false; }
            if (strstr(result, "{") == NULL) { printf("  FAIL row %u: missing '{'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 9 (method table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 10: default visibility is public, explicit wins verbatim
    {
        OutputBuffer out(8192);
        Config config;
        JavaFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f1 = ASTNodeFactory::createFunctionDeclNode();
        f1->m_name = "defVis";
        f1->m_returnType = "void";
        f1->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(f1);
        FunctionDeclNode* f2 = ASTNodeFactory::createFunctionDeclNode();
        f2->m_name = "privVis";
        f2->m_modifiers = "private static";
        f2->m_returnType = "int";
        f2->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(f2);
        ClassDeclNode* c1 = ASTNodeFactory::createClassDeclNode();
        c1->m_name = "FinalDog";
        c1->m_kind = "final class";
        c1->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(c1);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "public void defVis(") == NULL) { printf("  FAIL: default not public\n"); ok = false; }
        if (strstr(result, "private static int privVis(") == NULL) { printf("  FAIL: explicit mods lost\n"); ok = false; }
        if (strstr(result, "final class FinalDog") == NULL) { printf("  FAIL: kind not verbatim\n"); ok = false; }
        if (strstr(result, "public final class") != NULL) { printf("  FAIL: doubled public\n"); ok = false; }
        if (ok) { printf("Test 10 (visibility): PASSED\n"); passed++; }
        else { printf("Test 10 (visibility): FAILED\n"); failed++; }
    }

    // Test 11: package + imports table + interface + enum
    {
        OutputBuffer out(8192);
        Config config;
        JavaFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "com.example.shop";
        prog->m_statements.push_back(ns);
        static const char* kImports[] = {"java.util.List", "java.util.Map", "java.io.File", "static java.lang.Math"};
        for (size_t i = 0; i < 4; i++)
        {
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = kImports[i];
            prog->m_statements.push_back(imp);
        }
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "Named";
        iface->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(iface);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "JColor";
        IdentifierNode* m1 = ASTNodeFactory::createIdentifierNode();
        m1->m_name = "J_RED";
        enm->m_members.push_back(m1);
        IdentifierNode* m2 = ASTNodeFactory::createIdentifierNode();
        m2->m_name = "J_GREEN";
        enm->m_members.push_back(m2);
        prog->m_statements.push_back(enm);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "package com.example.shop;") == NULL) { printf("  FAIL: missing package\n"); ok = false; }
        for (size_t i = 0; i < 4; i++)
        {
            if (strstr(result, kImports[i]) == NULL) { printf("  FAIL: missing import '%s'\n", kImports[i]); ok = false; }
        }
        if (strstr(result, "interface Named") == NULL) { printf("  FAIL: missing iface\n"); ok = false; }
        if (strstr(result, "enum JColor") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        if (strstr(result, "J_RED") == NULL) { printf("  FAIL: missing member\n"); ok = false; }
        if (strstr(result, "J_GREEN") == NULL) { printf("  FAIL: missing member\n"); ok = false; }
        if (ok) { printf("Test 11 (package/import/iface/enum): PASSED\n"); passed++; }
        else { printf("Test 11 (package/import/iface/enum): FAILED\n"); failed++; }
    }

    // Test 12: control flow composite + multi-catch + comments
    {
        OutputBuffer out(8192);
        Config config;
        JavaFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* icond = ASTNodeFactory::createIdentifierNode();
        icond->m_name = "jflag";
        ifStmt->m_condition = icond;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = true;
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "int";
        init->m_name = "ji";
        LiteralNode* z = ASTNodeFactory::createLiteralNode();
        z->m_type = LiteralNode::TYPE_NUMBER;
        z->m_value = "0";
        init->m_initializer = z;
        forStmt->m_init = init;
        IdentifierNode* fcond = ASTNodeFactory::createIdentifierNode();
        fcond->m_name = "ji < 4";
        forStmt->m_condition = fcond;
        IdentifierNode* fupd = ASTNodeFactory::createIdentifierNode();
        fupd->m_name = "ji++";
        forStmt->m_update = fupd;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);
        WhileStmtNode* whileStmt = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* wcond = ASTNodeFactory::createIdentifierNode();
        wcond->m_name = "working";
        whileStmt->m_condition = wcond;
        whileStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(whileStmt);
        TryCatchStmtNode* tc = ASTNodeFactory::createTryCatchStmtNode();
        tc->m_body = ASTNodeFactory::createBlockNode();
        IdentifierNode* cc1 = ASTNodeFactory::createIdentifierNode();
        cc1->m_name = "IOException ioe";
        tc->m_catchClause = cc1;
        tc->m_catchBody = ASTNodeFactory::createBlockNode();
        TryCatchStmtNode* extra = ASTNodeFactory::createTryCatchStmtNode();
        IdentifierNode* cc2 = ASTNodeFactory::createIdentifierNode();
        cc2->m_name = "RuntimeException re";
        extra->m_catchClause = cc2;
        extra->m_catchBody = ASTNodeFactory::createBlockNode();
        tc->m_extraCatches.push_back(extra);
        prog->m_statements.push_back(tc);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " j-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " j-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "if (jflag)") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "for") == NULL) { printf("  FAIL: missing for\n"); ok = false; }
        if (strstr(result, "int ji = 0") == NULL) { printf("  FAIL: missing init\n"); ok = false; }
        if (strstr(result, "ji < 4") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "ji++") == NULL) { printf("  FAIL: missing update\n"); ok = false; }
        if (strstr(result, "while (working)") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "catch (IOException ioe)") == NULL) { printf("  FAIL: missing catch1\n"); ok = false; }
        if (strstr(result, "catch (RuntimeException re)") == NULL) { printf("  FAIL: missing catch2\n"); ok = false; }
        if (strstr(result, "j-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "j-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 12 (control flow): PASSED\n"); passed++; }
        else { printf("Test 12 (control flow): FAILED\n"); failed++; }
    }

    // Test 13: empty program + verbatim + idempotency
    {
        OutputBuffer outEmpty(256);
        Config cEmpty;
        JavaFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        if (strstr(outEmpty.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "@@@ bad java ###";
        prog->m_statements.push_back(raw);
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "IdemJava";
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);
        OutputBuffer out1(4096);
        Config cc1;
        JavaFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        JavaFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "@@@ bad java ###") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "IdemJava") == NULL) { printf("  FAIL: missing class\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 13 (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 13 (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("JavaFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
