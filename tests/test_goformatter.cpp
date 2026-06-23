#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/GoFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // ============================================================
    // Test 1: Go function declaration (func keyword, no semicolons)
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "main";
        BlockNode* funcBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* zero = ASTNodeFactory::createLiteralNode();
        zero->m_type = LiteralNode::TYPE_NUMBER;
        zero->m_value = "0";
        ret->m_argument = zero;
        funcBody->m_statements.push_back(ret);
        func->m_body = funcBody;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "func") == NULL) { printf("  FAIL: missing 'func'\n"); ok = false; }
        if (strstr(result, "main") == NULL) { printf("  FAIL: missing 'main'\n"); ok = false; }
        if (strstr(result, "(") == NULL) { printf("  FAIL: missing '('\n"); ok = false; }
        if (strstr(result, ")") == NULL) { printf("  FAIL: missing ')'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (strstr(result, "}") == NULL) { printf("  FAIL: missing '}'\n"); ok = false; }
        // Go should NOT have semicolons after return
        if (strstr(result, "return 0;") != NULL) { printf("  FAIL: unexpected semicolon after return\n"); ok = false; }
        if (ok) { printf("Test 1 (Go func decl): PASSED\n"); passed++; }
        else { printf("Test 1 (Go func decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 2: Short variable declaration (:=)
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "x";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "42";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "x") == NULL) { printf("  FAIL: missing 'x'\n"); ok = false; }
        if (strstr(result, "42") == NULL) { printf("  FAIL: missing '42'\n"); ok = false; }
        if (strstr(result, ":=") == NULL) { printf("  FAIL: missing ':='\n"); ok = false; }
        // Go should NOT have "let" keyword
        if (strstr(result, "let") != NULL) { printf("  FAIL: unexpected 'let' keyword\n"); ok = false; }
        if (ok) { printf("Test 2 (short var decl): PASSED\n"); passed++; }
        else { printf("Test 2 (short var decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 3: Go package import
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "fmt";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "import") == NULL) { printf("  FAIL: missing 'import'\n"); ok = false; }
        if (strstr(result, "fmt") == NULL) { printf("  FAIL: missing 'fmt'\n"); ok = false; }
        if (strstr(result, "\"fmt\"") == NULL) { printf("  FAIL: missing \"fmt\" quotes\n"); ok = false; }
        if (ok) { printf("Test 3 (import decl): PASSED\n"); passed++; }
        else { printf("Test 3 (import decl): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 4: Go struct (class -> type struct)
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Foo";
        BlockNode* clsBody = ASTNodeFactory::createBlockNode();
        cls->m_body = clsBody;
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "type") == NULL) { printf("  FAIL: missing 'type'\n"); ok = false; }
        if (strstr(result, "struct") == NULL) { printf("  FAIL: missing 'struct'\n"); ok = false; }
        if (strstr(result, "Foo") == NULL) { printf("  FAIL: missing 'Foo'\n"); ok = false; }
        if (strstr(result, "class") != NULL) { printf("  FAIL: unexpected 'class' keyword\n"); ok = false; }
        if (ok) { printf("Test 4 (Go struct): PASSED\n"); passed++; }
        else { printf("Test 4 (Go struct): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 5: Go interface
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "Reader";
        BlockNode* ifaceBody = ASTNodeFactory::createBlockNode();
        iface->m_body = ifaceBody;
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 5 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "type") == NULL) { printf("  FAIL: missing 'type'\n"); ok = false; }
        if (strstr(result, "interface") == NULL) { printf("  FAIL: missing 'interface'\n"); ok = false; }
        if (strstr(result, "Reader") == NULL) { printf("  FAIL: missing 'Reader'\n"); ok = false; }
        if (ok) { printf("Test 5 (Go interface): PASSED\n"); passed++; }
        else { printf("Test 5 (Go interface): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 6: Go iota enum
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "Color";
        IdentifierNode* member1 = ASTNodeFactory::createIdentifierNode();
        member1->m_name = "Red";
        enm->m_members.push_back(member1);
        IdentifierNode* member2 = ASTNodeFactory::createIdentifierNode();
        member2->m_name = "Green";
        enm->m_members.push_back(member2);
        IdentifierNode* member3 = ASTNodeFactory::createIdentifierNode();
        member3->m_name = "Blue";
        enm->m_members.push_back(member3);
        prog->m_statements.push_back(enm);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 6 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "const") == NULL) { printf("  FAIL: missing 'const'\n"); ok = false; }
        if (strstr(result, "iota") == NULL) { printf("  FAIL: missing 'iota'\n"); ok = false; }
        if (strstr(result, "Color") == NULL) { printf("  FAIL: missing 'Color'\n"); ok = false; }
        if (strstr(result, "Red") == NULL) { printf("  FAIL: missing 'Red'\n"); ok = false; }
        if (strstr(result, "Green") == NULL) { printf("  FAIL: missing 'Green'\n"); ok = false; }
        if (strstr(result, "Blue") == NULL) { printf("  FAIL: missing 'Blue'\n"); ok = false; }
        if (ok) { printf("Test 6 (Go iota enum): PASSED\n"); passed++; }
        else { printf("Test 6 (Go iota enum): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 7: Go type alias
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        TypeAliasNode* alias = ASTNodeFactory::createTypeAliasNode();
        alias->m_name = "MyInt";
        IdentifierNode* typeAnnotation = ASTNodeFactory::createIdentifierNode();
        typeAnnotation->m_name = "int";
        alias->m_typeAnnotation = typeAnnotation;
        prog->m_statements.push_back(alias);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 7 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "type") == NULL) { printf("  FAIL: missing 'type'\n"); ok = false; }
        if (strstr(result, "MyInt") == NULL) { printf("  FAIL: missing 'MyInt'\n"); ok = false; }
        if (strstr(result, "int") == NULL) { printf("  FAIL: missing 'int'\n"); ok = false; }
        if (ok) { printf("Test 7 (type alias): PASSED\n"); passed++; }
        else { printf("Test 7 (type alias): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 8: Go if statement (no parens around condition)
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "x";
        ifStmt->m_condition = cond;
        BlockNode* thenBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        LiteralNode* one = ASTNodeFactory::createLiteralNode();
        one->m_type = LiteralNode::TYPE_NUMBER;
        one->m_value = "1";
        ret->m_argument = one;
        thenBody->m_statements.push_back(ret);
        ifStmt->m_thenBranch = thenBody;
        prog->m_statements.push_back(ifStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 8 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "if") == NULL) { printf("  FAIL: missing 'if'\n"); ok = false; }
        if (strstr(result, "x") == NULL) { printf("  FAIL: missing 'x'\n"); ok = false; }
        if (strstr(result, "return") == NULL) { printf("  FAIL: missing 'return'\n"); ok = false; }
        if (strstr(result, "return 1;") != NULL) { printf("  FAIL: unexpected semicolon after return\n"); ok = false; }
        if (ok) { printf("Test 8 (Go if stmt): PASSED\n"); passed++; }
        else { printf("Test 8 (Go if stmt): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 9: Go while -> for mapping
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        WhileStmtNode* whileStmt = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "ok";
        whileStmt->m_condition = cond;
        BlockNode* body = ASTNodeFactory::createBlockNode();
        whileStmt->m_body = body;
        prog->m_statements.push_back(whileStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 9 output: [%s]\n", result);

        bool ok = true;
        // Go uses "for" instead of "while"
        if (strstr(result, "for") == NULL) { printf("  FAIL: missing 'for'\n"); ok = false; }
        if (strstr(result, "while") != NULL) { printf("  FAIL: unexpected 'while' keyword\n"); ok = false; }
        if (strstr(result, "ok") == NULL) { printf("  FAIL: missing 'ok'\n"); ok = false; }
        if (ok) { printf("Test 9 (Go while->for): PASSED\n"); passed++; }
        else { printf("Test 9 (Go while->for): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 10: Go throw -> panic
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ThrowStmtNode* thr = ASTNodeFactory::createThrowStmtNode();
        LiteralNode* err = ASTNodeFactory::createLiteralNode();
        err->m_type = LiteralNode::TYPE_STRING;
        err->m_value = "oops";
        thr->m_expression = err;
        prog->m_statements.push_back(thr);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 10 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "panic") == NULL) { printf("  FAIL: missing 'panic'\n"); ok = false; }
        if (strstr(result, "oops") == NULL) { printf("  FAIL: missing 'oops'\n"); ok = false; }
        if (strstr(result, "throw") != NULL) { printf("  FAIL: unexpected 'throw' keyword\n"); ok = false; }
        if (ok) { printf("Test 10 (Go panic): PASSED\n"); passed++; }
        else { printf("Test 10 (Go panic): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 11: Go arrow function -> anonymous func
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "add";
        ArrowFunctionNode* arrow = ASTNodeFactory::createArrowFunctionNode();
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "a";
        arrow->m_params.push_back(paramA);
        IdentifierNode* paramB = ASTNodeFactory::createIdentifierNode();
        paramB->m_name = "b";
        arrow->m_params.push_back(paramB);
        BlockNode* arrowBody = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        IdentifierNode* retExpr = ASTNodeFactory::createIdentifierNode();
        retExpr->m_name = "a";
        ret->m_argument = retExpr;
        arrowBody->m_statements.push_back(ret);
        arrow->m_body = arrowBody;
        varDecl->m_initializer = arrow;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 11 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "func") == NULL) { printf("  FAIL: missing 'func'\n"); ok = false; }
        if (strstr(result, "=>") != NULL) { printf("  FAIL: unexpected '=>' (arrow syntax)\n"); ok = false; }
        if (strstr(result, "a") == NULL) { printf("  FAIL: missing 'a'\n"); ok = false; }
        if (strstr(result, "b") == NULL) { printf("  FAIL: missing 'b'\n"); ok = false; }
        if (ok) { printf("Test 11 (Go arrow->func): PASSED\n"); passed++; }
        else { printf("Test 11 (Go arrow->func): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 12: Go export (capitalized, no export keyword)
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ExportDeclNode* exp = ASTNodeFactory::createExportDeclNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "Hello";
        BlockNode* funcBody = ASTNodeFactory::createBlockNode();
        func->m_body = funcBody;
        exp->m_declaration = func;
        prog->m_statements.push_back(exp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 12 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "export") != NULL) { printf("  FAIL: unexpected 'export' keyword\n"); ok = false; }
        if (strstr(result, "func") == NULL) { printf("  FAIL: missing 'func'\n"); ok = false; }
        if (strstr(result, "Hello") == NULL) { printf("  FAIL: missing 'Hello'\n"); ok = false; }
        if (ok) { printf("Test 12 (Go export): PASSED\n"); passed++; }
        else { printf("Test 12 (Go export): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 13: Go nil literal (null -> nil)
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "p";
        LiteralNode* nilLit = ASTNodeFactory::createLiteralNode();
        nilLit->m_type = LiteralNode::TYPE_NULL;
        nilLit->m_value = "null";
        varDecl->m_initializer = nilLit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 13 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "nil") == NULL) { printf("  FAIL: missing 'nil'\n"); ok = false; }
        if (strstr(result, "null") != NULL) { printf("  FAIL: unexpected 'null' keyword\n"); ok = false; }
        if (ok) { printf("Test 13 (Go nil): PASSED\n"); passed++; }
        else { printf("Test 13 (Go nil): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 14: Go for loop with semicolons
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "let";
        init->m_name = "i";
        LiteralNode* initVal = ASTNodeFactory::createLiteralNode();
        initVal->m_type = LiteralNode::TYPE_NUMBER;
        initVal->m_value = "0";
        init->m_initializer = initVal;
        forStmt->m_init = init;
        IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
        cond->m_name = "i < 10";
        forStmt->m_condition = cond;
        VariableDeclNode* update = ASTNodeFactory::createVariableDeclNode();
        update->m_kind = "let";
        update->m_name = "i++";
        forStmt->m_update = update;
        BlockNode* body = ASTNodeFactory::createBlockNode();
        forStmt->m_body = body;
        prog->m_statements.push_back(forStmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 14 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "for") == NULL) { printf("  FAIL: missing 'for'\n"); ok = false; }
        // For loop headers should have semicolons
        if (strstr(result, ";") == NULL) { printf("  FAIL: missing semicolons in for header\n"); ok = false; }
        if (ok) { printf("Test 14 (Go for loop): PASSED\n"); passed++; }
        else { printf("Test 14 (Go for loop): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 15: Go const variable declaration
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "const";
        varDecl->m_name = "Pi";
        LiteralNode* lit = ASTNodeFactory::createLiteralNode();
        lit->m_type = LiteralNode::TYPE_NUMBER;
        lit->m_value = "3.14";
        varDecl->m_initializer = lit;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 15 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "const") == NULL) { printf("  FAIL: missing 'const'\n"); ok = false; }
        if (strstr(result, "Pi") == NULL) { printf("  FAIL: missing 'Pi'\n"); ok = false; }
        if (strstr(result, "3.14") == NULL) { printf("  FAIL: missing '3.14'\n"); ok = false; }
        if (ok) { printf("Test 15 (Go const): PASSED\n"); passed++; }
        else { printf("Test 15 (Go const): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 16: Go namespace -> package comment
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "main";
        prog->m_statements.push_back(ns);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 16 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "package") == NULL) { printf("  FAIL: missing 'package'\n"); ok = false; }
        if (strstr(result, "main") == NULL) { printf("  FAIL: missing 'main'\n"); ok = false; }
        if (strstr(result, "namespace") != NULL) { printf("  FAIL: unexpected 'namespace' keyword\n"); ok = false; }
        if (ok) { printf("Test 16 (Go namespace): PASSED\n"); passed++; }
        else { printf("Test 16 (Go namespace): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 17: Go preprocessor -> build tags
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* pp = ASTNodeFactory::createPreprocessorDirectiveNode();
        pp->m_directive = "linux";
        prog->m_statements.push_back(pp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 17 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "go:build") == NULL) { printf("  FAIL: missing 'go:build'\n"); ok = false; }
        if (strstr(result, "linux") == NULL) { printf("  FAIL: missing 'linux'\n"); ok = false; }
        if (ok) { printf("Test 17 (Go build tags): PASSED\n"); passed++; }
        else { printf("Test 17 (Go build tags): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 18: Go array literal with type syntax
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "arr";
        ArrayLiteralNode* arr = ASTNodeFactory::createArrayLiteralNode();
        LiteralNode* elem1 = ASTNodeFactory::createLiteralNode();
        elem1->m_type = LiteralNode::TYPE_NUMBER;
        elem1->m_value = "1";
        arr->m_elements.push_back(elem1);
        LiteralNode* elem2 = ASTNodeFactory::createLiteralNode();
        elem2->m_type = LiteralNode::TYPE_NUMBER;
        elem2->m_value = "2";
        arr->m_elements.push_back(elem2);
        varDecl->m_initializer = arr;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 18 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "[]") == NULL) { printf("  FAIL: missing '[]'\n"); ok = false; }
        if (strstr(result, "1") == NULL) { printf("  FAIL: missing '1'\n"); ok = false; }
        if (strstr(result, "2") == NULL) { printf("  FAIL: missing '2'\n"); ok = false; }
        if (ok) { printf("Test 18 (Go array): PASSED\n"); passed++; }
        else { printf("Test 18 (Go array): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 19: Go object/struct literal
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "let";
        varDecl->m_name = "obj";
        ObjectLiteralNode* obj = ASTNodeFactory::createObjectLiteralNode();
        PropertyNode* prop = ASTNodeFactory::createPropertyNode();
        IdentifierNode* key = ASTNodeFactory::createIdentifierNode();
        key->m_name = "Key";
        prop->m_key = key;
        LiteralNode* val = ASTNodeFactory::createLiteralNode();
        val->m_type = LiteralNode::TYPE_STRING;
        val->m_value = "value";
        prop->m_value = val;
        obj->m_properties.push_back(prop);
        varDecl->m_initializer = obj;
        prog->m_statements.push_back(varDecl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 19 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "obj") == NULL) { printf("  FAIL: missing 'obj'\n"); ok = false; }
        if (strstr(result, "{") == NULL) { printf("  FAIL: missing '{'\n"); ok = false; }
        if (strstr(result, "}") == NULL) { printf("  FAIL: missing '}'\n"); ok = false; }
        if (strstr(result, "Key") == NULL) { printf("  FAIL: missing 'Key'\n"); ok = false; }
        if (strstr(result, "value") == NULL) { printf("  FAIL: missing 'value'\n"); ok = false; }
        if (ok) { printf("Test 19 (Go struct literal): PASSED\n"); passed++; }
        else { printf("Test 19 (Go struct literal): FAILED\n"); failed++; }
    }

    // ============================================================
    // Test 20: Go comment
    // ============================================================
    {
        OutputBuffer out(4096);
        Config config;
        GoFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        CommentNode* cmt = ASTNodeFactory::createCommentNode();
        cmt->m_text = " a Go comment";
        cmt->m_isBlock = false;
        prog->m_statements.push_back(cmt);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 20 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "//") == NULL) { printf("  FAIL: missing '//'\n"); ok = false; }
        if (strstr(result, "a Go comment") == NULL) { printf("  FAIL: missing comment text\n"); ok = false; }
        if (ok) { printf("Test 20 (Go comment): PASSED\n"); passed++; }
        else { printf("Test 20 (Go comment): FAILED\n"); failed++; }
    }

    // ============================================================
    // Summary
    // ============================================================
    printf("\n========================================\n");
    printf("GoFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}