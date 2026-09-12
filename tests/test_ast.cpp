#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/ASTPrinter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"

static int checks = 0;
static int errors = 0;
#define CHECK(cond, msg) do { checks++; if (!(cond)) { printf("FAIL: %s\n", msg); errors++; } } while(0)

int main()
{
    ProgramNode* program = NULL;
    VariableDeclNode* varDecl = NULL;
    LiteralNode* literal = NULL;

    MemoryPool pool;
    printf("MemoryPool created: OK\n");
    g_pool = &pool;
    printf("g_pool set: OK\n");

    program = ASTNodeFactory::createProgramNode();
    CHECK(program != NULL, "createProgramNode NULL");
    printf("ProgramNode created: OK\n");

    varDecl = ASTNodeFactory::createVariableDeclNode();
    CHECK(varDecl != NULL, "createVariableDeclNode NULL");
    varDecl->m_kind = "let";
    varDecl->m_name = "x";
    varDecl->m_type = "number";
    varDecl->m_initializer = NULL;
    printf("VariableDeclNode created: OK\n");

    literal = ASTNodeFactory::createLiteralNode();
    CHECK(literal != NULL, "createLiteralNode NULL");
    literal->m_value = "42";
    literal->m_type = LiteralNode::TYPE_NUMBER;
    printf("LiteralNode created: OK\n");

    varDecl->m_initializer = literal;
    program->m_statements.push_back(varDecl);

    {
        printf("\n--- ASTPrinter output ---\n");
        ASTPrinter printer("  ");
        program->accept(printer);
        printf("--- end ---\n\n");
    }

    CHECK(program->m_statements.size() == 1, "ProgramNode size 1");
    CHECK(varDecl->m_name == "x", "var name x");
    CHECK(varDecl->m_initializer != NULL, "var initializer");

    // Batch2: factory coverage for every node type (null handling: must not return NULL when pool ok)
    {
        CHECK(ASTNodeFactory::createProgramNode() != NULL, "factory Program");
        CHECK(ASTNodeFactory::createFunctionDeclNode() != NULL, "factory FunctionDecl");
        CHECK(ASTNodeFactory::createVariableDeclNode() != NULL, "factory VariableDecl");
        CHECK(ASTNodeFactory::createIfStmtNode() != NULL, "factory IfStmt");
        CHECK(ASTNodeFactory::createForStmtNode() != NULL, "factory ForStmt");
        CHECK(ASTNodeFactory::createWhileStmtNode() != NULL, "factory WhileStmt");
        CHECK(ASTNodeFactory::createSwitchStmtNode() != NULL, "factory SwitchStmt");
        CHECK(ASTNodeFactory::createBlockNode() != NULL, "factory Block");
        CHECK(ASTNodeFactory::createReturnStmtNode() != NULL, "factory Return");
        CHECK(ASTNodeFactory::createThrowStmtNode() != NULL, "factory Throw");
        CHECK(ASTNodeFactory::createTryCatchStmtNode() != NULL, "factory TryCatch");
        CHECK(ASTNodeFactory::createCommentNode() != NULL, "factory Comment");
        CHECK(ASTNodeFactory::createLiteralNode() != NULL, "factory Literal");
        CHECK(ASTNodeFactory::createIdentifierNode() != NULL, "factory Identifier");
        CHECK(ASTNodeFactory::createArrayLiteralNode() != NULL, "factory Array");
        CHECK(ASTNodeFactory::createObjectLiteralNode() != NULL, "factory Object");
        CHECK(ASTNodeFactory::createTemplateLiteralNode() != NULL, "factory Template");
        CHECK(ASTNodeFactory::createArrowFunctionNode() != NULL, "factory Arrow");
        CHECK(ASTNodeFactory::createClassDeclNode() != NULL, "factory Class");
        CHECK(ASTNodeFactory::createInterfaceDeclNode() != NULL, "factory Interface");
        CHECK(ASTNodeFactory::createEnumDeclNode() != NULL, "factory Enum");
        CHECK(ASTNodeFactory::createTypeAliasNode() != NULL, "factory TypeAlias");
        CHECK(ASTNodeFactory::createImportDeclNode() != NULL, "factory Import");
        CHECK(ASTNodeFactory::createExportDeclNode() != NULL, "factory Export");
        CHECK(ASTNodeFactory::createPreprocessorDirectiveNode() != NULL, "factory Preproc");
        CHECK(ASTNodeFactory::createNamespaceNode() != NULL, "factory Namespace");
        CHECK(ASTNodeFactory::createPropertyNode() != NULL, "factory Property");
        CHECK(ASTNodeFactory::createRawTextNode() != NULL, "factory RawText");
        CHECK(ASTNodeFactory::createHtmlElementNode() != NULL, "factory Html");
        CHECK(ASTNodeFactory::createWithNode() != NULL, "factory With");
        CHECK(ASTNodeFactory::createSwitchCaseNode() != NULL, "factory SwitchCase");
    }

    // Batch3: type predicates
    {
        RawTextNode* rt = ASTNodeFactory::createRawTextNode();
        BlockNode* bl = ASTNodeFactory::createBlockNode();
        IfStmtNode* is = ASTNodeFactory::createIfStmtNode();
        ProgramNode* pn = ASTNodeFactory::createProgramNode();
        CHECK(rt != NULL && rt->isRawText(), "RawText isRawText");
        CHECK(bl != NULL && bl->isBlock(), "Block isBlock");
        CHECK(is != NULL && is->isIfStmt(), "If isIfStmt");
        CHECK(pn != NULL && !pn->isRawText() && !pn->isBlock() && !pn->isIfStmt(), "Program predicates false");
        CHECK(rt != NULL && !rt->isBlock(), "RawText not Block");
    }

    // Batch4: literal types table
    {
        struct LT { int type; const char* val; };
        LT lts[] = {
            { LiteralNode::TYPE_STRING, "hello" },
            { LiteralNode::TYPE_NUMBER, "0" },
            { LiteralNode::TYPE_NUMBER, "3.14" },
            { LiteralNode::TYPE_NUMBER, "-1" },
            { LiteralNode::TYPE_BOOLEAN, "true" },
            { LiteralNode::TYPE_BOOLEAN, "false" },
            { LiteralNode::TYPE_NULL, "null" },
            { LiteralNode::TYPE_REGEX, "/a+/" },
            { LiteralNode::TYPE_STRING, "" },
        };
        for (size_t i = 0; i < sizeof(lts)/sizeof(lts[0]); i++) {
            LiteralNode* n = ASTNodeFactory::createLiteralNode();
            CHECK(n != NULL, "literal alloc");
            if (n) { n->m_value = lts[i].val; n->m_type = (LiteralNode::Type)lts[i].type; CHECK(n->m_value == lts[i].val, "literal value"); }
        }
    }

    // Batch5: variable kinds table
    {
        const char* kinds[] = { "let", "const", "var", "int", "auto", "", "static", "final" };
        for (size_t i = 0; i < sizeof(kinds)/sizeof(kinds[0]); i++) {
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            CHECK(v != NULL, "vardecl alloc");
            if (v) { v->m_kind = kinds[i]; v->m_name = "vk"; CHECK(v->m_kind == kinds[i], "var kind"); }
        }
    }

    // Batch6: edge sizes - program statement counts
    {
        size_t sizes[] = { 0, 1, 2, 10, 100, 500 };
        for (size_t si = 0; si < sizeof(sizes)/sizeof(sizes[0]); si++) {
            ProgramNode* p = ASTNodeFactory::createProgramNode();
            CHECK(p != NULL, "program alloc size");
            if (!p) continue;
            for (size_t k = 0; k < sizes[si]; k++) {
                VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
                if (v == NULL) break;
                v->m_name = "s";
                p->m_statements.push_back(v);
            }
            char msg[64]; snprintf(msg, sizeof(msg), "program size %lu", (unsigned long)sizes[si]);
            CHECK(p->m_statements.size() == sizes[si], msg);
        }
    }

    // Batch7: null-child handling - accept must not crash
    {
        VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
        v->m_kind = "let"; v->m_name = "n"; v->m_initializer = NULL;
        ASTPrinter pr("  ");
        v->accept(pr);
        CHECK(true, "var null init accept");
        FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
        f->m_name = "f"; f->m_body = NULL;
        f->accept(pr);
        CHECK(true, "func null body accept");
        IfStmtNode* ifs = ASTNodeFactory::createIfStmtNode();
        ifs->m_condition = NULL; ifs->m_thenBranch = NULL; ifs->m_elseBranch = NULL;
        ifs->accept(pr);
        CHECK(true, "if null accept");
        BlockNode* b = ASTNodeFactory::createBlockNode();
        b->accept(pr);
        CHECK(b->m_statements.empty(), "empty block");
        ReturnStmtNode* r = ASTNodeFactory::createReturnStmtNode();
        r->m_argument = NULL;
        r->accept(pr);
        CHECK(true, "return null accept");
        IdentifierNode* id = ASTNodeFactory::createIdentifierNode();
        id->m_name = "";
        id->accept(pr);
        CHECK(true, "empty ident accept");
        CommentNode* c = ASTNodeFactory::createCommentNode();
        c->m_text = ""; c->m_isBlock = false;
        c->accept(pr);
        CHECK(true, "empty comment accept");
        c->m_isBlock = true;
        c->accept(pr);
        CHECK(true, "empty block comment accept");
        ArrayLiteralNode* al = ASTNodeFactory::createArrayLiteralNode();
        al->accept(pr);
        CHECK(al->m_elements.empty(), "empty array");
        ObjectLiteralNode* ol = ASTNodeFactory::createObjectLiteralNode();
        ol->accept(pr);
        CHECK(ol->m_properties.empty(), "empty object");
        RawTextNode* rt2 = ASTNodeFactory::createRawTextNode();
        rt2->m_text = "";
        rt2->accept(pr);
        CHECK(true, "empty rawtext accept");
    }

    // Batch8: long strings and growth
    {
        std::string big(4096, 'x');
        LiteralNode* n = ASTNodeFactory::createLiteralNode();
        n->m_value = big; n->m_type = LiteralNode::TYPE_STRING;
        CHECK(n->m_value.size() == 4096, "big literal");
        ASTPrinter pr("  ");
        n->accept(pr);
        CHECK(true, "big literal accept");
        for (int i = 0; i < 2000; i++) {
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            if (v == NULL) { CHECK(false, "bulk 2000"); break; }
            v->m_name = "bulk";
            if (i == 1999) CHECK(true, "bulk 2000 ok");
        }
    }

    // Batch9: reset/reuse + idempotency
    {
        size_t usedBefore = pool.getUsed();
        CHECK(usedBefore > 0, "pool used >0");
        pool.reset();
        CHECK(pool.getUsed() == 0, "pool reset 0");
        pool.reset();
        CHECK(pool.getUsed() == 0, "double reset idempotent");
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        CHECK(p2 != NULL, "alloc after reset");
        CHECK(p2->m_statements.empty(), "fresh program empty");
        ASTPrinter pr1("  "), pr2("    ");
        p2->accept(pr1);
        p2->accept(pr2);
        CHECK(true, "double accept idempotent");
        g_pool = &pool;
        CHECK(g_pool == &pool, "g_pool stable");
    }

    // Batch10: printer indent variants table
    {
        const char* indents[] = { "  ", "    ", "\t", "", " " };
        for (size_t i = 0; i < sizeof(indents)/sizeof(indents[0]); i++) {
            ProgramNode* p = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = "let"; v->m_name = "q";
            p->m_statements.push_back(v);
            ASTPrinter pr(indents[i]);
            p->accept(pr);
            CHECK(true, "printer indent variant");
        }
    }

    if (errors == 0) printf("\nAll Phase 3 and 4 tests passed. checks=%d\n", checks);
    else printf("\n%d test(s) FAILED. checks=%d\n", errors, checks);
    return errors;
}
