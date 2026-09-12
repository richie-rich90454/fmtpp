// Easter egg: D formatter tests - unittest { assert(passes); }
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/DFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
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

    // Test 1: class with colon bases, struct kind
    {
        OutputBuffer out(4096);
        Config config;
        DFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
        sup->m_name = "Animal";
        cls->m_superClass = sup;
        IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
        iface->m_name = "Pet";
        cls->m_implements.push_back(iface);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class Dog : Animal, Pet") == NULL)
        {
            printf("  FAIL: missing 'class Dog : Animal, Pet'\n");
            ok = false;
        }
        if (strstr(result, "public") != NULL) { printf("  FAIL: unexpected 'public'\n"); ok = false; }
        if (ok) { printf("Test 1 (class): PASSED\n"); passed++; }
        else { printf("Test 1 (class): FAILED\n"); failed++; }
    }

    // Test 2: interface with colon bases
    {
        OutputBuffer out(4096);
        Config config;
        DFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "IList";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "ICollection";
        iface->m_extends.push_back(base);
        iface->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "interface IList : ICollection") == NULL)
        {
            printf("  FAIL: missing 'interface IList : ICollection'\n");
            ok = false;
        }
        if (strstr(result, "extends") != NULL) { printf("  FAIL: unexpected 'extends'\n"); ok = false; }
        if (ok) { printf("Test 2 (interface): PASSED\n"); passed++; }
        else { printf("Test 2 (interface): FAILED\n"); failed++; }
    }

    // Test 3: dotted import and module with semicolons
    {
        OutputBuffer out(4096);
        Config config;
        DFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "myapp";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "std.stdio";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "module myapp;") == NULL) { printf("  FAIL: missing 'module myapp;'\n"); ok = false; }
        if (strstr(result, "import std.stdio;") == NULL) { printf("  FAIL: missing 'import std.stdio;'\n"); ok = false; }
        if (ok) { printf("Test 3 (module/import): PASSED\n"); passed++; }
        else { printf("Test 3 (module/import): FAILED\n"); failed++; }
    }

    // Test 4: table-driven classes (kinds x bases, 12 rows)
    {
        struct Case { const char* kind; const char* name; };
        static const Case kCases[] = {
            {"", "dk00"}, {"class", "dk01"}, {"struct", "dk02"},
            {"union", "dk03"}, {"abstract class", "dk04"}, {"final class", "dk05"},
            {"", "dk06"}, {"struct", "dk07"}, {"class", "dk08"},
            {"union", "dk09"}, {"final class", "dk10"}, {"abstract class", "dk11"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            DFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
            cls->m_name = kCases[i].name;
            cls->m_kind = kCases[i].kind;
            IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
            sup->m_name = "BaseD";
            cls->m_superClass = sup;
            IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
            iface->m_name = "FaceD";
            cls->m_implements.push_back(iface);
            cls->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(cls);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, "BaseD") == NULL) { printf("  FAIL row %u: missing base\n", (unsigned)i); ok = false; }
            if (strstr(result, "FaceD") == NULL) { printf("  FAIL row %u: missing iface\n", (unsigned)i); ok = false; }
            if (strstr(result, ":") == NULL) { printf("  FAIL row %u: missing ':'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 4 (class table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 5: table-driven funcs (C-style same-line, 12 rows)
    {
        struct Case { const char* ret; const char* name; int nparams; };
        static const Case kCases[] = {
            {"int", "dfn00", 0}, {"void", "dfn01", 1}, {"string", "dfn02", 2},
            {"bool", "dfn03", 0}, {"double", "dfn04", 1}, {"char[]", "dfn05", 2},
            {"long", "dfn06", 3}, {"uint", "dfn07", 1}, {"float", "dfn08", 0},
            {"int", "dfn09", 2}, {"void", "dfn10", 3}, {"size_t", "dfn11", 1}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        static const char* kPool[] = {"int a", "string b", "bool c"};
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            DFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            func->m_name = kCases[i].name;
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
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].ret) == NULL) { printf("  FAIL row %u: missing ret\n", (unsigned)i); ok = false; }
            if (strstr(result, "(") == NULL) { printf("  FAIL row %u: missing '('\n", (unsigned)i); ok = false; }
            if (strstr(result, "{") == NULL) { printf("  FAIL row %u: missing '{'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 5 (func table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 6: imports/modules table + unittest + enum + control flow + comments
    {
        static const char* kMods[] = {"std.stdio", "std.array", "std.algorithm", "core.thread"};
        int tPassed = 0;
        for (int i = 0; i < 4; i++)
        {
            OutputBuffer out(4096);
            Config config;
            DFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            char modname[32];
            sprintf(modname, "dapp%d", i);
            NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
            ns->m_name = modname;
            prog->m_statements.push_back(ns);
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = kMods[i];
            prog->m_statements.push_back(imp);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            char wantMod[48];
            sprintf(wantMod, "module %s;", modname);
            if (strstr(result, wantMod) == NULL) { printf("  FAIL row %d: missing module\n", i); ok = false; }
            char wantImp[64];
            sprintf(wantImp, "import %s;", kMods[i]);
            if (strstr(result, wantImp) == NULL) { printf("  FAIL row %d: missing import\n", i); ok = false; }
            if (strstr(result, "extends") != NULL) { printf("  FAIL row %d: unexpected 'extends'\n", i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 6 (module/import table): %d/4 passed\n", tPassed);
        OutputBuffer out(8192);
        Config config;
        DFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "DColor";
        static const char* kMembers[] = {"dred", "dgreen", "dblue"};
        for (int i = 0; i < 3; i++)
        {
            IdentifierNode* m = ASTNodeFactory::createIdentifierNode();
            m->m_name = kMembers[i];
            enm->m_members.push_back(m);
        }
        prog->m_statements.push_back(enm);
        RawTextNode* ut = ASTNodeFactory::createRawTextNode();
        ut->m_text = "unittest { assert(true); }";
        prog->m_statements.push_back(ut);
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* ic = ASTNodeFactory::createIdentifierNode();
        ic->m_name = "dflag";
        ifStmt->m_condition = ic;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = true;
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "int";
        init->m_name = "di";
        LiteralNode* z = ASTNodeFactory::createLiteralNode();
        z->m_type = LiteralNode::TYPE_NUMBER;
        z->m_value = "0";
        init->m_initializer = z;
        forStmt->m_init = init;
        IdentifierNode* fc = ASTNodeFactory::createIdentifierNode();
        fc->m_name = "di < 6";
        forStmt->m_condition = fc;
        IdentifierNode* fu = ASTNodeFactory::createIdentifierNode();
        fu->m_name = "di++";
        forStmt->m_update = fu;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);
        WhileStmtNode* w = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* wc = ASTNodeFactory::createIdentifierNode();
        wc->m_name = "dwork";
        w->m_condition = wc;
        w->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(w);
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "dmode";
        sw->m_discriminant = disc;
        IdentifierNode* arm = ASTNodeFactory::createIdentifierNode();
        arm->m_name = "case 3: go3(); break;";
        sw->m_cases.push_back(arm);
        prog->m_statements.push_back(sw);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " d-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " d-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "enum DColor") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        for (int i = 0; i < 3; i++)
        {
            if (strstr(result, kMembers[i]) == NULL) { printf("  FAIL: missing '%s'\n", kMembers[i]); ok = false; }
        }
        if (strstr(result, "unittest { assert(true); }") == NULL) { printf("  FAIL: unittest altered\n"); ok = false; }
        if (strstr(result, "if (dflag)") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "int di = 0") == NULL) { printf("  FAIL: missing init\n"); ok = false; }
        if (strstr(result, "di < 6") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "di++") == NULL) { printf("  FAIL: missing update\n"); ok = false; }
        if (strstr(result, "while (dwork)") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "switch (dmode)") == NULL) { printf("  FAIL: missing switch\n"); ok = false; }
        if (strstr(result, "case 3: go3(); break;") == NULL) { printf("  FAIL: case altered\n"); ok = false; }
        if (strstr(result, "d-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "d-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 6b (enum/flow/comments): PASSED\n"); passed++; }
        else { printf("Test 6b (enum/flow/comments): FAILED\n"); failed++; }
    }

    // Test 7: empty + verbatim + idempotent
    {
        OutputBuffer outEmpty(256);
        Config cEmpty;
        DFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        if (strstr(outEmpty.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "@@@ bad d ~~~";
        prog->m_statements.push_back(raw);
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "IdemD";
        iface->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(iface);
        OutputBuffer out1(4096);
        Config cc1;
        DFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        DFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "@@@ bad d ~~~") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "IdemD") == NULL) { printf("  FAIL: missing iface\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 7 (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 7 (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("DFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
