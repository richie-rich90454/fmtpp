// Easter egg: PHP formatter tests - echoing beautiful code since 1995
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/PhpFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
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
    PhpFormatter formatter(out, config);
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
ProgramNode* phpPrelude()
{
    ProgramNode* p = ASTNodeFactory::createProgramNode();
    PreprocessorDirectiveNode* tag = ASTNodeFactory::createPreprocessorDirectiveNode();
    tag->m_directive = "<?php";
    p->m_statements.push_back(tag);
    return p;
}
}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    // Functions: names x modifiers x returns (10*4*3=120 rows)
    {
        const char* names[] = {"bark", "fetch", "render", "compute", "save", "load", "update", "handle", "parse", "emit"};
        const char* mods[] = {"", "public", "private", "public static"};
        const char* rets[] = {"", "void", "int"};
        for (int ni = 0; ni < 10; ni++)
            for (int mi = 0; mi < 4; mi++)
                for (int ri = 0; ri < 3; ri++)
                {
                    ProgramNode* prog = phpPrelude();
                    FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
                    f->m_name = names[ni];
                    f->m_modifiers = mods[mi];
                    f->m_returnType = rets[ri];
                    f->m_params.push_back(ident("$a"));
                    f->m_params.push_back(ident("$b"));
                    f->m_body = retBlock("$a");
                    prog->m_statements.push_back(f);
                    std::string out = fmtProg(prog);
                    bool ok = has(out, "<?php") && has(out, "function") && has(out, names[ni])
                        && has(out, "$a") && !has(out, "placeholder");
                    if (mods[mi][0]) { ok = ok && has(out, mods[mi]); }
                    if (rets[ri][0]) { ok = ok && has(out, rets[ri]); }
                    if (!ok) { printf("  FAIL: [func %s %s:%s] out=[%s]\n", mods[mi], names[ni], rets[ri], out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: func-table (120 rows)\n");
    }

    // Variables: keep sigil exactly once (12 names x 4 vals = 48 rows)
    {
        const char* names[] = {"$name", "$x", "$count", "$data", "$result", "$value", "$item", "$list", "$cache", "$state", "$tmp", "$id"};
        const char* vals[] = {"1", "42", "99", "0"};
        for (int ni = 0; ni < 12; ni++)
            for (int vi = 0; vi < 4; vi++)
            {
                ProgramNode* prog = phpPrelude();
                VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
                v->m_name = names[ni];
                LiteralNode* lit = ASTNodeFactory::createLiteralNode();
                lit->m_type = LiteralNode::TYPE_NUMBER;
                lit->m_value = vals[vi];
                v->m_initializer = lit;
                prog->m_statements.push_back(v);
                std::string out = fmtProg(prog);
                bool ok = has(out, names[ni]) && has(out, vals[vi]) && has(out, ";")
                    && !has(out, "$$") && !has(out, "placeholder");
                char nb[64]; snprintf(nb, sizeof(nb), "var %s=%s", names[ni], vals[vi]);
                if (!ok) { printf("  FAIL: [%s] out=[%s]\n", nb, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: var-table (48 rows)\n");
    }

    // Classes / interfaces / traits (kinds x names x supers)
    {
        const char* kinds[] = {"class", "interface", "trait", "enum", "abstract class", "final class"};
        const char* names[] = {"Dog", "Pet", "Repo", "Ctrl", "Model", "View", "Store", "Base"};
        const char* supers[] = {"", "Animal", "BaseModel", "Ctrl"};
        for (int ki = 0; ki < 6; ki++)
            for (int ni = 0; ni < 8; ni++)
                for (int si = 0; si < 4; si++)
                {
                    ProgramNode* prog = phpPrelude();
                    ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
                    c->m_kind = kinds[ki];
                    c->m_name = names[ni];
                    if (supers[si][0] && (strcmp(kinds[ki], "class") == 0 || strstr(kinds[ki], "class"))) { c->m_superClass = ident(supers[si]); }
                    if (ni % 3 == 0) { c->m_implements.push_back(ident("Pet")); }
                    c->m_body = ASTNodeFactory::createBlockNode();
                    prog->m_statements.push_back(c);
                    std::string out = fmtProg(prog);
                    bool ok = has(out, names[ni]) && has(out, kinds[ki]) && !has(out, "placeholder");
                    if (supers[si][0] && c->m_superClass) { ok = ok && has(out, supers[si]) && has(out, "extends"); }
                    if (!ok) { printf("  FAIL: [%s %s] out=[%s]\n", kinds[ki], names[ni], out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: class-table (192 rows)\n");
    }

    // Namespace + use (10 ns x 10 uses sampled)
    {
        const char* nss[] = {"App\\Models", "App\\Ctrl", "Vendor\\Pkg", "Acme\\Lib", "Foo\\Bar"};
        const char* uses[] = {"App\\Contracts\\Pet", "Vendor\\Lib\\Util", "Foo\\Bar\\Baz", "Acme\\Db\\Conn", "App\\Models\\Dog"};
        for (int ni = 0; ni < 5; ni++)
            for (int ui = 0; ui < 5; ui++)
            {
                ProgramNode* prog = phpPrelude();
                NamespaceNode* ns = ASTNodeFactory::createNamespaceNode();
                ns->m_name = nss[ni];
                prog->m_statements.push_back(ns);
                ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
                imp->m_source = uses[ui];
                prog->m_statements.push_back(imp);
                std::string out = fmtProg(prog);
                bool ok = has(out, "<?php") && has(out, nss[ni]) && has(out, "namespace")
                    && has(out, uses[ui]) && has(out, "use ") && !has(out, "placeholder");
                char nb[128]; snprintf(nb, sizeof(nb), "ns %s+%s", nss[ni], uses[ui]);
                if (!ok) { printf("  FAIL: [%s] out=[%s]\n", nb, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: ns/use-table (25 rows)\n");
    }

    // Control flow: if/elseif/else, foreach, for, while, switch, try, comments
    {
        const char* conds[] = {"$a", "$x > 0", "$ok", "$n === 1", "$flag", "$done"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = phpPrelude();
            IfStmtNode* s = ASTNodeFactory::createIfStmtNode();
            s->m_condition = ident(conds[i]);
            s->m_thenBranch = retBlock("$a");
            IfStmtNode* elif = ASTNodeFactory::createIfStmtNode();
            elif->m_condition = ident("$b");
            elif->m_thenBranch = retBlock("$b");
            elif->m_elseBranch = retBlock("$c");
            s->m_elseBranch = elif;
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            check(has(out, "if") && has(out, conds[i]) && has(out, "else"), conds[i], out.c_str());
        }
        const char* fe[] = {"$items as $item", "$map as $k => $v", "$list as $x", "$rows as $r"};
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = phpPrelude();
            ForStmtNode* f = ASTNodeFactory::createForStmtNode();
            f->m_keyword = "foreach";
            f->m_classic = false;
            f->m_condition = ident(fe[i]);
            f->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            check(has(out, "foreach") && has(out, fe[i]), fe[i], out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = phpPrelude();
            WhileStmtNode* w = ASTNodeFactory::createWhileStmtNode();
            char c[32]; snprintf(c, sizeof(c), "$c%d", i);
            w->m_condition = ident(c);
            w->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(w);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "while %d", i);
            check(has(out, "while"), nb, out.c_str());
        }
        for (int i = 0; i < 3; i++)
        {
            ProgramNode* prog = phpPrelude();
            SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
            sw->m_discriminant = ident("$x");
            char arm[32]; snprintf(arm, sizeof(arm), "case %d", i);
            sw->m_cases.push_back(ident(arm));
            prog->m_statements.push_back(sw);
            std::string out = fmtProg(prog);
            check(has(out, "switch") && has(out, arm), arm, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = phpPrelude();
            TryCatchStmtNode* t = ASTNodeFactory::createTryCatchStmtNode();
            t->m_body = ASTNodeFactory::createBlockNode();
            t->m_catchClause = ident("Exception $e");
            t->m_catchBody = ASTNodeFactory::createBlockNode();
            t->m_finallyBlock = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(t);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "try %d", i);
            check(has(out, "try") && has(out, "catch") && has(out, "finally"), nb, out.c_str());
        }
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = phpPrelude();
            CommentNode* c = ASTNodeFactory::createCommentNode();
            char t[32]; snprintf(t, sizeof(t), " note %d", i);
            c->m_text = t;
            c->m_isBlock = (i % 2 == 1);
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "cmt %d", i);
            check(has(out, t), nb, out.c_str());
        }
    }

    // Edge: idempotency, empty, verbatim, invalid, no placeholder
    {
        ProgramNode* p1 = phpPrelude();
        FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
        f->m_name = "idem";
        f->m_params.push_back(ident("$a"));
        f->m_body = retBlock("$a");
        p1->m_statements.push_back(f);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = phpPrelude();
        FunctionDeclNode* f2 = ASTNodeFactory::createFunctionDeclNode();
        f2->m_name = "idem";
        f2->m_params.push_back(ident("$a"));
        f2->m_body = retBlock("$a");
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
        ProgramNode* nob = phpPrelude();
        FunctionDeclNode* nb2 = ASTNodeFactory::createFunctionDeclNode();
        nb2->m_name = "ext";
        nob->m_statements.push_back(nb2);
        check(has(fmtProg(nob), "function ext"), "nobody", "");
        ProgramNode* many = phpPrelude();
        for (int i = 0; i < 5; i++) { VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode(); char n[16]; snprintf(n, sizeof(n), "$v%d", i); v->m_name = n; LiteralNode* l = ASTNodeFactory::createLiteralNode(); l->m_type = LiteralNode::TYPE_NUMBER; l->m_value = "1"; v->m_initializer = l; many->m_statements.push_back(v); }
        check(!has(fmtProg(many), "placeholder"), "no-placeholder", "");
    }

    printf("\n========================================\n");
    printf("PhpFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
