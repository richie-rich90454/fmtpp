// Easter egg: Rust formatter tests - where the borrow checker is always watching
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/RustFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: Rust function (fn name() -> Ret)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        func->m_returnType = "i32";
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
        if (strstr(result, "fn main()") == NULL) { printf("  FAIL: missing 'fn main()'\n"); ok = false; }
        if (strstr(result, "-> i32") == NULL) { printf("  FAIL: missing '-> i32'\n"); ok = false; }
        if (strstr(result, "return 0;") == NULL) { printf("  FAIL: missing 'return 0;'\n"); ok = false; }
        if (ok) { printf("Test 1 (fn decl): PASSED\n"); passed++; }
        else { printf("Test 1 (fn decl): FAILED\n"); failed++; }
    }

    // Test 2: Rust let binding with type (let x: i32 = 42;)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "x";
        varDecl->m_type = "i32";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "42";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "let x: i32 = 42;") == NULL) { printf("  FAIL: missing 'let x: i32 = 42;'\n"); ok = false; }
        if (ok) { printf("Test 2 (let binding): PASSED\n"); passed++; }
        else { printf("Test 2 (let binding): FAILED\n"); failed++; }
    }

    // Test 3: Rust mutable binding (let mut)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

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
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "let mut count") == NULL) { printf("  FAIL: missing 'let mut count'\n"); ok = false; }
        if (ok) { printf("Test 3 (let mut): PASSED\n"); passed++; }
        else { printf("Test 3 (let mut): FAILED\n"); failed++; }
    }

    // Test 4: Rust use declaration (use path::to::module;)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "std";
        IdentifierNode* spec = ASTNodeFactory::createIdentifierNode();
        spec->m_name = "collections";
        imp->m_specifiers.push_back(spec);
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "use std::collections;") == NULL) { printf("  FAIL: missing 'use std::collections;'\n"); ok = false; }
        if (strstr(result, "import") != NULL) { printf("  FAIL: unexpected 'import' keyword\n"); ok = false; }
        if (ok) { printf("Test 4 (use decl): PASSED\n"); passed++; }
        else { printf("Test 4 (use decl): FAILED\n"); failed++; }
    }

    // Test 5: Rust match expression (switch -> match)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "value";
        sw->m_discriminant = disc;
        IdentifierNode* arm = ASTNodeFactory::createIdentifierNode();
        arm->m_name = "_ => 0";
        sw->m_cases.push_back(arm);
        prog->m_statements.push_back(sw);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "match value") == NULL) { printf("  FAIL: missing 'match value'\n"); ok = false; }
        if (strstr(result, "_ => 0") == NULL) { printf("  FAIL: missing '_ => 0'\n"); ok = false; }
        if (strstr(result, "switch") != NULL) { printf("  FAIL: unexpected 'switch' keyword\n"); ok = false; }
        if (ok) { printf("Test 5 (match expr): PASSED\n"); passed++; }
        else { printf("Test 5 (match expr): FAILED\n"); failed++; }
    }

    // Test 6: Rust panic (throw -> panic!)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ThrowStmtNode* thr = ASTNodeFactory::createThrowStmtNode();
        LiteralNode* msg = ASTNodeFactory::createLiteralNode();
        msg->m_type = LiteralNode::TYPE_NUMBER;
        msg->m_value = "42";
        thr->m_expression = msg;
        prog->m_statements.push_back(thr);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "panic!(42);") == NULL) { printf("  FAIL: missing 'panic!(42);'\n"); ok = false; }
        if (strstr(result, "throw") != NULL) { printf("  FAIL: unexpected 'throw' keyword\n"); ok = false; }
        if (ok) { printf("Test 6 (panic): PASSED\n"); passed++; }
        else { printf("Test 6 (panic): FAILED\n"); failed++; }
    }

    // Test 7: Rust vec! macro (array literal -> vec!)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "nums";
        ArrayLiteralNode* arr = ASTNodeFactory::createArrayLiteralNode();
        LiteralNode* e1 = ASTNodeFactory::createLiteralNode();
        e1->m_type = LiteralNode::TYPE_NUMBER;
        e1->m_value = "1";
        arr->m_elements.push_back(e1);
        LiteralNode* e2 = ASTNodeFactory::createLiteralNode();
        e2->m_type = LiteralNode::TYPE_NUMBER;
        e2->m_value = "2";
        arr->m_elements.push_back(e2);
        varDecl->m_initializer = arr;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 7 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "vec![1, 2]") == NULL) { printf("  FAIL: missing 'vec![1, 2]'\n"); ok = false; }
        if (ok) { printf("Test 7 (vec! macro): PASSED\n"); passed++; }
        else { printf("Test 7 (vec! macro): FAILED\n"); failed++; }
    }

    // Test 8: Rust closure (arrow fn -> |params|)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ArrowFunctionNode* closure = ASTNodeFactory::createArrowFunctionNode();
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "a";
        closure->m_params.push_back(paramA);
        IdentifierNode* paramB = ASTNodeFactory::createIdentifierNode();
        paramB->m_name = "b";
        closure->m_params.push_back(paramB);
        closure->m_body = NULL;
        prog->m_statements.push_back(closure);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 8 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "|a, b|") == NULL) { printf("  FAIL: missing '|a, b|'\n"); ok = false; }
        if (strstr(result, "=>") != NULL) { printf("  FAIL: unexpected '=>' (arrow syntax)\n"); ok = false; }
        if (ok) { printf("Test 8 (closure): PASSED\n"); passed++; }
        else { printf("Test 8 (closure): FAILED\n"); failed++; }
    }

    // Test 9: Rust trait + impl + mod + type + enum + attribute
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();

        InterfaceDeclNode* trait = ASTNodeFactory::createInterfaceDeclNode();
        trait->m_name = "Drawable";
        BlockNode* traitBody = ASTNodeFactory::createBlockNode();
        trait->m_body = traitBody;
        prog->m_statements.push_back(trait);

        ClassDeclNode* impl = ASTNodeFactory::createClassDeclNode();
        impl->m_name = "Circle";
        BlockNode* implBody = ASTNodeFactory::createBlockNode();
        impl->m_body = implBody;
        prog->m_statements.push_back(impl);

        NamespaceNode* mod = ASTNodeFactory::createNamespaceNode();
        mod->m_name = "shapes";
        prog->m_statements.push_back(mod);

        TypeAliasNode* alias = ASTNodeFactory::createTypeAliasNode();
        alias->m_name = "Meters";
        IdentifierNode* aliased = ASTNodeFactory::createIdentifierNode();
        aliased->m_name = "f64";
        alias->m_typeAnnotation = aliased;
        prog->m_statements.push_back(alias);

        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "Color";
        IdentifierNode* variant = ASTNodeFactory::createIdentifierNode();
        variant->m_name = "Red";
        enm->m_members.push_back(variant);
        prog->m_statements.push_back(enm);

        PreprocessorDirectiveNode* attr = ASTNodeFactory::createPreprocessorDirectiveNode();
        attr->m_directive = "derive(Debug)";
        prog->m_statements.push_back(attr);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 9 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "trait Drawable") == NULL) { printf("  FAIL: missing 'trait Drawable'\n"); ok = false; }
        if (strstr(result, "impl Circle") == NULL) { printf("  FAIL: missing 'impl Circle'\n"); ok = false; }
        if (strstr(result, "mod shapes") == NULL) { printf("  FAIL: missing 'mod shapes'\n"); ok = false; }
        if (strstr(result, "type Meters = f64;") == NULL) { printf("  FAIL: missing 'type Meters = f64;'\n"); ok = false; }
        if (strstr(result, "enum Color") == NULL) { printf("  FAIL: missing 'enum Color'\n"); ok = false; }
        if (strstr(result, "Red") == NULL) { printf("  FAIL: missing 'Red'\n"); ok = false; }
        if (strstr(result, "#[derive(Debug)]") == NULL) { printf("  FAIL: missing '#[derive(Debug)]'\n"); ok = false; }
        if (ok) { printf("Test 9 (trait/impl/mod/type/enum/attr): PASSED\n"); passed++; }
        else { printf("Test 9 (trait/impl/mod/type/enum/attr): FAILED\n"); failed++; }
    }

    // Test 10: Rust None literal (null -> None)
    {
        OutputBuffer out(4096);
        Config config;
        RustFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "maybe";
        LiteralNode* noneLit = ASTNodeFactory::createLiteralNode();
        noneLit->m_type = LiteralNode::TYPE_NULL;
        noneLit->m_value = "null";
        varDecl->m_initializer = noneLit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 10 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "None") == NULL) { printf("  FAIL: missing 'None'\n"); ok = false; }
        if (ok) { printf("Test 10 (None literal): PASSED\n"); passed++; }
        else { printf("Test 10 (None literal): FAILED\n"); failed++; }
    }

    // Test 11: table-driven fns (modifiers x return types, 20 rows)
    {
        struct Case { const char* mods; const char* ret; };
        static const Case kCases[] = {
            {"", "i32"}, {"pub", "i32"}, {"pub(crate)", "bool"},
            {"async", "String"}, {"pub async", "()"}, {"", ""},
            {"pub", "Vec<u8>"}, {"", "Result<T>"}, {"pub", "&str"},
            {"", "u64"}, {"pub", "f64"}, {"async", "()"},
            {"pub(crate)", "i32"}, {"", "bool"}, {"pub", ""},
            {"", "Option<T>"}, {"pub async", "i32"}, {"", "usize"},
            {"pub", "char"}, {"async", "bool"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            RustFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            char fname[32];
            sprintf(fname, "rfn%02u", (unsigned)i);
            func->m_name = fname;
            func->m_modifiers = kCases[i].mods;
            func->m_returnType = kCases[i].ret;
            int np = (int)(i % 3);
            static const char* kParams[] = {"x: i32", "y: bool", "s: &str"};
            for (int p = 0; p < np; p++)
            {
                IdentifierNode* param = ASTNodeFactory::createIdentifierNode();
                param->m_name = kParams[p];
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
            if (strstr(result, "{") == NULL) { printf("  FAIL row %u: missing '{'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 11 (fn table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 12: table-driven bindings (let/mut/const/static, 20 rows)
    {
        struct Case { const char* kind; const char* name; const char* type; const char* init; const char* want; };
        static const Case kCases[] = {
            {"let", "rv00", "i32", "0", "let rv00"}, {"mut", "rv01", "", "1", "let mut rv01"},
            {"const", "RV02", "u32", "2", "const RV02"}, {"static", "RV03", "i32", "3", "static RV03"},
            {"let", "rv04", "String", "4", "let rv04"}, {"let mut", "rv05", "i32", "5", "let mut rv05"},
            {"let", "rv06", "bool", "6", "let rv06"}, {"mut", "rv07", "f64", "7", "let mut rv07"},
            {"const", "RV08", "bool", "8", "const RV08"}, {"static", "RV09", "u64", "9", "static RV09"},
            {"let", "rv10", "Vec<u8>", "10", "let rv10"}, {"let", "rv11", "Option<i32>", "11", "let rv11"},
            {"mut", "rv12", "String", "12", "let mut rv12"}, {"const", "RV13", "usize", "13", "const RV13"},
            {"static", "RV14", "bool", "14", "static RV14"}, {"let", "rv15", "char", "15", "let rv15"},
            {"let mut", "rv16", "u8", "16", "let mut rv16"}, {"let", "rv17", "&str", "17", "let rv17"},
            {"mut", "rv18", "", "18", "let mut rv18"}, {"let", "rv19", "f32", "19", "let rv19"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            RustFormatter formatter(out, config);
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
            if (strstr(result, kCases[i].want) == NULL) { printf("  FAIL row %u: missing '%s'\n", (unsigned)i, kCases[i].want); ok = false; }
            if (strstr(result, kCases[i].init) == NULL) { printf("  FAIL row %u: missing init\n", (unsigned)i); ok = false; }
            if (strstr(result, ";") == NULL) { printf("  FAIL row %u: missing ';'\n", (unsigned)i); ok = false; }
            if (strstr(result, "placeholder") != NULL) { printf("  FAIL row %u: placeholder leak\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 12 (binding table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 13: struct/impl-trait/lifetime/bool literals
    {
        OutputBuffer out(8192);
        Config config;
        RustFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* ps = ASTNodeFactory::createClassDeclNode();
        ps->m_name = "PubPoint";
        ps->m_kind = "pub struct";
        ps->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ps);
        ClassDeclNode* im = ASTNodeFactory::createClassDeclNode();
        im->m_name = "Circle";
        IdentifierNode* tr = ASTNodeFactory::createIdentifierNode();
        tr->m_name = "Drawable";
        im->m_superClass = tr;
        im->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(im);
        FunctionDeclNode* fn = ASTNodeFactory::createFunctionDeclNode();
        fn->m_name = "borrowed";
        fn->m_modifiers = "pub";
        fn->m_returnType = "&str";
        IdentifierNode* pa = ASTNodeFactory::createIdentifierNode();
        pa->m_name = "s: &'a str";
        fn->m_params.push_back(pa);
        fn->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(fn);
        VariableDeclNode* vb = ASTNodeFactory::createVariableDeclNode();
        vb->m_kind = "let";
        vb->m_name = "flag";
        vb->m_type = "bool";
        LiteralNode* bl = ASTNodeFactory::createLiteralNode();
        bl->m_type = LiteralNode::TYPE_BOOLEAN;
        bl->m_value = "true";
        vb->m_initializer = bl;
        prog->m_statements.push_back(vb);
        VariableDeclNode* vs = ASTNodeFactory::createVariableDeclNode();
        vs->m_kind = "let";
        vs->m_name = "label";
        LiteralNode* sl = ASTNodeFactory::createLiteralNode();
        sl->m_type = LiteralNode::TYPE_STRING;
        sl->m_value = "rust-str";
        vs->m_initializer = sl;
        prog->m_statements.push_back(vs);
        formatter.format(prog);
        const char* result = out.getData();
        printf("Test 13 output: [%s]\n", result);
        bool ok = true;
        if (strstr(result, "pub struct PubPoint") == NULL) { printf("  FAIL: missing pub struct\n"); ok = false; }
        if (strstr(result, "impl Drawable for Circle") == NULL) { printf("  FAIL: missing impl-trait\n"); ok = false; }
        if (strstr(result, "pub fn borrowed(s: &'a str) -> &str") == NULL) { printf("  FAIL: lifetime sig lost\n"); ok = false; }
        if (strstr(result, "let flag: bool = true;") == NULL) { printf("  FAIL: missing bool binding\n"); ok = false; }
        if (strstr(result, "rust-str") == NULL) { printf("  FAIL: missing string\n"); ok = false; }
        if (ok) { printf("Test 13 (struct/impl/lifetime): PASSED\n"); passed++; }
        else { printf("Test 13 (struct/impl/lifetime): FAILED\n"); failed++; }
    }

    // Test 14: control flow (if/else, for-in, while, match arms, closures)
    {
        OutputBuffer out(8192);
        Config config;
        RustFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* icond = ASTNodeFactory::createIdentifierNode();
        icond->m_name = "ready";
        ifStmt->m_condition = icond;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        IdentifierNode* finit = ASTNodeFactory::createIdentifierNode();
        finit->m_name = "item";
        forStmt->m_init = finit;
        IdentifierNode* fcond = ASTNodeFactory::createIdentifierNode();
        fcond->m_name = "items.iter()";
        forStmt->m_condition = fcond;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);
        WhileStmtNode* whileStmt = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* wcond = ASTNodeFactory::createIdentifierNode();
        wcond->m_name = "running";
        whileStmt->m_condition = wcond;
        whileStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(whileStmt);
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "shape";
        sw->m_discriminant = disc;
        IdentifierNode* a1 = ASTNodeFactory::createIdentifierNode();
        a1->m_name = "Some(x) => x";
        sw->m_cases.push_back(a1);
        IdentifierNode* a2 = ASTNodeFactory::createIdentifierNode();
        a2->m_name = "None => 0";
        sw->m_cases.push_back(a2);
        prog->m_statements.push_back(sw);
        ArrowFunctionNode* clo = ASTNodeFactory::createArrowFunctionNode();
        IdentifierNode* cp = ASTNodeFactory::createIdentifierNode();
        cp->m_name = "v: i32";
        clo->m_params.push_back(cp);
        clo->m_returnType = "i32";
        BlockNode* cloBody = ASTNodeFactory::createBlockNode();
        clo->m_body = cloBody;
        prog->m_statements.push_back(clo);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " rust-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "if ready") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "for item in items.iter()") == NULL) { printf("  FAIL: missing for-in\n"); ok = false; }
        if (strstr(result, "while running") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "match shape") == NULL) { printf("  FAIL: missing match\n"); ok = false; }
        if (strstr(result, "Some(x) => x") == NULL) { printf("  FAIL: missing arm1\n"); ok = false; }
        if (strstr(result, "None => 0") == NULL) { printf("  FAIL: missing arm2\n"); ok = false; }
        if (strstr(result, "|v: i32| -> i32") == NULL) { printf("  FAIL: missing closure\n"); ok = false; }
        if (strstr(result, "rust-line-note") == NULL) { printf("  FAIL: missing comment\n"); ok = false; }
        if (ok) { printf("Test 14 (control flow): PASSED\n"); passed++; }
        else { printf("Test 14 (control flow): FAILED\n"); failed++; }
    }

    // Test 15: use forms + enum variants + type + mod nesting + attrs
    {
        OutputBuffer out(8192);
        Config config;
        RustFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* u1 = ASTNodeFactory::createImportDeclNode();
        u1->m_source = "std";
        IdentifierNode* s1 = ASTNodeFactory::createIdentifierNode();
        s1->m_name = "fmt";
        u1->m_specifiers.push_back(s1);
        prog->m_statements.push_back(u1);
        ImportDeclNode* u2 = ASTNodeFactory::createImportDeclNode();
        u2->m_source = "crate";
        IdentifierNode* d2 = ASTNodeFactory::createIdentifierNode();
        d2->m_name = "shapes";
        u2->m_defaultImport = d2;
        IdentifierNode* s2 = ASTNodeFactory::createIdentifierNode();
        s2->m_name = "Circle";
        u2->m_specifiers.push_back(s2);
        prog->m_statements.push_back(u2);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "BigShape";
        static const char* kVars[] = {"Round", "Square", "Graph(9)", "Done"};
        for (size_t i = 0; i < 4; i++)
        {
            IdentifierNode* v = ASTNodeFactory::createIdentifierNode();
            v->m_name = kVars[i];
            enm->m_members.push_back(v);
        }
        prog->m_statements.push_back(enm);
        InterfaceDeclNode* trait = ASTNodeFactory::createInterfaceDeclNode();
        trait->m_name = "Multi";
        IdentifierNode* e1 = ASTNodeFactory::createIdentifierNode();
        e1->m_name = "Draw";
        trait->m_extends.push_back(e1);
        IdentifierNode* e2 = ASTNodeFactory::createIdentifierNode();
        e2->m_name = "Save";
        trait->m_extends.push_back(e2);
        trait->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(trait);
        NamespaceNode* mod = ASTNodeFactory::createNamespaceNode();
        mod->m_name = "outer";
        BlockNode* modBody = ASTNodeFactory::createBlockNode();
        NamespaceNode* inner = ASTNodeFactory::createNamespaceNode();
        inner->m_name = "inner";
        modBody->m_statements.push_back(inner);
        mod->m_body = modBody;
        prog->m_statements.push_back(mod);
        PreprocessorDirectiveNode* attr = ASTNodeFactory::createPreprocessorDirectiveNode();
        attr->m_directive = "allow(dead_code)";
        prog->m_statements.push_back(attr);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "use std::fmt;") == NULL) { printf("  FAIL: missing use1\n"); ok = false; }
        if (strstr(result, "use crate::shapes::{Circle}") == NULL && strstr(result, "Circle") == NULL) { printf("  FAIL: missing use2\n"); ok = false; }
        if (strstr(result, "enum BigShape") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        for (size_t i = 0; i < 4; i++)
        {
            if (strstr(result, kVars[i]) == NULL) { printf("  FAIL: missing '%s'\n", kVars[i]); ok = false; }
        }
        if (strstr(result, "trait Multi: Draw + Save") == NULL) { printf("  FAIL: missing multi-trait\n"); ok = false; }
        if (strstr(result, "mod outer") == NULL) { printf("  FAIL: missing mod\n"); ok = false; }
        if (strstr(result, "mod inner") == NULL) { printf("  FAIL: missing inner mod\n"); ok = false; }
        if (strstr(result, "#[allow(dead_code)]") == NULL) { printf("  FAIL: missing attr\n"); ok = false; }
        if (ok) { printf("Test 15 (use/enum/trait/mod): PASSED\n"); passed++; }
        else { printf("Test 15 (use/enum/trait/mod): FAILED\n"); failed++; }
    }

    // Test 16: empty vec, struct literal, empty program, verbatim, idempotent
    {
        OutputBuffer out(8192);
        Config config;
        RustFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* ve = ASTNodeFactory::createVariableDeclNode();
        ve->m_kind = "let";
        ve->m_name = "empty";
        ve->m_type = "Vec<i32>";
        ArrayLiteralNode* arr = ASTNodeFactory::createArrayLiteralNode();
        ve->m_initializer = arr;
        prog->m_statements.push_back(ve);
        VariableDeclNode* vo = ASTNodeFactory::createVariableDeclNode();
        vo->m_kind = "let";
        vo->m_name = "cfg";
        ObjectLiteralNode* obj = ASTNodeFactory::createObjectLiteralNode();
        PropertyNode* pr = ASTNodeFactory::createPropertyNode();
        IdentifierNode* k = ASTNodeFactory::createIdentifierNode();
        k->m_name = "debug";
        pr->m_key = k;
        LiteralNode* bv = ASTNodeFactory::createLiteralNode();
        bv->m_type = LiteralNode::TYPE_BOOLEAN;
        bv->m_value = "false";
        pr->m_value = bv;
        obj->m_properties.push_back(pr);
        vo->m_initializer = obj;
        prog->m_statements.push_back(vo);
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "!!! bad rust ~~~";
        prog->m_statements.push_back(raw);
        formatter.format(prog);
        const char* result = out.getData();
        OutputBuffer outB(8192);
        Config configB;
        RustFormatter formatterB(outB, configB);
        formatterB.format(prog);
        OutputBuffer outEmpty(256);
        Config cEmpty;
        RustFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (strstr(result, "vec![]") == NULL) { printf("  FAIL: missing empty vec\n"); ok = false; }
        if (strstr(result, "debug") == NULL) { printf("  FAIL: missing struct key\n"); ok = false; }
        if (strstr(result, "false") == NULL) { printf("  FAIL: missing bool\n"); ok = false; }
        if (strstr(result, "!!! bad rust ~~~") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strcmp(result, outB.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (outEmpty.getSize() != 0) { printf("  FAIL: empty not empty\n"); ok = false; }
        if (strstr(result, "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 16 (vec/literal/idempotent): PASSED\n"); passed++; }
        else { printf("Test 16 (vec/literal/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("RustFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
