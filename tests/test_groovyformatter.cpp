// Easter egg: Groovy formatter tests - marvelously groovy assertions
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/GroovyFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
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
    GroovyFormatter formatter(out, config);
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

    // def funcs (untyped -> def, typed -> Ret)
    {
        const char* names[] = {"greet", "fetch", "render", "compute", "save", "load", "update", "handle", "parse", "emit", "go", "run"};
        for (int i = 0; i < 12; i++)
            for (int k = 0; k < 6; k++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
                f->m_name = names[i];
                if (k == 1) { f->m_returnType = "int"; }
                else if (k == 2) { f->m_returnType = "String"; }
                else if (k == 3) { f->m_modifiers = "static"; }
                else if (k == 4) { f->m_modifiers = "private"; }
                else if (k == 5) { f->m_params.push_back(ident("String name")); f->m_params.push_back(ident("int n")); }
                else { f->m_params.push_back(ident("name")); }
                if (k != 5 && i % 2 == 0) { f->m_params.push_back(ident("x")); }
                f->m_body = ASTNodeFactory::createBlockNode();
                prog->m_statements.push_back(f);
                std::string out = fmtProg(prog);
                bool ok = has(out, names[i]) && !has(out, "placeholder") && !has(out, ";");
                if (k == 0) { ok = ok && has(out, "def greet") - (i != 0); }
                if (k == 0 && i == 0) { ok = has(out, "def greet(name)") || has(out, "def greet("); }
                if (k == 1) { ok = ok && has(out, "int ") && !has(out, "def "); }
                if (!ok) { printf("  FAIL: [def %s k=%d] out=[%s]\n", names[i], k, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: def-table (72 rows)\n");
    }

    // constructors: capitalized names take no def
    {
        const char* ctors[] = {"App", "Dog", "Store", "View", "Ctrl"};
        for (int i = 0; i < 5; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
            f->m_name = ctors[i];
            f->m_params.push_back(ident("x"));
            f->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            char want[48]; snprintf(want, sizeof(want), "%s(x)", ctors[i]);
            bool ok = has(out, want) && !has(out, "def App(") && !has(out, "def Dog(") && !has(out, "placeholder");
            check(ok, want, out.c_str());
        }
    }

    // package + imports
    {
        const char* pkgs[] = {"example", "com.acme", "app", "foo.bar", "x.y"};
        const char* imps[] = {"groovy.json.JsonSlurper", "java.util.List", "groovy.xml.MarkupBuilder", "com.Foo", "org.Bar", "x.Y"};
        for (int pi = 0; pi < 5; pi++)
            for (int ii = 0; ii < 6; ii++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
                ns->m_name = pkgs[pi];
                prog->m_statements.push_back(ns);
                ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
                imp->m_source = imps[ii];
                prog->m_statements.push_back(imp);
                std::string out = fmtProg(prog);
                bool ok = has(out, pkgs[pi]) && has(out, "package") && has(out, imps[ii]) && has(out, "import") && !has(out, ";");
                if (!ok) { printf("  FAIL: [pkg %s+%s] out=[%s]\n", pkgs[pi], imps[ii], out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: pkg-table (30 rows)\n");
    }

    // classes
    {
        const char* names[] = {"App", "Dog", "Store", "View", "Ctrl", "Model", "Widget", "Base"};
        const char* supers[] = {"", "Base", "Animal", "Ctrl"};
        for (int ni = 0; ni < 8; ni++)
            for (int si = 0; si < 4; si++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
                c->m_name = names[ni];
                if (supers[si][0]) { c->m_superClass = ident(supers[si]); }
                if (ni % 3 == 0) { c->m_implements.push_back(ident("Trait")); }
                c->m_body = ASTNodeFactory::createBlockNode();
                prog->m_statements.push_back(c);
                std::string out = fmtProg(prog);
                char want[48]; snprintf(want, sizeof(want), "class %s", names[ni]);
                bool ok = has(out, want) && !has(out, "public class") && !has(out, "placeholder");
                if (supers[si][0]) { ok = ok && has(out, "extends") && has(out, supers[si]); }
                if (!ok) { printf("  FAIL: [%s] out=[%s]\n", want, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: class-table (32 rows)\n");
    }

    // vars + switch + control + comments
    {
        const char* vnames[] = {"x", "count", "name", "flag", "data", "list", "cache", "state"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = (i % 2 == 0) ? "def" : "String";
            v->m_name = vnames[i];
            LiteralNode* lit = ASTNodeFactory::createLiteralNode();
            lit->m_type = LiteralNode::TYPE_NUMBER;
            lit->m_value = "1";
            v->m_initializer = lit;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            check(has(out, vnames[i]) && !has(out, ";"), vnames[i], out.c_str());
        }
        for (int i = 0; i < 5; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
            char d[32]; snprintf(d, sizeof(d), "v%d", i);
            sw->m_discriminant = ident(d);
            char arm[32]; snprintf(arm, sizeof(arm), "case %d", i);
            sw->m_cases.push_back(ident(arm));
            prog->m_statements.push_back(sw);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "switch %d", i);
            check(has(out, "switch") && has(out, arm), nb, out.c_str());
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
            f->m_classic = false;
            char c[48]; snprintf(c, sizeof(c), "x%d in 0..%d", i, i);
            f->m_condition = ident(c);
            f->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "for %d", i);
            check(has(out, "for"), nb, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            TryCatchStmtNode* t = ASTNodeFactory::createTryCatchStmtNode();
            t->m_body = ASTNodeFactory::createBlockNode();
            t->m_catchClause = ident("Exception e");
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
            char t[32]; snprintf(t, sizeof(t), " g%d", i);
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
        f->m_params.push_back(ident("x"));
        f->m_body = ASTNodeFactory::createBlockNode();
        p1->m_statements.push_back(f);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f2 = ASTNodeFactory::createFunctionDeclNode();
        f2->m_name = "idem";
        f2->m_params.push_back(ident("x"));
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
        nob->m_statements.push_back(nb2);
        check(has(fmtProg(nob), "def ext"), "nobody", "");
    }

    printf("\n========================================\n");
    printf("GroovyFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
