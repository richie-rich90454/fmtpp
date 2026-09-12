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
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"

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

    printf("\n========================================\n");
    printf("RustFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
