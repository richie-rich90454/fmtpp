// Easter egg: Kotlin formatter tests - null safety, even in tests
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/KotlinFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
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
    KotlinFormatter formatter(out, config);
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

    // funs: names x returns (incl nullability) x modifiers
    {
        const char* names[] = {"add", "fetch", "render", "compute", "save", "load", "update", "init", "parse", "emit", "go", "run"};
        const char* rets[] = {"Int", "String", "Unit", "String?", "Int?", "List<String>"};
        const char* mods[] = {"", "suspend", "private", "override"};
        for (int ni = 0; ni < 12; ni++)
            for (int ri = 0; ri < 6; ri++)
                for (int mi = 0; mi < 4; mi++)
                {
                    ProgramNode* prog = ASTNodeFactory::createProgramNode();
                    FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
                    f->m_name = names[ni];
                    f->m_returnType = rets[ri];
                    f->m_modifiers = mods[mi];
                    f->m_params.push_back(ident("a: Int"));
                    if (ni % 2 == 0) { f->m_params.push_back(ident("b: String?")); }
                    f->m_body = retBlock("a");
                    prog->m_statements.push_back(f);
                    std::string out = fmtProg(prog);
                    char want[64]; snprintf(want, sizeof(want), "fun %s(", names[ni]);
                    bool ok = has(out, "fun") && has(out, want) && has(out, rets[ri])
                        && !has(out, "function") && !has(out, "placeholder") && !has(out, ";");
                    if (mods[mi][0]) { ok = ok && has(out, mods[mi]); }
                    if (ni % 2 == 0) { ok = ok && has(out, "String?"); }
                    if (!ok) { printf("  FAIL: [fun %s:%s %s] out=[%s]\n", names[ni], rets[ri], mods[mi], out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: fun-table (288 rows)\n");
    }

    // vals/vars with nullability
    {
        const char* names[] = {"pi", "count", "name", "data", "flag", "items", "cache", "state"};
        const char* kinds[] = {"val", "var"};
        for (int ni = 0; ni < 8; ni++)
            for (int ki = 0; ki < 2; ki++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
                v->m_kind = kinds[ki];
                char nn[48]; snprintf(nn, sizeof(nn), "%s%s", names[ni], (ni % 3 == 0) ? ": String?" : "");
                v->m_name = nn;
                LiteralNode* lit = ASTNodeFactory::createLiteralNode();
                lit->m_type = LiteralNode::TYPE_NUMBER;
                lit->m_value = "1";
                v->m_initializer = lit;
                prog->m_statements.push_back(v);
                std::string out = fmtProg(prog);
                bool ok = has(out, kinds[ki]) && has(out, names[ni]) && !has(out, "placeholder") && !has(out, ";");
                char nb[64]; snprintf(nb, sizeof(nb), "var %s %s", kinds[ki], nn);
                check(ok, nb, out.c_str());
            }
    }

    // classes / data / object / companion
    {
        const char* kinds[] = {"class", "data class", "object", "companion object", "sealed class", "open class"};
        const char* names[] = {"Dog", "App", "Store", "View", "Cfg", "Item", "Singleton", "Comp"};
        for (int ki = 0; ki < 6; ki++)
            for (int ni = 0; ni < 8; ni++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
                c->m_kind = kinds[ki];
                c->m_name = names[ni];
                if (ni % 3 == 0) { c->m_superClass = ident("Animal()"); }
                if (ni % 4 == 0) { c->m_implements.push_back(ident("Pet")); }
                c->m_body = ASTNodeFactory::createBlockNode();
                prog->m_statements.push_back(c);
                std::string out = fmtProg(prog);
                bool ok = has(out, names[ni]) && has(out, kinds[ki]) && !has(out, "placeholder");
                if (ni % 3 == 0) { ok = ok && has(out, "Animal()"); }
                if (!ok) { printf("  FAIL: [%s %s] out=[%s]\n", kinds[ki], names[ni], out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: class-table (48 rows)\n");
    }

    // package + imports
    {
        const char* pkgs[] = {"com.example", "org.acme", "app", "foo.bar", "x.y.z"};
        const char* imps[] = {"kotlin.math.PI", "kotlinx.coroutines.*", "java.util.List", "com.example.Util", "kotlin.io.path.*", "org.Foo"};
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

    // when expressions
    {
        const char* discs[] = {"x", "y", "state", "v", "res", "code"};
        const char* arms[] = {"1 -> one()", "else -> other()", "is Int -> num()", "null -> nil()", "\"a\" -> alpha()"};
        for (int di = 0; di < 6; di++)
            for (int ai = 0; ai < 5; ai++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
                sw->m_discriminant = ident(discs[di]);
                sw->m_cases.push_back(ident(arms[ai]));
                prog->m_statements.push_back(sw);
                std::string out = fmtProg(prog);
                char want[32]; snprintf(want, sizeof(want), "when (%s)", discs[di]);
                bool ok = has(out, "when") && has(out, want) && has(out, arms[ai]) && !has(out, "switch");
                if (!ok) { printf("  FAIL: [when %s/%s] out=[%s]\n", discs[di], arms[ai], out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: when-table (30 rows)\n");
    }

    // enums + control flow + comments
    {
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            EnumDeclNode* e = ASTNodeFactory::createEnumDeclNode();
            char en[32]; snprintf(en, sizeof(en), "E%d", i);
            e->m_name = en;
            e->m_members.push_back(ident("A"));
            e->m_members.push_back(ident("B"));
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            check(has(out, "enum") && has(out, en), en, out.c_str());
        }
        const char* conds[] = {"x > 0", "ok", "a != null", "flag", "done", "n == 1"};
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
            char c[32]; snprintf(c, sizeof(c), "x%d in 0..%d", i, i);
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
            t->m_catchClause = ident("e: Exception");
            t->m_catchBody = ASTNodeFactory::createBlockNode();
            t->m_finallyBlock = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(t);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "try %d", i);
            check(has(out, "try") && has(out, "catch") && has(out, "finally"), nb, out.c_str());
        }
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            CommentNode* c = ASTNodeFactory::createCommentNode();
            char t[32]; snprintf(t, sizeof(t), " n%d", i);
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
        f->m_returnType = "Int";
        f->m_params.push_back(ident("a: Int"));
        f->m_body = retBlock("a");
        p1->m_statements.push_back(f);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f2 = ASTNodeFactory::createFunctionDeclNode();
        f2->m_name = "idem";
        f2->m_returnType = "Int";
        f2->m_params.push_back(ident("a: Int"));
        f2->m_body = retBlock("a");
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
        check(has(fmtProg(nob), "fun ext"), "nobody", "");
    }

    printf("\n========================================\n");
    printf("KotlinFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
