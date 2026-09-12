// Easter egg: V formatter tests - v is for victory over ugly code
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/VFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: mut binding keeps mut
    {
        OutputBuffer out(4096);
        Config config;
        VFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "mut";
        varDecl->m_name = "count";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "0";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "mut count := 0") == NULL) { printf("  FAIL: missing 'mut count := 0'\n"); ok = false; }
        if (ok) { printf("Test 1 (mut): PASSED\n"); passed++; }
        else { printf("Test 1 (mut): FAILED\n"); failed++; }
    }

    // Test 2: real enum, not iota
    {
        OutputBuffer out(4096);
        Config config;
        VFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "Color";
        IdentifierNode* member = ASTNodeFactory::createIdentifierNode();
        member->m_name = "red";
        enm->m_members.push_back(member);
        prog->m_statements.push_back(enm);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "enum Color") == NULL) { printf("  FAIL: missing 'enum Color'\n"); ok = false; }
        if (strstr(result, "red") == NULL) { printf("  FAIL: missing 'red'\n"); ok = false; }
        if (strstr(result, "iota") != NULL) { printf("  FAIL: unexpected 'iota'\n"); ok = false; }
        if (ok) { printf("Test 2 (enum): PASSED\n"); passed++; }
        else { printf("Test 2 (enum): FAILED\n"); failed++; }
    }

    // Test 3: module, unquoted import, verbatim flag
    {
        OutputBuffer out(4096);
        Config config;
        VFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "main";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "os";
        prog->m_statements.push_back(imp);
        PreprocessorDirectiveNode* flag = ASTNodeFactory::createPreprocessorDirectiveNode();
        flag->m_directive = "#flag -lm";
        prog->m_statements.push_back(flag);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "module main") == NULL) { printf("  FAIL: missing 'module main'\n"); ok = false; }
        if (strstr(result, "import os") == NULL) { printf("  FAIL: missing 'import os'\n"); ok = false; }
        if (strstr(result, "\"os\"") != NULL) { printf("  FAIL: import must not be quoted\n"); ok = false; }
        if (strstr(result, "#flag -lm") == NULL) { printf("  FAIL: missing '#flag -lm'\n"); ok = false; }
        if (strstr(result, "go:build") != NULL) { printf("  FAIL: unexpected 'go:build'\n"); ok = false; }
        if (ok) { printf("Test 3 (module/import/flag): PASSED\n"); passed++; }
        else { printf("Test 3 (module/import/flag): FAILED\n"); failed++; }
    }

    // Test 4: table-driven := bindings (mut/let, 12 rows)
    {
        struct Case { const char* kind; const char* name; const char* init; };
        static const Case kCases[] = {
            {"mut", "vx00", "0"}, {"let", "vx01", "1"},
            {"mut", "vx02", "2"}, {"let", "vx03", "3"},
            {"mut", "vx04", "4"}, {"let", "vx05", "5"},
            {"mut", "vx06", "6"}, {"let", "vx07", "7"},
            {"mut", "vx08", "8"}, {"let", "vx09", "9"},
            {"mut", "vx10", "10"}, {"let", "vx11", "11"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            VFormatter formatter(out, config);
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
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].init) == NULL) { printf("  FAIL row %u: missing init\n", (unsigned)i); ok = false; }
            if (strstr(result, ":=") == NULL) { printf("  FAIL row %u: missing ':='\n", (unsigned)i); ok = false; }
            if (strstr(result, ";") != NULL) { printf("  FAIL row %u: unexpected ';'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 4 (binding table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 5: table-driven fns print fn not func (12 rows)
    {
        struct Case { const char* ret; int nparams; };
        static const Case kCases[] = {
            {"int", 1}, {"string", 2}, {"bool", 0}, {"", 1},
            {"int", 2}, {"f64", 1}, {"", 0}, {"bool", 2},
            {"string", 0}, {"int", 3}, {"u8", 1}, {"", 2}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        static const char* kPool[] = {"a int", "b string", "c bool"};
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            VFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            char fname[32];
            sprintf(fname, "vfn%02u", (unsigned)i);
            func->m_name = fname;
            func->m_returnType = kCases[i].ret;
            for (int p = 0; p < kCases[i].nparams; p++)
            {
                IdentifierNode* param = ASTNodeFactory::createIdentifierNode();
                param->m_name = kPool[p];
                func->m_params.push_back(param);
            }
            func->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(func);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, "fn") == NULL) { printf("  FAIL row %u: missing 'fn'\n", (unsigned)i); ok = false; }
            if (strstr(result, fname) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, "(") == NULL) { printf("  FAIL row %u: missing '('\n", (unsigned)i); ok = false; }
            if (strstr(result, ")") == NULL) { printf("  FAIL row %u: missing ')'\n", (unsigned)i); ok = false; }
            if (strstr(result, "func") != NULL) { printf("  FAIL row %u: unexpected 'func'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 5 (fn table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 6: structs table + single-quote strings + flow + comments
    {
        static const char* kStructs[] = {"PtA", "PtB", "PtC", "PtD"};
        int tPassed = 0;
        for (int i = 0; i < 4; i++)
        {
            OutputBuffer out(4096);
            Config config;
            VFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
            cls->m_name = kStructs[i];
            cls->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(cls);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            char want[64];
            sprintf(want, "struct %s", kStructs[i]);
            if (strstr(result, want) == NULL) { printf("  FAIL row %d: missing '%s'\n", i, want); ok = false; }
            if (strstr(result, "type ") != NULL) { printf("  FAIL row %d: unexpected Go 'type'\n", i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 6 (struct table): %d/4 passed\n", tPassed);
        OutputBuffer out(8192);
        Config config;
        VFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* st = ASTNodeFactory::createVariableDeclNode();
        st->m_kind = "let";
        st->m_name = "word";
        LiteralNode* sl = ASTNodeFactory::createLiteralNode();
        sl->m_type = LiteralNode::TYPE_STRING;
        sl->m_value = "vlang";
        st->m_initializer = sl;
        prog->m_statements.push_back(st);
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* ic = ASTNodeFactory::createIdentifierNode();
        ic->m_name = "vflag";
        ifStmt->m_condition = ic;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* rng = ASTNodeFactory::createForStmtNode();
        IdentifierNode* rc = ASTNodeFactory::createIdentifierNode();
        rc->m_name = "i in 0..4";
        rng->m_condition = rc;
        rng->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(rng);
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "vsel";
        sw->m_discriminant = disc;
        IdentifierNode* arm = ASTNodeFactory::createIdentifierNode();
        arm->m_name = "1 { one() }";
        sw->m_cases.push_back(arm);
        prog->m_statements.push_back(sw);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " v-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " v-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "'vlang'") == NULL) { printf("  FAIL: string not single-quoted\n"); ok = false; }
        if (strstr(result, "\"vlang\"") != NULL) { printf("  FAIL: string must not be double-quoted\n"); ok = false; }
        if (strstr(result, "if vflag") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "for i in 0..4") == NULL) { printf("  FAIL: missing range for\n"); ok = false; }
        if (strstr(result, "switch vsel") == NULL && strstr(result, "match vsel") == NULL) { printf("  FAIL: missing switch/match\n"); ok = false; }
        if (strstr(result, "1 { one() }") == NULL) { printf("  FAIL: arm altered\n"); ok = false; }
        if (strstr(result, "v-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "v-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 6b (strings/flow/comments): PASSED\n"); passed++; }
        else { printf("Test 6b (strings/flow/comments): FAILED\n"); failed++; }
    }

    // Test 7: modules/imports table + empty + verbatim + idempotent
    {
        static const char* kMods[] = {"main", "cli", "net", "json"};
        static const char* kImps[] = {"os", "flag", "net", "json"};
        int tPassed = 0;
        for (int i = 0; i < 4; i++)
        {
            OutputBuffer out(4096);
            Config config;
            VFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
            ns->m_name = kMods[i];
            prog->m_statements.push_back(ns);
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = kImps[i];
            prog->m_statements.push_back(imp);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            char wantMod[48];
            sprintf(wantMod, "module %s", kMods[i]);
            if (strstr(result, wantMod) == NULL) { printf("  FAIL row %d: missing module\n", i); ok = false; }
            char wantImp[48];
            sprintf(wantImp, "import %s", kImps[i]);
            if (strstr(result, wantImp) == NULL) { printf("  FAIL row %d: missing import\n", i); ok = false; }
            char quoted[48];
            sprintf(quoted, "\"%s\"", kImps[i]);
            if (strstr(result, quoted) != NULL) { printf("  FAIL row %d: import must not be quoted\n", i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 7 (module/import table): %d/4 passed\n", tPassed);
        OutputBuffer outEmpty(256);
        Config cEmpty;
        VFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "@@@ bad v ~~~";
        prog->m_statements.push_back(raw);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "VIdem";
        IdentifierNode* m = ASTNodeFactory::createIdentifierNode();
        m->m_name = "va";
        enm->m_members.push_back(m);
        prog->m_statements.push_back(enm);
        OutputBuffer out1(4096);
        Config cc1;
        VFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        VFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "@@@ bad v ~~~") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "enum VIdem") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        if (strstr(out1.getData(), "iota") != NULL) { printf("  FAIL: unexpected 'iota'\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 7b (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 7b (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("VFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
