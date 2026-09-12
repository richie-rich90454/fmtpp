// Easter egg: GLSL formatter tests - vertex-tested, fragment-approved
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/GlslFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
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

    // Test 1: struct keeps kind and trailing semicolon
    {
        OutputBuffer out(4096);
        Config config;
        GlslFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Light";
        cls->m_kind = "struct";
        BlockNode* body = ASTNodeFactory::createBlockNode();
        VariableDeclNode* field = ASTNodeFactory::createVariableDeclNode();
        field->m_kind = "vec3";
        field->m_name = "position";
        body->m_statements.push_back(field);
        cls->m_body = body;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "struct Light") == NULL) { printf("  FAIL: missing 'struct Light'\n"); ok = false; }
        if (strstr(result, "vec3 position;") == NULL) { printf("  FAIL: missing 'vec3 position;'\n"); ok = false; }
        if (strstr(result, "};") == NULL) { printf("  FAIL: missing trailing '};'\n"); ok = false; }
        if (ok) { printf("Test 1 (struct): PASSED\n"); passed++; }
        else { printf("Test 1 (struct): FAILED\n"); failed++; }
    }

    // Test 2: version, precision, uniform, main
    {
        OutputBuffer out(4096);
        Config config;
        GlslFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* ver = ASTNodeFactory::createPreprocessorDirectiveNode();
        ver->m_directive = "#version 330 core";
        prog->m_statements.push_back(ver);
        VariableDeclNode* uni = ASTNodeFactory::createVariableDeclNode();
        uni->m_kind = "uniform vec3";
        uni->m_name = "color";
        prog->m_statements.push_back(uni);
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "#version 330 core") == NULL) { printf("  FAIL: missing version\n"); ok = false; }
        if (strstr(result, "uniform vec3 color;") == NULL) { printf("  FAIL: missing uniform\n"); ok = false; }
        if (strstr(result, "void main()") == NULL) { printf("  FAIL: missing 'void main()'\n"); ok = false; }
        if (strstr(result, "function") != NULL) { printf("  FAIL: unexpected 'function'\n"); ok = false; }
        if (ok) { printf("Test 2 (shader prelude): PASSED\n"); passed++; }
        else { printf("Test 2 (shader prelude): FAILED\n"); failed++; }
    }

    // Test 3: table-driven var decls (scalar/vector/matrix/storage, 16 rows)
    {
        struct Case { const char* kind; const char* name; const char* init; };
        static const Case kCases[] = {
            {"float", "gx00", "0"}, {"vec2", "gx01", "1"},
            {"vec3", "gx02", "2"}, {"vec4", "gx03", "3"},
            {"int", "gx04", "4"}, {"uint", "gx05", "5"},
            {"bool", "gx06", "6"}, {"mat2", "gx07", "7"},
            {"mat3", "gx08", "8"}, {"mat4", "gx09", "9"},
            {"sampler2D", "gx10", "10"}, {"uniform vec3", "gx11", "11"},
            {"in vec2", "gx12", "12"}, {"out vec4", "gx13", "13"},
            {"const float", "gx14", "14"}, {"highp vec3", "gx15", "15"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            GlslFormatter formatter(out, config);
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
        printf("Test 3 (var table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 4: table-driven funcs (return types x params, 10 rows)
    {
        struct Case { const char* ret; const char* name; int nparams; };
        static const Case kCases[] = {
            {"void", "gfn00", 0}, {"float", "gfn01", 1}, {"vec4", "gfn02", 2},
            {"int", "gfn03", 1}, {"mat4", "gfn04", 2}, {"vec3", "gfn05", 1},
            {"bool", "gfn06", 0}, {"vec2", "gfn07", 2}, {"void", "gfn08", 1},
            {"float", "gfn09", 3}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        static const char* kPool[] = {"vec4 col", "float t", "vec3 pos"};
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            GlslFormatter formatter(out, config);
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
            if (strstr(result, ") {") == NULL) { printf("  FAIL row %u: not same-line\n", (unsigned)i); ok = false; }
            if (strstr(result, "function") != NULL) { printf("  FAIL row %u: unexpected 'function'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 4 (func table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 5: structs table + version/precision/layout + control flow + comments
    {
        static const char* kStructs[] = {"Light", "Material", "Camera", "Vertex"};
        int tPassed = 0;
        for (int i = 0; i < 4; i++)
        {
            OutputBuffer out(4096);
            Config config;
            GlslFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
            cls->m_name = kStructs[i];
            cls->m_kind = "struct";
            BlockNode* body = ASTNodeFactory::createBlockNode();
            VariableDeclNode* field = ASTNodeFactory::createVariableDeclNode();
            field->m_kind = "vec3";
            char fname[32];
            sprintf(fname, "pos%d", i);
            field->m_name = fname;
            body->m_statements.push_back(field);
            cls->m_body = body;
            prog->m_statements.push_back(cls);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            char want[64];
            sprintf(want, "struct %s", kStructs[i]);
            if (strstr(result, want) == NULL) { printf("  FAIL row %d: missing '%s'\n", i, want); ok = false; }
            if (strstr(result, fname) == NULL) { printf("  FAIL row %d: missing field\n", i); ok = false; }
            if (strstr(result, "};") == NULL) { printf("  FAIL row %d: missing '};'\n", i); ok = false; }
            if (strstr(result, "class") != NULL) { printf("  FAIL row %d: unexpected 'class'\n", i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 5 (struct table): %d/4 passed\n", tPassed);
        OutputBuffer out(8192);
        Config config;
        GlslFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* ver = ASTNodeFactory::createPreprocessorDirectiveNode();
        ver->m_directive = "#version 450 core";
        prog->m_statements.push_back(ver);
        PreprocessorDirectiveNode* def = ASTNodeFactory::createPreprocessorDirectiveNode();
        def->m_directive = "#define PI 3.14159";
        prog->m_statements.push_back(def);
        RawTextNode* prec = ASTNodeFactory::createRawTextNode();
        prec->m_text = "precision mediump float;";
        prog->m_statements.push_back(prec);
        RawTextNode* lay = ASTNodeFactory::createRawTextNode();
        lay->m_text = "layout(location = 0) out vec4 fragColor;";
        prog->m_statements.push_back(lay);
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* ic = ASTNodeFactory::createIdentifierNode();
        ic->m_name = "lit";
        ifStmt->m_condition = ic;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = true;
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "int";
        init->m_name = "gi";
        LiteralNode* z = ASTNodeFactory::createLiteralNode();
        z->m_type = LiteralNode::TYPE_NUMBER;
        z->m_value = "0";
        init->m_initializer = z;
        forStmt->m_init = init;
        IdentifierNode* fc = ASTNodeFactory::createIdentifierNode();
        fc->m_name = "gi < 4";
        forStmt->m_condition = fc;
        IdentifierNode* fu = ASTNodeFactory::createIdentifierNode();
        fu->m_name = "gi++";
        forStmt->m_update = fu;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);
        WhileStmtNode* w = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* wc = ASTNodeFactory::createIdentifierNode();
        wc->m_name = "bounce";
        w->m_condition = wc;
        w->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(w);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " glsl-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " glsl-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "#version 450 core") == NULL) { printf("  FAIL: missing version\n"); ok = false; }
        if (strstr(result, "#define PI 3.14159") == NULL) { printf("  FAIL: missing define\n"); ok = false; }
        if (strstr(result, "precision mediump float;") == NULL) { printf("  FAIL: precision altered\n"); ok = false; }
        if (strstr(result, "layout(location = 0) out vec4 fragColor;") == NULL) { printf("  FAIL: layout altered\n"); ok = false; }
        if (strstr(result, "if (lit)") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "int gi = 0") == NULL) { printf("  FAIL: missing init\n"); ok = false; }
        if (strstr(result, "gi < 4") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "gi++") == NULL) { printf("  FAIL: missing update\n"); ok = false; }
        if (strstr(result, "while (bounce)") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "glsl-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "glsl-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 5b (shader/flow/comments): PASSED\n"); passed++; }
        else { printf("Test 5b (shader/flow/comments): FAILED\n"); failed++; }
    }

    // Test 6: empty + verbatim + idempotent
    {
        OutputBuffer outEmpty(256);
        Config cEmpty;
        GlslFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        if (strstr(outEmpty.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "@@@ bad glsl ~~~";
        prog->m_statements.push_back(raw);
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "idemgl";
        func->m_returnType = "void";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);
        OutputBuffer out1(4096);
        Config cc1;
        GlslFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        GlslFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "@@@ bad glsl ~~~") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "void idemgl()") == NULL) { printf("  FAIL: missing func\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 6 (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 6 (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("GlslFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
