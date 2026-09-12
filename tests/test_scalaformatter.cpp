// Easter egg: Scala formatter tests - scalable assertions for scalable code
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/ScalaFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: def with return type
    {
        OutputBuffer out(4096);
        Config config;
        ScalaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "add";
        func->m_returnType = "Int";
        IdentifierNode* paramA = ASTNodeFactory::createIdentifierNode();
        paramA->m_name = "a: Int";
        func->m_params.push_back(paramA);
        BlockNode* body = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        IdentifierNode* arg = ASTNodeFactory::createIdentifierNode();
        arg->m_name = "a";
        ret->m_argument = arg;
        body->m_statements.push_back(ret);
        func->m_body = body;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "def add(a: Int): Int") == NULL) { printf("  FAIL: missing 'def add(a: Int): Int'\n"); ok = false; }
        if (strstr(result, ";") != NULL) { printf("  FAIL: unexpected semicolon\n"); ok = false; }
        if (ok) { printf("Test 1 (def): PASSED\n"); passed++; }
        else { printf("Test 1 (def): FAILED\n"); failed++; }
    }

    // Test 2: class with extends and with
    {
        OutputBuffer out(4096);
        Config config;
        ScalaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
        sup->m_name = "Animal";
        cls->m_superClass = sup;
        IdentifierNode* trait = ASTNodeFactory::createIdentifierNode();
        trait->m_name = "Pet";
        cls->m_implements.push_back(trait);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);
        ClassDeclNode* obj = ASTNodeFactory::createClassDeclNode();
        obj->m_name = "Main";
        obj->m_kind = "object";
        obj->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(obj);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "class Dog extends Animal with Pet") == NULL)
        {
            printf("  FAIL: missing 'class Dog extends Animal with Pet'\n");
            ok = false;
        }
        if (strstr(result, "object Main") == NULL) { printf("  FAIL: missing 'object Main'\n"); ok = false; }
        if (ok) { printf("Test 2 (class/object): PASSED\n"); passed++; }
        else { printf("Test 2 (class/object): FAILED\n"); failed++; }
    }

    // Test 3: match expression
    {
        OutputBuffer out(4096);
        Config config;
        ScalaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
        IdentifierNode* disc = ASTNodeFactory::createIdentifierNode();
        disc->m_name = "x";
        sw->m_discriminant = disc;
        IdentifierNode* arm = ASTNodeFactory::createIdentifierNode();
        arm->m_name = "case 1 => one()";
        sw->m_cases.push_back(arm);
        prog->m_statements.push_back(sw);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "x match") == NULL) { printf("  FAIL: missing 'x match'\n"); ok = false; }
        if (strstr(result, "case 1 => one()") == NULL) { printf("  FAIL: missing arm\n"); ok = false; }
        if (strstr(result, "switch") != NULL) { printf("  FAIL: unexpected 'switch'\n"); ok = false; }
        if (ok) { printf("Test 3 (match): PASSED\n"); passed++; }
        else { printf("Test 3 (match): FAILED\n"); failed++; }
    }

    // Test 4: package and import without semicolons
    {
        OutputBuffer out(4096);
        Config config;
        ScalaFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
        ns->m_name = "example";
        prog->m_statements.push_back(ns);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "scala.math.PI";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 4 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "package example") == NULL) { printf("  FAIL: missing package\n"); ok = false; }
        if (strstr(result, "import scala.math.PI") == NULL) { printf("  FAIL: missing import\n"); ok = false; }
        if (strstr(result, ";") != NULL) { printf("  FAIL: unexpected semicolon\n"); ok = false; }
        if (ok) { printf("Test 4 (package/import): PASSED\n"); passed++; }
        else { printf("Test 4 (package/import): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("ScalaFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
