// Easter egg: Zig formatter tests - comptime assertions, runtime passes
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/ZigFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: pub fn with trailing return type
    {
        OutputBuffer out(4096);
        Config config;
        ZigFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_modifiers = "pub";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "pub fn main() void") == NULL) { printf("  FAIL: missing 'pub fn main() void'\n"); ok = false; }
        if (strstr(result, "->") != NULL) { printf("  FAIL: unexpected '->'\n"); ok = false; }
        if (ok) { printf("Test 1 (fn decl): PASSED\n"); passed++; }
        else { printf("Test 1 (fn decl): FAILED\n"); failed++; }
    }

    // Test 2: const struct container
    {
        OutputBuffer out(4096);
        Config config;
        ZigFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Point";
        cls->m_kind = "struct";
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "const Point = struct") == NULL) { printf("  FAIL: missing 'const Point = struct'\n"); ok = false; }
        if (ok) { printf("Test 2 (struct): PASSED\n"); passed++; }
        else { printf("Test 2 (struct): FAILED\n"); failed++; }
    }

    // Test 3: const x: T = v and @import
    {
        OutputBuffer out(4096);
        Config config;
        ZigFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "const";
        varDecl->m_name = "x";
        varDecl->m_type = "i32";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "5";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "std";
        IdentifierNode* def = ASTNodeFactory::createIdentifierNode();
        def->m_name = "std";
        imp->m_defaultImport = def;
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "const x: i32 = 5;") == NULL) { printf("  FAIL: missing 'const x: i32 = 5;'\n"); ok = false; }
        if (strstr(result, "const std = @import(\"std\");") == NULL) { printf("  FAIL: missing '@import'\n"); ok = false; }
        if (ok) { printf("Test 3 (const/import): PASSED\n"); passed++; }
        else { printf("Test 3 (const/import): FAILED\n"); failed++; }
    }

    // Test 4: for with capture, no added parentheses
    {
        OutputBuffer out(4096);
        Config config;
        ZigFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = false;
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "(items) |x|";
        forStmt->m_condition = cond;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "for (items) |x|") == NULL) { printf("  FAIL: missing 'for (items) |x|'\n"); ok = false; }
        if (strstr(result, "for ((items)") != NULL) { printf("  FAIL: doubled parentheses\n"); ok = false; }
        if (ok) { printf("Test 4 (for capture): PASSED\n"); passed++; }
        else { printf("Test 4 (for capture): FAILED\n"); failed++; }
    }

    // Test 5: table-driven fns (modifiers x return types, 14 rows)
    {
        struct Case { const char* mods; const char* ret; };
        static const Case kCases[] = {
            {"pub", "void"}, {"", "void"}, {"pub", "i32"},
            {"export", "void"}, {"", "u8"}, {"pub", "!void"},
            {"", "i32"}, {"pub", "bool"}, {"", "f32"},
            {"export", "i32"}, {"pub", "usize"}, {"", "void"},
            {"pub", "c_int"}, {"", "bool"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            ZigFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            char fname[32];
            sprintf(fname, "zfn%02u", (unsigned)i);
            func->m_name = fname;
            func->m_modifiers = kCases[i].mods;
            func->m_returnType = kCases[i].ret;
            if ((i % 3) == 0)
            {
                IdentifierNode* p = ASTNodeFactory::createIdentifierNode();
                p->m_name = "x: i32";
                func->m_params.push_back(p);
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
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 5 (fn table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 6: table-driven var decls (const/var x types, 12 rows)
    {
        struct Case { const char* kind; const char* name; const char* type; const char* init; };
        static const Case kCases[] = {
            {"const", "zx00", "i32", "0"}, {"var", "zx01", "u8", "1"},
            {"const", "zx02", "bool", "2"}, {"var", "zx03", "f32", "3"},
            {"const", "zx04", "usize", "4"}, {"var", "zx05", "c_int", "5"},
            {"const", "zx06", "u64", "6"}, {"var", "zx07", "i8", "7"},
            {"const", "zx08", "f64", "8"}, {"var", "zx09", "u16", "9"},
            {"const", "zx10", "isize", "10"}, {"var", "zx11", "u32", "11"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            ZigFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
            varDecl->m_kind = kCases[i].kind;
            varDecl->m_name = kCases[i].name;
            varDecl->m_type = kCases[i].type;
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
            if (strstr(result, kCases[i].type) == NULL) { printf("  FAIL row %u: missing type\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].init) == NULL) { printf("  FAIL row %u: missing init\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 6 (var table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 7: struct/enum/union kinds + prototypes + control flow + comments
    {
        OutputBuffer out(8192);
        Config config;
        ZigFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        static const char* kKinds[] = {"struct", "enum", "union"};
        static const char* kNames[] = {"ZStruct", "ZEnum", "ZUnion"};
        for (int i = 0; i < 3; i++)
        {
            ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
            cls->m_name = kNames[i];
            cls->m_kind = kKinds[i];
            cls->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(cls);
        }
        FunctionDeclNode* proto = ASTNodeFactory::createFunctionDeclNode();
        proto->m_name = "zproto";
        proto->m_returnType = "void";
        proto->m_body = NULL;
        prog->m_statements.push_back(proto);
        WhileStmtNode* w = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* wc = ASTNodeFactory::createIdentifierNode();
        wc->m_name = "zok";
        w->m_condition = wc;
        w->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(w);
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* ic = ASTNodeFactory::createIdentifierNode();
        ic->m_name = "zflag";
        ifStmt->m_condition = ic;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "zsel";
        sw->m_discriminant = disc;
        IdentifierNode* arm = ASTNodeFactory::createIdentifierNode();
        arm->m_name = "0 => one,";
        sw->m_cases.push_back(arm);
        prog->m_statements.push_back(sw);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " zig-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " zig-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        for (int i = 0; i < 3; i++)
        {
            char want[64];
            sprintf(want, "const %s = %s", kNames[i], kKinds[i]);
            if (strstr(result, want) == NULL) { printf("  FAIL: missing '%s'\n", want); ok = false; }
        }
        if (strstr(result, "->") != NULL) { printf("  FAIL: unexpected '->'\n"); ok = false; }
        if (strstr(result, "fn zproto() void;") == NULL) { printf("  FAIL: missing prototype\n"); ok = false; }
        if (strstr(result, "while (zok)") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "if (zflag)") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "switch (zsel)") == NULL) { printf("  FAIL: missing switch\n"); ok = false; }
        if (strstr(result, "0 => one,") == NULL) { printf("  FAIL: arm altered\n"); ok = false; }
        if (strstr(result, "zig-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "zig-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 7 (kinds/proto/flow): PASSED\n"); passed++; }
        else { printf("Test 7 (kinds/proto/flow): FAILED\n"); failed++; }
    }

    // Test 8: imports table + empty + verbatim + idempotent
    {
        static const char* kSrc[] = {"std", "builtin", "root", "c"};
        int tPassed = 0;
        for (int i = 0; i < 4; i++)
        {
            OutputBuffer out(4096);
            Config config;
            ZigFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = kSrc[i];
            char dname[32];
            sprintf(dname, "zimp%d", i);
            IdentifierNode* def = ASTNodeFactory::createIdentifierNode();
            def->m_name = dname;
            imp->m_defaultImport = def;
            prog->m_statements.push_back(imp);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            char want[64];
            sprintf(want, "const %s = @import(\"%s\");", dname, kSrc[i]);
            if (strstr(result, want) == NULL) { printf("  FAIL row %d: missing '%s'\n", i, want); ok = false; }
            if (strstr(result, "placeholder") != NULL) { printf("  FAIL row %d: placeholder leak\n", i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 8 (import table): %d/4 passed\n", tPassed);
        OutputBuffer outEmpty(256);
        Config cEmpty;
        ZigFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "### bad zig %%%";
        prog->m_statements.push_back(raw);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "ZIdem";
        IdentifierNode* m = ASTNodeFactory::createIdentifierNode();
        m->m_name = "za";
        enm->m_members.push_back(m);
        prog->m_statements.push_back(enm);
        OutputBuffer out1(4096);
        Config cc1;
        ZigFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        ZigFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "### bad zig %%%") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "ZIdem") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 8b (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 8b (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("ZigFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
