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

    printf("\n========================================\n");
    printf("ZigFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
