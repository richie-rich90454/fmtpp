// Easter egg: testing the C-style formatter base - where braces meet their destiny
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CstyleFormatterBase.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"

int main()
{
    // 1. Create MemoryPool and set g_pool
    MemoryPool pool;
    g_pool = &pool;

    // 2. Create a simple AST: let x = 42;
    ProgramNode* program = ASTNodeFactory::createProgramNode();
    VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
    varDecl->m_kind = "let";
    varDecl->m_name = "x";
    LiteralNode* lit = ASTNodeFactory::createLiteralNode();
    lit->m_type = LiteralNode::TYPE_NUMBER;
    lit->m_value = "42";
    varDecl->m_initializer = lit;
    program->m_statements.push_back(varDecl);

    // 3. Create OutputBuffer, Config, CstyleFormatterBase
    OutputBuffer out(4096);
    Config config;
    CstyleFormatterBase formatter(out, config);

    // 4. Format the program
    formatter.format(program);

    // 5. Verify output
    const char* result = out.getData();
    printf("Output: [%s]\n", result);

    bool ok = true;
    if (strstr(result, "let") == NULL) { printf("FAIL: missing 'let'\n"); ok = false; }
    if (strstr(result, "x") == NULL) { printf("FAIL: missing 'x'\n"); ok = false; }
    if (strstr(result, "42") == NULL) { printf("FAIL: missing '42'\n"); ok = false; }
    if (strstr(result, ";") == NULL) { printf("FAIL: missing semicolon\n"); ok = false; }

    if (ok) { printf("Test 1 (variable decl): PASSED\n"); }
    else { printf("Test 1 (variable decl): FAILED\n"); return 1; }

    // Test 2: return statement
    OutputBuffer out2(4096);
    CstyleFormatterBase formatter2(out2, config);
    ProgramNode* prog2 = ASTNodeFactory::createProgramNode();
    ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
    LiteralNode* lit2 = ASTNodeFactory::createLiteralNode();
    lit2->m_type = LiteralNode::TYPE_NUMBER;
    lit2->m_value = "0";
    ret->m_argument = lit2;
    prog2->m_statements.push_back(ret);
    formatter2.format(prog2);
    const char* r2 = out2.getData();
    printf("Output2: [%s]\n", r2);
    ok = true;
    if (strstr(r2, "return") == NULL) { printf("FAIL: missing 'return'\n"); ok = false; }
    if (strstr(r2, "0") == NULL) { printf("FAIL: missing '0'\n"); ok = false; }
    if (ok) { printf("Test 2 (return stmt): PASSED\n"); }
    else { printf("Test 2 (return stmt): FAILED\n"); return 1; }

    // Test 3: block with statements
    OutputBuffer out3(4096);
    CstyleFormatterBase formatter3(out3, config);
    ProgramNode* prog3 = ASTNodeFactory::createProgramNode();
    BlockNode* block = ASTNodeFactory::createBlockNode();
    VariableDeclNode* vd1 = ASTNodeFactory::createVariableDeclNode();
    vd1->m_kind = "const";
    vd1->m_name = "y";
    LiteralNode* lv1 = ASTNodeFactory::createLiteralNode();
    lv1->m_type = LiteralNode::TYPE_STRING;
    lv1->m_value = "hello";
    vd1->m_initializer = lv1;
    block->m_statements.push_back(vd1);
    prog3->m_statements.push_back(block);
    formatter3.format(prog3);
    printf("Output3: [%s]\n", out3.getData());
    ok = true;
    if (strstr(out3.getData(), "{") == NULL) { printf("FAIL: missing '{'\n"); ok = false; }
    if (strstr(out3.getData(), "}") == NULL) { printf("FAIL: missing '}'\n"); ok = false; }
    if (strstr(out3.getData(), "const") == NULL) { printf("FAIL: missing 'const'\n"); ok = false; }
    if (ok) { printf("Test 3 (block): PASSED\n"); }
    else { printf("Test 3 (block): FAILED\n"); return 1; }

    // Test 4: if statement
    OutputBuffer out4(4096);
    CstyleFormatterBase formatter4(out4, config);
    ProgramNode* prog4 = ASTNodeFactory::createProgramNode();
    IfStmtNode* ifs = ASTNodeFactory::createIfStmtNode();
    IdentifierNode* cond = ASTNodeFactory::createIdentifierNode();
    cond->m_name = "flag";
    ifs->m_condition = cond;
    BlockNode* thenBlock = ASTNodeFactory::createBlockNode();
    ReturnStmtNode* retStmt = ASTNodeFactory::createReturnStmtNode();
    thenBlock->m_statements.push_back(retStmt);
    ifs->m_thenBranch = thenBlock;
    prog4->m_statements.push_back(ifs);
    formatter4.format(prog4);
    printf("Output4: [%s]\n", out4.getData());
    ok = true;
    if (strstr(out4.getData(), "if") == NULL) { printf("FAIL: missing 'if'\n"); ok = false; }
    if (strstr(out4.getData(), "flag") == NULL) { printf("FAIL: missing 'flag'\n"); ok = false; }
    if (ok) { printf("Test 4 (if stmt): PASSED\n"); }
    else { printf("Test 4 (if stmt): FAILED\n"); return 1; }

    // Test 5: function declaration
    OutputBuffer out5(4096);
    CstyleFormatterBase formatter5(out5, config);
    ProgramNode* prog5 = ASTNodeFactory::createProgramNode();
    FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
    func->m_name = "main";
    BlockNode* funcBody = ASTNodeFactory::createBlockNode();
    ReturnStmtNode* ret2 = ASTNodeFactory::createReturnStmtNode();
    LiteralNode* zero = ASTNodeFactory::createLiteralNode();
    zero->m_type = LiteralNode::TYPE_NUMBER;
    zero->m_value = "0";
    ret2->m_argument = zero;
    funcBody->m_statements.push_back(ret2);
    func->m_body = funcBody;
    prog5->m_statements.push_back(func);
    formatter5.format(prog5);
    printf("Output5: [%s]\n", out5.getData());
    ok = true;
    if (strstr(out5.getData(), "function") == NULL) { printf("FAIL: missing 'function'\n"); ok = false; }
    if (strstr(out5.getData(), "main") == NULL) { printf("FAIL: missing 'main'\n"); ok = false; }
    if (ok) { printf("Test 5 (function decl): PASSED\n"); }
    else { printf("Test 5 (function decl): FAILED\n"); return 1; }

    printf("All Phase 8 tests passed.\n");
    return 0;
}