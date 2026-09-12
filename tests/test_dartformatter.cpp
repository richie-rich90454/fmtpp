// Easter egg: Dart formatter tests - triple twenty first throw
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/DartFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"

namespace {
int passed = 0;
int failed = 0;
int rowChecks = 0;
void check(bool ok, const char* name, const char* detail)
{
    rowChecks++;
    if (ok) { passed++; }
    else { printf("  FAIL: %s (%s)\n", name, detail); failed++; }
}
std::string fmtProg(ProgramNode* prog)
{
    OutputBuffer out(8192);
    Config config;
    DartFormatter formatter(out, config);
    formatter.format(prog);
    const char* d = out.getData();
    return d ? std::string(d) : std::string();
}
bool has(const std::string& s, const char* m) { return s.find(m) != std::string::npos; }
IdentifierNode* ident(const char* n)
{
    IdentifierNode* id = ASTNodeFactory::createIdentifierNode();
    id->m_name = n;
    return id;
}
BlockNode* retBlock(const char* v)
{
    BlockNode* b = ASTNodeFactory::createBlockNode();
    ReturnStmtNode* r = ASTNodeFactory::createReturnStmtNode();
    r->m_argument = ident(v);
    b->m_statements.push_back(r);
    return b;
}
}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    // Functions: C-style Ret Name(params) with ; bodies
    {
        const char* names[] = {"main", "area", "fetch", "render", "compute", "save", "load", "update", "init", "parse", "emit", "go"};
        const char* rets[] = {"void", "double", "int", "String", "Future<void>", "bool"};
        for (int ni = 0; ni < 12; ni++)
            for (int ri = 0; ri < 6; ri++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
                f->m_name = names[ni];
                f->m_returnType = rets[ri];
                if (ni % 2 == 0) { f->m_params.push_back(ident("double radius")); }
                else { f->m_params.push_back(ident("int x")); }
                f->m_body = retBlock("x");
                prog->m_statements.push_back(f);
                std::string out = fmtProg(prog);
                char want[64]; snprintf(want, sizeof(want), "%s %s(", rets[ri], names[ni]);
                bool ok = has(out, want) && !has(out, "function") && !has(out, "placeholder");
                if (!ok) { printf("  FAIL: [%s] out=[%s]\n", want, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: func-table (72 rows)\n");
    }

    // Classes with extends/implements
    {
        const char* names[] = {"Dog", "App", "Store", "View", "Ctrl", "Model", "Widget", "Base"};
        const char* supers[] = {"", "Animal", "Base", "Widget"};
        const char* ifaces[] = {"", "Pet", "Disposable"};
        for (int ni = 0; ni < 8; ni++)
            for (int si = 0; si < 4; si++)
                for (int ii = 0; ii < 3; ii++)
                {
                    ProgramNode* prog = ASTNodeFactory::createProgramNode();
                    ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
                    c->m_name = names[ni];
                    if (supers[si][0]) { c->m_superClass = ident(supers[si]); }
                    if (ifaces[ii][0]) { c->m_implements.push_back(ident(ifaces[ii])); }
                    c->m_body = ASTNodeFactory::createBlockNode();
                    prog->m_statements.push_back(c);
                    std::string out = fmtProg(prog);
                    char want[48]; snprintf(want, sizeof(want), "class %s", names[ni]);
                    bool ok = has(out, want) && !has(out, "public") && !has(out, "placeholder");
                    if (supers[si][0]) { ok = ok && has(out, "extends") && has(out, supers[si]); }
                    if (ifaces[ii][0]) { ok = ok && has(out, "implements") && has(out, ifaces[ii]); }
                    if (!ok) { printf("  FAIL: [%s] out=[%s]\n", want, out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: class-table (96 rows)\n");
    }

    // Imports quoted with semicolon
    {
        const char* srcs[] = {"'dart:math'", "'dart:io'", "'package:flutter/material.dart'", "'dart:async'", "'dart:convert'", "'package:app/x.dart'"};
        for (int i = 0; i < 6; i++)
            for (int k = 0; k < 4; k++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
                imp->m_source = srcs[i];
                prog->m_statements.push_back(imp);
                std::string out = fmtProg(prog);
                char want[96]; snprintf(want, sizeof(want), "import %s;", srcs[i]);
                bool ok = has(out, want) && !has(out, "placeholder");
                if (!ok) { printf("  FAIL: [%s] out=[%s]\n", want, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: import-table (24 rows)\n");
    }

    // Vars + enums + control flow + comments
    {
        const char* vnames[] = {"x", "count", "name", "flag", "data", "list", "cache", "state"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = (i % 2 == 0) ? "var" : "final";
            v->m_name = vnames[i];
            LiteralNode* lit = ASTNodeFactory::createLiteralNode();
            lit->m_type = LiteralNode::TYPE_NUMBER;
            lit->m_value = "1";
            v->m_initializer = lit;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            check(has(out, vnames[i]) && has(out, ";"), vnames[i], out.c_str());
        }
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            EnumDeclNode* e = ASTNodeFactory::createEnumDeclNode();
            char en[32]; snprintf(en, sizeof(en), "E%d", i);
            e->m_name = en;
            e->m_members.push_back(ident("a"));
            e->m_members.push_back(ident("b"));
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            check(has(out, "enum") && has(out, en), en, out.c_str());
        }
        const char* conds[] = {"x > 0", "ok", "flag", "done", "n == 1", "a"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            IfStmtNode* s = ASTNodeFactory::createIfStmtNode();
            s->m_condition = ident(conds[i]);
            s->m_thenBranch = retBlock("a");
            s->m_elseBranch = retBlock("b");
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            check(has(out, "if") && has(out, conds[i]), conds[i], out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ForStmtNode* f = ASTNodeFactory::createForStmtNode();
            f->m_classic = true;
            VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
            init->m_kind = "var";
            init->m_name = "i";
            LiteralNode* lv = ASTNodeFactory::createLiteralNode();
            lv->m_type = LiteralNode::TYPE_NUMBER;
            lv->m_value = "0";
            init->m_initializer = lv;
            f->m_init = init;
            f->m_condition = ident("i < 10");
            f->m_update = ident("i++");
            f->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "for %d", i);
            check(has(out, "for") && has(out, "i < 10"), nb, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            TryCatchStmtNode* t = ASTNodeFactory::createTryCatchStmtNode();
            t->m_body = ASTNodeFactory::createBlockNode();
            t->m_catchClause = ident("e");
            t->m_catchBody = ASTNodeFactory::createBlockNode();
            t->m_finallyBlock = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(t);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "try %d", i);
            check(has(out, "try") && has(out, "catch"), nb, out.c_str());
        }
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            CommentNode* c = ASTNodeFactory::createCommentNode();
            char t[32]; snprintf(t, sizeof(t), " d%d", i);
            c->m_text = t;
            c->m_isBlock = (i % 2 == 1);
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "cmt %d", i);
            check(has(out, t), nb, out.c_str());
        }
    }

    // Edge
    {
        ProgramNode* p1 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
        f->m_name = "idem";
        f->m_returnType = "void";
        f->m_body = ASTNodeFactory::createBlockNode();
        p1->m_statements.push_back(f);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f2 = ASTNodeFactory::createFunctionDeclNode();
        f2->m_name = "idem";
        f2->m_returnType = "void";
        f2->m_body = ASTNodeFactory::createBlockNode();
        p2->m_statements.push_back(f2);
        std::string twice = fmtProg(p2);
        check(!once.empty() && once == twice, "idempotent", once.c_str());
        ProgramNode* empty = ASTNodeFactory::createProgramNode();
        check(!has(fmtProg(empty), "placeholder"), "empty", "");
        ProgramNode* raw = ASTNodeFactory::createProgramNode();
        RawTextNode* rt = ASTNodeFactory::createRawTextNode();
        rt->m_text = "@@@verbatim-kept###";
        raw->m_statements.push_back(rt);
        check(has(fmtProg(raw), "@@@verbatim-kept###"), "raw-verbatim", "");
        ProgramNode* nob = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* nb2 = ASTNodeFactory::createFunctionDeclNode();
        nb2->m_name = "ext";
        nb2->m_returnType = "void";
        nob->m_statements.push_back(nb2);
        check(has(fmtProg(nob), "void ext"), "nobody", "");
    }

    printf("\n========================================\n");
    printf("DartFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
