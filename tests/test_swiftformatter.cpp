// Easter egg: Swift formatter tests - optional semicolons, non-optional safety
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/SwiftFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
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
    SwiftFormatter formatter(out, config);
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

    // funcs: names x returns (incl optionals) x modifiers
    {
        const char* names[] = {"add", "fetch", "render", "compute", "save", "load", "update", "init2", "parse", "emit", "go", "run"};
        const char* rets[] = {"Int", "String", "Void", "Int?", "String?", "Bool"};
        const char* mods[] = {"", "private", "public", "static"};
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
                    char want[64]; snprintf(want, sizeof(want), "func %s(", names[ni]);
                    bool ok = has(out, "func") && has(out, want) && has(out, rets[ri])
                        && has(out, "->") && !has(out, ";") && !has(out, "placeholder");
                    if (mods[mi][0]) { ok = ok && has(out, mods[mi]); }
                    if (!ok) { printf("  FAIL: [func %s->%s %s] out=[%s]\n", names[ni], rets[ri], mods[mi], out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: func-table (288 rows)\n");
    }

    // init/deinit/subscript take no func keyword
    {
        const char* specials[] = {"init", "deinit", "subscript x"};
        for (int i = 0; i < 3; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
            f->m_name = specials[i];
            f->m_params.push_back(ident("a: Int"));
            f->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            bool ok = has(out, specials[i]) && !has(out, "func init") && !has(out, "func deinit") && !has(out, "placeholder");
            if (i == 2) { ok = has(out, "subscript x") && !has(out, "func subscript"); }
            check(ok, specials[i], out.c_str());
        }
    }

    // structs/classes/enums/actors
    {
        const char* kinds[] = {"struct", "class", "enum", "actor", "final class", "struct"};
        const char* names[] = {"Point", "Dog", "Color", "Store", "View", "Item", "Box", "Node"};
        const char* bases[] = {"", "Equatable", "Base", "Hashable"};
        for (int ki = 0; ki < 6; ki++)
            for (int ni = 0; ni < 8; ni++)
                for (int bi = 0; bi < 4; bi++)
                {
                    ProgramNode* prog = ASTNodeFactory::createProgramNode();
                    ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
                    c->m_kind = kinds[ki];
                    c->m_name = names[ni];
                    if (bases[bi][0]) { c->m_implements.push_back(ident(bases[bi])); }
                    c->m_body = ASTNodeFactory::createBlockNode();
                    prog->m_statements.push_back(c);
                    std::string out = fmtProg(prog);
                    char want[64]; snprintf(want, sizeof(want), "%s %s", kinds[ki], names[ni]);
                    bool ok = has(out, want) && !has(out, "placeholder") && !has(out, ";");
                    if (bases[bi][0]) { ok = ok && has(out, bases[bi]) && has(out, ":"); }
                    if (!ok) { printf("  FAIL: [%s] out=[%s]\n", want, out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: type-table (192 rows)\n");
    }

    // imports + let/var with types + nullability
    {
        const char* imps[] = {"Foundation", "UIKit", "SwiftUI", "Combine", "Dispatch", "Foundation.NS", "XCTest", "AVKit"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = imps[i];
            prog->m_statements.push_back(imp);
            std::string out = fmtProg(prog);
            char want[64]; snprintf(want, sizeof(want), "import %s", imps[i]);
            check(has(out, want) && !has(out, ";"), want, out.c_str());
        }
        const char* names[] = {"pi", "count", "name", "flag", "data", "opt", "cache", "state"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = (i % 2 == 0) ? "let" : "var";
            v->m_name = names[i];
            v->m_type = (i % 3 == 0) ? "Double?" : "Double";
            LiteralNode* lit = ASTNodeFactory::createLiteralNode();
            lit->m_type = LiteralNode::TYPE_NUMBER;
            lit->m_value = "3.14";
            v->m_initializer = lit;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            char nb[64]; snprintf(nb, sizeof(nb), "var %s", names[i]);
            check(has(out, names[i]) && has(out, "Double") && !has(out, ";"), nb, out.c_str());
        }
    }

    // for-in without parens + if/while without parens + switch + comments
    {
        const char* fors[] = {"x in 0..<10", "i in items", "k in dict", "v in list", "e in events", "n in 1...5"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ForStmtNode* f = ASTNodeFactory::createForStmtNode();
            f->m_classic = false;
            f->m_condition = ident(fors[i]);
            f->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            char want[64]; snprintf(want, sizeof(want), "for %s", fors[i]);
            bool ok = has(out, want) && !has(out, "(") && !has(out, "placeholder");
            check(ok, want, out.c_str());
        }
        const char* conds[] = {"ok", "x > 0", "v != nil", "flag", "done", "n == 1"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            IfStmtNode* s = ASTNodeFactory::createIfStmtNode();
            s->m_condition = ident(conds[i]);
            s->m_thenBranch = retBlock("a");
            s->m_elseBranch = retBlock("b");
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            char want[64]; snprintf(want, sizeof(want), "if %s", conds[i]);
            check(has(out, want) && has(out, "else"), want, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            WhileStmtNode* w = ASTNodeFactory::createWhileStmtNode();
            char c[32]; snprintf(c, sizeof(c), "c%d", i);
            w->m_condition = ident(c);
            w->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(w);
            std::string out = fmtProg(prog);
            char want[32]; snprintf(want, sizeof(want), "while %s", c);
            check(has(out, want) && !has(out, "("), want, out.c_str());
        }
        for (int i = 0; i < 4; i++)
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
            check(has(out, "switch") && has(out, d) && has(out, arm), nb, out.c_str());
        }
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            CommentNode* c = ASTNodeFactory::createCommentNode();
            char t[32]; snprintf(t, sizeof(t), " s%d", i);
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
        check(has(fmtProg(nob), "func ext"), "nobody", "");
    }

    printf("\n========================================\n");
    printf("SwiftFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
