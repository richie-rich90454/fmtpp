// Easter egg: C# formatter tests - Microsoft's answer to Java, with extra sharpness
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CSharpFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
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

    // Test 1: C# class with base class + interface
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* superCls = ASTNodeFactory::createIdentifierNode();
        superCls->m_name = "Animal";
        cls->m_superClass = superCls;
        IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
        iface->m_name = "IDisposable";
        cls->m_implements.push_back(iface);
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public") == NULL) { printf("  FAIL: missing 'public'\n"); ok = false; }
        if (strstr(result, "class") == NULL) { printf("  FAIL: missing 'class'\n"); ok = false; }
        if (strstr(result, "Dog") == NULL) { printf("  FAIL: missing 'Dog'\n"); ok = false; }
        if (strstr(result, ":") == NULL) { printf("  FAIL: missing ':'\n"); ok = false; }
        if (strstr(result, "Animal") == NULL) { printf("  FAIL: missing 'Animal'\n"); ok = false; }
        if (strstr(result, "IDisposable") == NULL) { printf("  FAIL: missing 'IDisposable'\n"); ok = false; }
        if (ok) { printf("Test 1 (class bases): PASSED\n"); passed++; }
        else { printf("Test 1 (class bases): FAILED\n"); failed++; }
    }

    // Test 2: C# method declaration (public ReturnType Name)
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Main";
        func->m_returnType = "void";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public void Main(") == NULL) { printf("  FAIL: missing 'public void Main('\n"); ok = false; }
        if (ok) { printf("Test 2 (method decl): PASSED\n"); passed++; }
        else { printf("Test 2 (method decl): FAILED\n"); failed++; }
    }

    // Test 3: C# namespace declaration
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "MyApp";
        BlockNode* nsBody = ASTNodeFactory::createBlockNode();
        ns->m_body = nsBody;
        prog->m_statements.push_back(ns);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "namespace") == NULL) { printf("  FAIL: missing 'namespace'\n"); ok = false; }
        if (strstr(result, "MyApp") == NULL) { printf("  FAIL: missing 'MyApp'\n"); ok = false; }
        if (ok) { printf("Test 3 (namespace): PASSED\n"); passed++; }
        else { printf("Test 3 (namespace): FAILED\n"); failed++; }
    }

    // Test 4: C# interface with extended interface
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "IList";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "ICollection";
        iface->m_extends.push_back(base);
        BlockNode* ifaceBody = ASTNodeFactory::createBlockNode();
        iface->m_body = ifaceBody;
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public interface IList") == NULL) { printf("  FAIL: missing 'public interface IList'\n"); ok = false; }
        if (strstr(result, "ICollection") == NULL) { printf("  FAIL: missing 'ICollection'\n"); ok = false; }
        if (ok) { printf("Test 4 (interface): PASSED\n"); passed++; }
        else { printf("Test 4 (interface): FAILED\n"); failed++; }
    }

    // Test 5: C# method with params and return
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Add";
        func->m_returnType = "int";
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "int a";
        func->m_params.push_back(paramA);
        IdentifierNode* paramB = ASTNodeFactory::createIdentifierNode();
        paramB->m_name = "int b";
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
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public int Add(") == NULL) { printf("  FAIL: missing 'public int Add('\n"); ok = false; }
        if (strstr(result, "int a") == NULL) { printf("  FAIL: missing 'int a'\n"); ok = false; }
        if (strstr(result, "int b") == NULL) { printf("  FAIL: missing 'int b'\n"); ok = false; }
        if (strstr(result, "return") == NULL) { printf("  FAIL: missing 'return'\n"); ok = false; }
        if (ok) { printf("Test 5 (method with params): PASSED\n"); passed++; }
        else { printf("Test 5 (method with params): FAILED\n"); failed++; }
    }

    // Test 6: C# method prototype without body
    {
        OutputBuffer out(4096);
        Config config;
        CSharpFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Dispose";
        func->m_returnType = "void";
        func->m_body = NULL;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "public void Dispose();") == NULL) { printf("  FAIL: missing 'public void Dispose();'\n"); ok = false; }
        if (ok) { printf("Test 6 (method prototype): PASSED\n"); passed++; }
        else { printf("Test 6 (method prototype): FAILED\n"); failed++; }
    }

    // Test 7: table-driven methods (modifiers x return types)
    {
        struct Case { const char* mods; const char* ret; const char* name; };
        static const Case kCases[] = {
            {"", "void", "csm00"}, {"static", "void", "csm01"},
            {"virtual", "int", "csm02"}, {"override", "string", "csm03"},
            {"private static", "int", "csm04"}, {"", "bool", "csm05"},
            {"async", "void", "csm06"}, {"sealed", "int", "csm07"},
            {"", "double", "csm08"}, {"static", "string", "csm09"},
            {"public static", "void", "csm10"}, {"private", "bool", "csm11"},
            {"", "long", "csm12"}, {"virtual", "double", "csm13"},
            {"override", "void", "csm14"}, {"static", "long", "csm15"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            CSharpFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            func->m_name = kCases[i].name;
            func->m_modifiers = kCases[i].mods;
            func->m_returnType = kCases[i].ret;
            if ((i % 2) == 0) { func->m_body = ASTNodeFactory::createBlockNode(); }
            else { func->m_body = NULL; }
            prog->m_statements.push_back(func);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, "(") == NULL) { printf("  FAIL row %u: missing '('\n", (unsigned)i); ok = false; }
            if (strstr(result, ")") == NULL) { printf("  FAIL row %u: missing ')'\n", (unsigned)i); ok = false; }
            if ((i % 2) == 0)
            {
                if (strstr(result, "{") == NULL) { printf("  FAIL row %u: missing '{'\n", (unsigned)i); ok = false; }
            }
            else
            {
                if (strstr(result, ";") == NULL) { printf("  FAIL row %u: missing ';'\n", (unsigned)i); ok = false; }
            }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 7 (method table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 8: table-driven classes (kinds x bases)
    {
        struct Case { const char* kind; const char* name; };
        static const Case kCases[] = {
            {"", "csk00"}, {"class", "csk01"},
            {"struct", "csk02"}, {"sealed class", "csk03"},
            {"static class", "csk04"}, {"abstract class", "csk05"},
            {"", "csk06"}, {"struct", "csk07"},
            {"sealed class", "csk08"}, {"class", "csk09"},
            {"static class", "csk10"}, {"abstract class", "csk11"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            CSharpFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
            cls->m_name = kCases[i].name;
            cls->m_kind = kCases[i].kind;
            IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
            sup->m_name = "BaseCs";
            cls->m_superClass = sup;
            IdentifierNode* iface = ASTNodeFactory::createIdentifierNode();
            iface->m_name = "IFaceCs";
            cls->m_implements.push_back(iface);
            cls->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(cls);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, "BaseCs") == NULL) { printf("  FAIL row %u: missing base\n", (unsigned)i); ok = false; }
            if (strstr(result, "IFaceCs") == NULL) { printf("  FAIL row %u: missing iface\n", (unsigned)i); ok = false; }
            if (strstr(result, ":") == NULL) { printf("  FAIL row %u: missing ':'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 8 (class table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 9: struct kind + generic var/params + enum
    {
        OutputBuffer out(8192);
        Config config;
        CSharpFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* st = ASTNodeFactory::createClassDeclNode();
        st->m_name = "PtCs";
        st->m_kind = "struct";
        st->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(st);
        VariableDeclNode* gv = ASTNodeFactory::createVariableDeclNode();
        gv->m_kind = "List<string>";
        gv->m_name = "names";
        prog->m_statements.push_back(gv);
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Lookup";
        func->m_returnType = "Dictionary<string,int>";
        IdentifierNode* gp = ASTNodeFactory::createIdentifierNode();
        gp->m_name = "Dictionary<string,int> map";
        func->m_params.push_back(gp);
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "CsColor";
        IdentifierNode* m1 = ASTNodeFactory::createIdentifierNode();
        m1->m_name = "CsRed";
        enm->m_members.push_back(m1);
        IdentifierNode* m2 = ASTNodeFactory::createIdentifierNode();
        m2->m_name = "CsBlue";
        enm->m_members.push_back(m2);
        prog->m_statements.push_back(enm);
        formatter.format(prog);
        const char* result = out.getData();
        printf("Test 9 output: [%s]\n", result);
        bool ok = true;
        if (strstr(result, "struct PtCs") == NULL) { printf("  FAIL: missing 'struct PtCs'\n"); ok = false; }
        if (strstr(result, "List<string>") == NULL) { printf("  FAIL: missing generic var\n"); ok = false; }
        if (strstr(result, "names") == NULL) { printf("  FAIL: missing var name\n"); ok = false; }
        if (strstr(result, "Dictionary<string,int>") == NULL) { printf("  FAIL: missing generic type\n"); ok = false; }
        if (strstr(result, "enum CsColor") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        if (strstr(result, "CsRed") == NULL) { printf("  FAIL: missing member\n"); ok = false; }
        if (strstr(result, "CsBlue") == NULL) { printf("  FAIL: missing member\n"); ok = false; }
        if (ok) { printf("Test 9 (struct/generics/enum): PASSED\n"); passed++; }
        else { printf("Test 9 (struct/generics/enum): FAILED\n"); failed++; }
    }

    // Test 10: namespace nesting + interface multi-extends + method Allman
    {
        OutputBuffer out(8192);
        Config config;
        CSharpFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "CsApp";
        BlockNode* nsBody = ASTNodeFactory::createBlockNode();
        ClassDeclNode* inner = ASTNodeFactory::createClassDeclNode();
        inner->m_name = "Inner";
        inner->m_body = ASTNodeFactory::createBlockNode();
        nsBody->m_statements.push_back(inner);
        ns->m_body = nsBody;
        prog->m_statements.push_back(ns);
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "IMulti";
        IdentifierNode* b1 = ASTNodeFactory::createIdentifierNode();
        b1->m_name = "IBase1";
        iface->m_extends.push_back(b1);
        IdentifierNode* b2 = ASTNodeFactory::createIdentifierNode();
        b2->m_name = "IBase2";
        iface->m_extends.push_back(b2);
        iface->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(iface);
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Deep";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "namespace CsApp") == NULL) { printf("  FAIL: missing ns\n"); ok = false; }
        if (strstr(result, "Inner") == NULL) { printf("  FAIL: missing inner class\n"); ok = false; }
        if (strstr(result, "public interface IMulti") == NULL) { printf("  FAIL: missing iface\n"); ok = false; }
        if (strstr(result, "IBase1") == NULL) { printf("  FAIL: missing base1\n"); ok = false; }
        if (strstr(result, "IBase2") == NULL) { printf("  FAIL: missing base2\n"); ok = false; }
        if (strstr(result, "public void Deep(") == NULL) { printf("  FAIL: missing method\n"); ok = false; }
        if (strstr(result, ")\n") == NULL) { printf("  FAIL: method brace not Allman\n"); ok = false; }
        if (ok) { printf("Test 10 (ns/iface/Allman): PASSED\n"); passed++; }
        else { printf("Test 10 (ns/iface/Allman): FAILED\n"); failed++; }
    }

    // Test 11: control flow composite + comments
    {
        OutputBuffer out(8192);
        Config config;
        CSharpFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* icond = ASTNodeFactory::createIdentifierNode();
        icond->m_name = "csflag";
        ifStmt->m_condition = icond;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        BlockNode* elseB = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = elseB;
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = true;
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "int";
        init->m_name = "qi";
        LiteralNode* z = ASTNodeFactory::createLiteralNode();
        z->m_type = LiteralNode::TYPE_NUMBER;
        z->m_value = "0";
        init->m_initializer = z;
        forStmt->m_init = init;
        IdentifierNode* fcond = ASTNodeFactory::createIdentifierNode();
        fcond->m_name = "qi < 3";
        forStmt->m_condition = fcond;
        IdentifierNode* fupd = ASTNodeFactory::createIdentifierNode();
        fupd->m_name = "qi++";
        forStmt->m_update = fupd;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);
        WhileStmtNode* whileStmt = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* wcond = ASTNodeFactory::createIdentifierNode();
        wcond->m_name = "spinning";
        whileStmt->m_condition = wcond;
        whileStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(whileStmt);
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "sel";
        sw->m_discriminant = disc;
        IdentifierNode* sc = ASTNodeFactory::createIdentifierNode();
        sc->m_name = "case 4: use4(); break;";
        sw->m_cases.push_back(sc);
        prog->m_statements.push_back(sw);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " cs-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " cs-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "if (csflag)") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "for") == NULL) { printf("  FAIL: missing for\n"); ok = false; }
        if (strstr(result, "int qi = 0") == NULL) { printf("  FAIL: missing init\n"); ok = false; }
        if (strstr(result, "qi < 3") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "qi++") == NULL) { printf("  FAIL: missing update\n"); ok = false; }
        if (strstr(result, "while (spinning)") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "switch (sel)") == NULL) { printf("  FAIL: missing switch\n"); ok = false; }
        if (strstr(result, "case 4: use4(); break;") == NULL) { printf("  FAIL: case altered\n"); ok = false; }
        if (strstr(result, "cs-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "cs-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 11 (control flow): PASSED\n"); passed++; }
        else { printf("Test 11 (control flow): FAILED\n"); failed++; }
    }

    // Test 12: empty program + verbatim + idempotency
    {
        OutputBuffer outEmpty(256);
        Config cEmpty;
        CSharpFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        if (strstr(outEmpty.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "[[[ bad c# ~~~";
        prog->m_statements.push_back(raw);
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "IdemCs";
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);
        OutputBuffer out1(4096);
        Config cc1;
        CSharpFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        CSharpFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "[[[ bad c# ~~~") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "IdemCs") == NULL) { printf("  FAIL: missing class\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 12 (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 12 (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("CSharpFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
