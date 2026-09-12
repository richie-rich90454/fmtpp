// Easter egg: C++ formatter tests - because sometimes you need more pluses
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CppFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
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

    // Test 1: C++ function with return type prefix (int main())
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_returnType = "int";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* zero = ASTNodeFactory::createLiteralNode();
        zero->m_type = LiteralNode::TYPE_NUMBER;
        zero->m_value = "0";
        ret->m_argument = zero;
        body->m_statements.push_back(ret);
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "int") == NULL) { printf("  FAIL: missing 'int'\n"); ok = false; }
        if (strstr(result, "main") == NULL) { printf("  FAIL: missing 'main'\n"); ok = false; }
        if (strstr(result, "(") == NULL) { printf("  FAIL: missing '('\n"); ok = false; }
        if (strstr(result, ")") == NULL) { printf("  FAIL: missing ')'\n"); ok = false; }
        if (strstr(result, "return 0;") == NULL) { printf("  FAIL: missing 'return 0;'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function' keyword\n"); ok = false; }
        if (ok) { printf("Test 1 (function decl): PASSED\n"); passed++; }
        else { printf("Test 1 (function decl): FAILED\n"); failed++; }
    }

    // Test 2: C++ class with public inheritance
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* superCls = ASTNodeFactory::createIdentifierNode();
        superCls->m_name = "Animal";
        cls->m_superClass = superCls;
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class") == NULL) { printf("  FAIL: missing 'class'\n"); ok = false; }
        if (strstr(result, "Dog") == NULL) { printf("  FAIL: missing 'Dog'\n"); ok = false; }
        if (strstr(result, ": public") == NULL) { printf("  FAIL: missing ': public'\n"); ok = false; }
        if (strstr(result, "Animal") == NULL) { printf("  FAIL: missing 'Animal'\n"); ok = false; }
        if (ok) { printf("Test 2 (class inheritance): PASSED\n"); passed++; }
        else { printf("Test 2 (class inheritance): FAILED\n"); failed++; }
    }

    // Test 3: C++ namespace declaration
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "std";
        BlockNode* nsBody = ASTNodeFactory::createBlockNode();
        ns->m_body = nsBody;
        prog->m_statements.push_back(ns);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "namespace") == NULL) { printf("  FAIL: missing 'namespace'\n"); ok = false; }
        if (strstr(result, "std") == NULL) { printf("  FAIL: missing 'std'\n"); ok = false; }
        if (ok) { printf("Test 3 (namespace): PASSED\n"); passed++; }
        else { printf("Test 3 (namespace): FAILED\n"); failed++; }
    }

    // Test 4: C++ function declaration without body (forward declaration)
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "helper";
        func->m_returnType = "void";
        func->m_body = NULL;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "void helper();") == NULL) { printf("  FAIL: missing 'void helper();'\n"); ok = false; }
        if (ok) { printf("Test 4 (forward decl): PASSED\n"); passed++; }
        else { printf("Test 4 (forward decl): FAILED\n"); failed++; }
    }

    // Test 5: C++ #include passthrough
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* pp = ASTNodeFactory::createPreprocessorDirectiveNode();
        pp->m_directive = "#include <iostream>";
        prog->m_statements.push_back(pp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "#include <iostream>") == NULL) { printf("  FAIL: missing '#include <iostream>'\n"); ok = false; }
        if (ok) { printf("Test 5 (include): PASSED\n"); passed++; }
        else { printf("Test 5 (include): FAILED\n"); failed++; }
    }

    // Test 6: C++ class implementing interface (comma-separated bases)
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Widget";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "Drawable";
        cls->m_implements.push_back(base);
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class Widget") == NULL) { printf("  FAIL: missing 'class Widget'\n"); ok = false; }
        if (strstr(result, "Drawable") == NULL) { printf("  FAIL: missing 'Drawable'\n"); ok = false; }
        if (ok) { printf("Test 6 (class implements): PASSED\n"); passed++; }
        else { printf("Test 6 (class implements): FAILED\n"); failed++; }
    }

    // Test 7: C++ member variable with type
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "std::string";
        varDecl->m_name = "name";
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 7 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "std::string") == NULL) { printf("  FAIL: missing 'std::string'\n"); ok = false; }
        if (strstr(result, "name") == NULL) { printf("  FAIL: missing 'name'\n"); ok = false; }
        if (ok) { printf("Test 7 (member var): PASSED\n"); passed++; }
        else { printf("Test 7 (member var): FAILED\n"); failed++; }
    }

    // Test 8: C++ struct keeps its kind (not rewritten to class)
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Point";
        cls->m_kind = "struct";
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 8 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "struct Point") == NULL) { printf("  FAIL: missing 'struct Point'\n"); ok = false; }
        if (strstr(result, "class Point") != NULL) { printf("  FAIL: struct rewritten to class\n"); ok = false; }
        if (ok) { printf("Test 8 (struct kind): PASSED\n"); passed++; }
        else { printf("Test 8 (struct kind): FAILED\n"); failed++; }
    }

    // Test 9: table-driven var decls (STL types, refs, pointers)
    {
        struct Case { const char* kind; const char* name; const char* init; };
        static const Case kCases[] = {
            {"int", "cv00", "0"}, {"std::string", "cv01", "1"},
            {"const std::string&", "cv02", "2"}, {"auto", "cv03", "3"},
            {"bool", "cv04", "4"}, {"double", "cv05", "5"},
            {"char *", "cv06", "6"}, {"unsigned", "cv07", "7"},
            {"int64_t", "cv08", "8"}, {"uint32_t", "cv09", "9"},
            {"std::vector<int>", "cv10", "10"}, {"const int", "cv11", "11"},
            {"static int", "cv12", "12"}, {"volatile int", "cv13", "13"},
            {"long long", "cv14", "14"}, {"size_t", "cv15", "15"},
            {"float", "cv16", "16"}, {"wchar_t", "cv17", "17"},
            {"int *", "cv18", "18"}, {"unsigned long", "cv19", "19"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            CppFormatter formatter(out, config);
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
            if (strstr(result, ";") == NULL) { printf("  FAIL row %u: missing ';'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 9 (var table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 10: table-driven Allman functions (brace on next line)
    {
        struct Case { const char* mods; const char* ret; const char* name; };
        static const Case kCases[] = {
            {"", "int", "pfn00"}, {"static", "int", "pfn01"},
            {"inline", "void", "pfn02"}, {"virtual", "int", "pfn03"},
            {"constexpr", "int", "pfn04"}, {"", "double", "pfn05"},
            {"static", "std::string", "pfn06"}, {"", "bool", "pfn07"},
            {"virtual", "void", "pfn08"}, {"inline", "long", "pfn09"},
            {"", "size_t", "pfn10"}, {"static", "float", "pfn11"},
            {"constexpr", "bool", "pfn12"}, {"", "char *", "pfn13"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            CppFormatter formatter(out, config);
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
            if (strstr(result, "(") == NULL) { printf("  FAIL row %u: missing '('\n", (unsigned)i); ok = false; }
            if (strstr(result, ")") == NULL) { printf("  FAIL row %u: missing ')'\n", (unsigned)i); ok = false; }
            if (strstr(result, "{") == NULL) { printf("  FAIL row %u: missing '{'\n", (unsigned)i); ok = false; }
            if (strstr(result, ")\n{") == NULL) { printf("  FAIL row %u: not Allman\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 10 (Allman table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 11: Allman differs from C same-line style
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "allman";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "void allman()") == NULL) { printf("  FAIL: missing signature\n"); ok = false; }
        if (strstr(result, ")\n{") == NULL) { printf("  FAIL: expected Allman newline brace\n"); ok = false; }
        if (strstr(result, ") {") != NULL) { printf("  FAIL: brace must not be same-line\n"); ok = false; }
        if (ok) { printf("Test 11 (Allman vs same-line): PASSED\n"); passed++; }
        else { printf("Test 11 (Allman vs same-line): FAILED\n"); failed++; }
    }

    // Test 12: nested namespaces + union kind + enum
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* outer = ASTNodeFactory::createNamespaceNode();
        outer->m_name = "outer_ns";
        BlockNode* outerBody = ASTNodeFactory::createBlockNode();
        NamespaceNode* inner = ASTNodeFactory::createNamespaceNode();
        inner->m_name = "inner_ns";
        inner->m_body = ASTNodeFactory::createBlockNode();
        outerBody->m_statements.push_back(inner);
        outer->m_body = outerBody;
        prog->m_statements.push_back(outer);
        ClassDeclNode* uni = ASTNodeFactory::createClassDeclNode();
        uni->m_name = "Word";
        uni->m_kind = "union";
        uni->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(uni);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "CppColor";
        IdentifierNode* m1 = ASTNodeFactory::createIdentifierNode();
        m1->m_name = "CC_RED";
        enm->m_members.push_back(m1);
        IdentifierNode* m2 = ASTNodeFactory::createIdentifierNode();
        m2->m_name = "CC_BLUE";
        enm->m_members.push_back(m2);
        prog->m_statements.push_back(enm);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "namespace outer_ns") == NULL) { printf("  FAIL: missing outer ns\n"); ok = false; }
        if (strstr(result, "namespace inner_ns") == NULL) { printf("  FAIL: missing inner ns\n"); ok = false; }
        if (strstr(result, "union Word") == NULL) { printf("  FAIL: missing 'union Word'\n"); ok = false; }
        if (strstr(result, "class Word") != NULL) { printf("  FAIL: union rewritten to class\n"); ok = false; }
        if (strstr(result, "enum CppColor") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        if (strstr(result, "CC_RED") == NULL) { printf("  FAIL: missing member\n"); ok = false; }
        if (strstr(result, "CC_BLUE") == NULL) { printf("  FAIL: missing member\n"); ok = false; }
        if (strstr(result, "};") == NULL) { printf("  FAIL: missing '};'\n"); ok = false; }
        if (ok) { printf("Test 12 (ns/union/enum): PASSED\n"); passed++; }
        else { printf("Test 12 (ns/union/enum): FAILED\n"); failed++; }
    }

    // Test 13: template + access specifiers passthrough
    {
        OutputBuffer out(4096);
        Config config;
        CppFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* tmpl = ASTNodeFactory::createRawTextNode();
        tmpl->m_text = "template <typename T>";
        prog->m_statements.push_back(tmpl);
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Box";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        RawTextNode* pub = ASTNodeFactory::createRawTextNode();
        pub->m_text = "public:";
        body->m_statements.push_back(pub);
        RawTextNode* prv = ASTNodeFactory::createRawTextNode();
        prv->m_text = "private:";
        body->m_statements.push_back(prv);
        cls->m_body = body;
        prog->m_statements.push_back(cls);
        FunctionDeclNode* fn = ASTNodeFactory::createFunctionDeclNode();
        fn->m_name = "getx";
        fn->m_returnType = "int";
        fn->m_modifiers = "noexcept";
        fn->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(fn);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "template <typename T>") == NULL) { printf("  FAIL: template altered\n"); ok = false; }
        if (strstr(result, "class Box") == NULL) { printf("  FAIL: missing class\n"); ok = false; }
        if (strstr(result, "public:") == NULL) { printf("  FAIL: missing 'public:'\n"); ok = false; }
        if (strstr(result, "private:") == NULL) { printf("  FAIL: missing 'private:'\n"); ok = false; }
        if (strstr(result, "noexcept") == NULL) { printf("  FAIL: missing 'noexcept'\n"); ok = false; }
        if (ok) { printf("Test 13 (template/access/noexcept): PASSED\n"); passed++; }
        else { printf("Test 13 (template/access/noexcept): FAILED\n"); failed++; }
    }

    // Test 14: control flow composite (if/for/while/switch + comments)
    {
        OutputBuffer out(8192);
        Config config;
        CppFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* icond = ASTNodeFactory::createIdentifierNode();
        icond->m_name = "cppflag";
        ifStmt->m_condition = icond;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = true;
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "int";
        init->m_name = "k";
        LiteralNode* z = ASTNodeFactory::createLiteralNode();
        z->m_type = LiteralNode::TYPE_NUMBER;
        z->m_value = "0";
        init->m_initializer = z;
        forStmt->m_init = init;
        IdentifierNode* fcond = ASTNodeFactory::createIdentifierNode();
        fcond->m_name = "k < 5";
        forStmt->m_condition = fcond;
        IdentifierNode* fupd = ASTNodeFactory::createIdentifierNode();
        fupd->m_name = "k++";
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
        disc->m_name = "kind";
        sw->m_discriminant = disc;
        IdentifierNode* sc = ASTNodeFactory::createIdentifierNode();
        sc->m_name = "case 2: go2(); break;";
        sw->m_cases.push_back(sc);
        prog->m_statements.push_back(sw);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " cpp-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " cpp-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "if (cppflag)") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "for") == NULL) { printf("  FAIL: missing for\n"); ok = false; }
        if (strstr(result, "int k = 0") == NULL) { printf("  FAIL: missing init\n"); ok = false; }
        if (strstr(result, "k < 5") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "k++") == NULL) { printf("  FAIL: missing update\n"); ok = false; }
        if (strstr(result, "while (spinning)") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "switch (kind)") == NULL) { printf("  FAIL: missing switch\n"); ok = false; }
        if (strstr(result, "case 2: go2(); break;") == NULL) { printf("  FAIL: case altered\n"); ok = false; }
        if (strstr(result, "cpp-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "cpp-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 14 (control flow): PASSED\n"); passed++; }
        else { printf("Test 14 (control flow): FAILED\n"); failed++; }
    }

    // Test 15: empty program + verbatim + idempotency
    {
        OutputBuffer outEmpty(256);
        Config cEmpty;
        CppFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        if (strstr(outEmpty.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "%%% bad c++ [[[";
        prog->m_statements.push_back(raw);
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "idemcpp";
        func->m_returnType = "int";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        OutputBuffer out1(4096);
        Config cc1;
        CppFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        CppFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "%%% bad c++ [[") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "idemcpp") == NULL) { printf("  FAIL: missing func\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 15 (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 15 (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("CppFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
