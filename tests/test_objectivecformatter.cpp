// Easter egg: Objective-C formatter tests - messages sent, tests received
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/ObjectiveCFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: @interface with superclass and protocol, ends with @end
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Dog";
        cls->m_kind = "@interface";
        IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
        sup->m_name = "NSObject";
        cls->m_superClass = sup;
        IdentifierNode* proto = ASTNodeFactory::createIdentifierNode();
        proto->m_name = "Pet";
        cls->m_implements.push_back(proto);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "@interface Dog : NSObject <Pet>") == NULL)
        {
            printf("  FAIL: missing '@interface Dog : NSObject <Pet>'\n");
            ok = false;
        }
        if (strstr(result, "@end") == NULL) { printf("  FAIL: missing '@end'\n"); ok = false; }
        if (ok) { printf("Test 1 (@interface): PASSED\n"); passed++; }
        else { printf("Test 1 (@interface): FAILED\n"); failed++; }
    }

    // Test 2: method keeps its raw signature
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "- (void)bark:(int)volume";
        func->m_body = NULL;
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "- (void)bark:(int)volume;") == NULL)
        {
            printf("  FAIL: missing method prototype\n");
            ok = false;
        }
        if (ok) { printf("Test 2 (method): PASSED\n"); passed++; }
        else { printf("Test 2 (method): FAILED\n"); failed++; }
    }

    // Test 3: @protocol with @end
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
        iface->m_name = "Pet";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "NSObject";
        iface->m_extends.push_back(base);
        iface->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(iface);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "@protocol Pet <NSObject>") == NULL)
        {
            printf("  FAIL: missing '@protocol Pet <NSObject>'\n");
            ok = false;
        }
        if (strstr(result, "@end") == NULL) { printf("  FAIL: missing '@end'\n"); ok = false; }
        if (ok) { printf("Test 3 (@protocol): PASSED\n"); passed++; }
        else { printf("Test 3 (@protocol): FAILED\n"); failed++; }
    }

    // Test 4: plain C function still works
    {
        OutputBuffer out(4096);
        Config config;
        ObjectiveCFormatter formatter(out, config);

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
        if (ok) { printf("Test 4 (C function): PASSED\n"); passed++; }
        else { printf("Test 4 (C function): FAILED\n"); failed++; }
    }

    // Test 5: table-driven methods with bodies (14 rows)
    {
        static const char* kSigs[] = {
            "- (void)ocm00", "+ (int)ocm01:(int)x", "- (NSString *)ocm02:(int)n",
            "+ (void)ocm03:(int)a b:(int)c", "- (BOOL)ocm04", "+ (id)ocm05",
            "- (void)ocm06:(NSString *)s", "+ (int)ocm07", "- (float)ocm08:(float)f",
            "+ (void)ocm09:(int)x y:(int)z", "- (int)ocm10", "+ (BOOL)ocm11:(BOOL)b",
            "- (void)ocm12:(int)n m:(int)k", "+ (NSString *)ocm13"
        };
        int tPassed = 0;
        for (int i = 0; i < 14; i++)
        {
            OutputBuffer out(4096);
            Config config;
            ObjectiveCFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            func->m_name = kSigs[i];
            func->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(func);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kSigs[i]) == NULL) { printf("  FAIL row %d: signature altered\n", i); ok = false; }
            if (strstr(result, "{") == NULL) { printf("  FAIL row %d: missing '{'\n", i); ok = false; }
            if (strstr(result, "}") == NULL) { printf("  FAIL row %d: missing '}'\n", i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 5 (method table): %d/14 passed\n", tPassed);
    }

    // Test 6: table-driven @interface/@implementation/@protocol (12 rows)
    {
        struct Case { const char* kind; const char* name; const char* base; };
        static const Case kCases[] = {
            {"@interface", "OCC00", "NSObject"}, {"@implementation", "OCC01", ""},
            {"@interface", "OCC02", "UIView"}, {"@implementation", "OCC03", ""},
            {"@interface", "OCC04", "NSObject"}, {"@implementation", "OCC05", ""},
            {"@protocol", "OCP06", "NSObject"}, {"@protocol", "OCP07", ""},
            {"@interface", "OCC08", "UIResponder"}, {"@implementation", "OCC09", ""},
            {"@protocol", "OCP10", "NSCopying"}, {"@protocol", "OCP11", ""}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            ObjectiveCFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            if (kCases[i].kind[1] == 'p')
            {
                InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
                iface->m_name = kCases[i].name;
                if (kCases[i].base[0] != '\0')
                {
                    IdentifierNode* b = ASTNodeFactory::createIdentifierNode();
                    b->m_name = kCases[i].base;
                    iface->m_extends.push_back(b);
                }
                iface->m_body = ASTNodeFactory::createBlockNode();
                prog->m_statements.push_back(iface);
            }
            else
            {
                ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
                cls->m_name = kCases[i].name;
                cls->m_kind = kCases[i].kind;
                if (kCases[i].base[0] != '\0')
                {
                    IdentifierNode* sup = ASTNodeFactory::createIdentifierNode();
                    sup->m_name = kCases[i].base;
                    cls->m_superClass = sup;
                }
                IdentifierNode* proto = ASTNodeFactory::createIdentifierNode();
                proto->m_name = "Pet";
                cls->m_implements.push_back(proto);
                cls->m_body = ASTNodeFactory::createBlockNode();
                prog->m_statements.push_back(cls);
            }
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].kind) == NULL) { printf("  FAIL row %u: missing kind\n", (unsigned)i); ok = false; }
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (kCases[i].base[0] != '\0')
            {
                if (strstr(result, kCases[i].base) == NULL) { printf("  FAIL row %u: missing base\n", (unsigned)i); ok = false; }
            }
            else
            {
                if (strstr(result, "@end") == NULL) { printf("  FAIL row %u: missing '@end'\n", (unsigned)i); ok = false; }
            }
            if (strstr(result, "@end") == NULL) { printf("  FAIL row %u: missing '@end'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 6 (decl table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 7: C funcs with params + #import + cflow + comments
    {
        OutputBuffer out(8192);
        Config config;
        ObjectiveCFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* pp = ASTNodeFactory::createPreprocessorDirectiveNode();
        pp->m_directive = "#import <Foundation/Foundation.h>";
        prog->m_statements.push_back(pp);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "UIKit/UIKit.h";
        prog->m_statements.push_back(imp);
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "ocsum";
        func->m_returnType = "int";
        IdentifierNode* p1 = ASTNodeFactory::createIdentifierNode();
        p1->m_name = "int a";
        func->m_params.push_back(p1);
        IdentifierNode* p2 = ASTNodeFactory::createIdentifierNode();
        p2->m_name = "int b";
        func->m_params.push_back(p2);
        BlockNode* body = ASTNodeFactory::createBlockNode();
        ReturnStmtNode* ret = ASTNodeFactory::createReturnStmtNode();
        IdentifierNode* re = ASTNodeFactory::createIdentifierNode();
        re->m_name = "a";
        ret->m_argument = re;
        body->m_statements.push_back(ret);
        func->m_body = body;
        prog->m_statements.push_back(func);
        VariableDeclNode* varDecl = ASTNodeFactory::createVariableDeclNode();
        varDecl->m_kind = "NSString *";
        varDecl->m_name = "title";
        LiteralNode* sl = ASTNodeFactory::createLiteralNode();
        sl->m_type = LiteralNode::TYPE_STRING;
        sl->m_value = "hello-oc";
        varDecl->m_initializer = sl;
        prog->m_statements.push_back(varDecl);
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* ic = ASTNodeFactory::createIdentifierNode();
        ic->m_name = "ocflag";
        ifStmt->m_condition = ic;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = true;
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "int";
        init->m_name = "oi";
        LiteralNode* z = ASTNodeFactory::createLiteralNode();
        z->m_type = LiteralNode::TYPE_NUMBER;
        z->m_value = "0";
        init->m_initializer = z;
        forStmt->m_init = init;
        IdentifierNode* fc = ASTNodeFactory::createIdentifierNode();
        fc->m_name = "oi < 2";
        forStmt->m_condition = fc;
        IdentifierNode* fu = ASTNodeFactory::createIdentifierNode();
        fu->m_name = "oi++";
        forStmt->m_update = fu;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);
        WhileStmtNode* w = ASTNodeFactory::createWhileStmtNode();
        IdentifierNode* wc = ASTNodeFactory::createIdentifierNode();
        wc->m_name = "ocwork";
        w->m_condition = wc;
        w->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(w);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " oc-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " oc-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        if (strstr(result, "#import <Foundation/Foundation.h>") == NULL) { printf("  FAIL: import altered\n"); ok = false; }
        if (strstr(result, "UIKit/UIKit.h") == NULL) { printf("  FAIL: missing import src\n"); ok = false; }
        if (strstr(result, "int ocsum(int a, int b)") == NULL) { printf("  FAIL: missing C func\n"); ok = false; }
        if (strstr(result, "return a;") == NULL) { printf("  FAIL: missing return\n"); ok = false; }
        if (strstr(result, "NSString *") == NULL) { printf("  FAIL: missing NSString kind\n"); ok = false; }
        if (strstr(result, "hello-oc") == NULL) { printf("  FAIL: missing string\n"); ok = false; }
        if (strstr(result, "if (ocflag)") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "int oi = 0") == NULL) { printf("  FAIL: missing init\n"); ok = false; }
        if (strstr(result, "oi < 2") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "oi++") == NULL) { printf("  FAIL: missing update\n"); ok = false; }
        if (strstr(result, "while (ocwork)") == NULL) { printf("  FAIL: missing while\n"); ok = false; }
        if (strstr(result, "oc-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "oc-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 7 (cfunc/import/flow): PASSED\n"); passed++; }
        else { printf("Test 7 (cfunc/import/flow): FAILED\n"); failed++; }
    }

    // Test 8: empty + verbatim + idempotent
    {
        OutputBuffer outEmpty(256);
        Config cEmpty;
        ObjectiveCFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        if (strstr(outEmpty.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "@@@ bad objc ~~~";
        prog->m_statements.push_back(raw);
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "IdemDog";
        cls->m_kind = "@implementation";
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);
        OutputBuffer out1(4096);
        Config cc1;
        ObjectiveCFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        ObjectiveCFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "@@@ bad objc ~~~") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "@implementation IdemDog") == NULL) { printf("  FAIL: missing impl\n"); ok = false; }
        if (strstr(out1.getData(), "@end") == NULL) { printf("  FAIL: missing '@end'\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 8 (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 8 (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("ObjectiveCFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
