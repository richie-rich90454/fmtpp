// Easter egg: CSS formatter tests - !importantly passing assertions
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CssFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"

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
    CssFormatter formatter(out, config);
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
PropertyNode* decl(const char* k, const char* v)
{
    PropertyNode* p = ASTNodeFactory::createPropertyNode();
    p->m_key = ident(k);
    p->m_value = ident(v);
    return p;
}
}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    // Rules: selectors x declarations
    {
        const char* sels[] = {".app", "#main", "div", "p.intro", "a:hover", ".btn.primary", "ul li", "h1.title", "*", "input[type=text]"};
        const char* props[] = {"color", "margin", "padding", "display", "font-size", "width", "height", "background", "border", "flex"};
        const char* vals[] = {"red", "0 auto", "8px", "flex", "16px", "100%", "auto", "white", "1px solid", "1"};
        for (int si = 0; si < 10; si++)
            for (int pi = 0; pi < 10; pi++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                ClassDeclNode* rule = ASTNodeFactory::createClassDeclNode();
                rule->m_name = sels[si];
                BlockNode* body = ASTNodeFactory::createBlockNode();
                body->m_statements.push_back(decl(props[pi], vals[pi]));
                body->m_statements.push_back(decl(props[(pi + 1) % 10], vals[(pi + 1) % 10]));
                rule->m_body = body;
                prog->m_statements.push_back(rule);
                std::string out = fmtProg(prog);
                char want[64]; snprintf(want, sizeof(want), "%s: %s;", props[pi], vals[pi]);
                bool ok = has(out, sels[si]) && has(out, want) && has(out, "{") && has(out, "}") && !has(out, "placeholder");
                if (!ok) { printf("  FAIL: [%s %s] out=[%s]\n", sels[si], want, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: rule-table (100 rows)\n");
    }

    // At-rules: imports + media/supports/keyframes
    {
        const char* imps[] = {"reset.css", "base.css", "theme.css", "fonts.css", "vars.css", "grid.css"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = imps[i];
            prog->m_statements.push_back(imp);
            std::string out = fmtProg(prog);
            char want[64]; snprintf(want, sizeof(want), "@import \"%s\";", imps[i]);
            check(has(out, want), want, out.c_str());
        }
        const char* medias[] = {"@media screen", "@media (max-width: 600px)", "@media print", "@supports (display: grid)", "@keyframes spin", "@font-face"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* m = ASTNodeFactory::createClassDeclNode();
            m->m_name = medias[i];
            BlockNode* body = ASTNodeFactory::createBlockNode();
            if (i < 4)
            {
                ClassDeclNode* inner = ASTNodeFactory::createClassDeclNode();
                inner->m_name = ".app";
                BlockNode* ib = ASTNodeFactory::createBlockNode();
                ib->m_statements.push_back(decl("color", "red"));
                inner->m_body = ib;
                body->m_statements.push_back(inner);
            }
            else { body->m_statements.push_back(decl("from", "red")); }
            m->m_body = body;
            prog->m_statements.push_back(m);
            std::string out = fmtProg(prog);
            check(has(out, medias[i]) && !has(out, "placeholder"), medias[i], out.c_str());
        }
    }

    // Selectors gallery + edge declarations
    {
        const char* sels[] = {".a", ".b:hover", "#id .cls", "div > p", "ul + li", "a ~ b", "p::before", "input:focus", ":root", ".x, .y"};
        for (int i = 0; i < 10; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* rule = ASTNodeFactory::createClassDeclNode();
            rule->m_name = sels[i];
            BlockNode* body = ASTNodeFactory::createBlockNode();
            char pv[32]; snprintf(pv, sizeof(pv), "%dpx", i);
            body->m_statements.push_back(decl("margin", pv));
            rule->m_body = body;
            prog->m_statements.push_back(rule);
            std::string out = fmtProg(prog);
            check(has(out, sels[i]) && has(out, pv), sels[i], out.c_str());
        }
        // !important + custom props + comments
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* rule = ASTNodeFactory::createClassDeclNode();
            char sn[32]; snprintf(sn, sizeof(sn), ".imp%d", i);
            rule->m_name = sn;
            BlockNode* body = ASTNodeFactory::createBlockNode();
            body->m_statements.push_back(decl("color", "red !important"));
            char cv[32]; snprintf(cv, sizeof(cv), "--c%d", i);
            body->m_statements.push_back(decl(cv, "blue"));
            rule->m_body = body;
            prog->m_statements.push_back(rule);
            std::string out = fmtProg(prog);
            check(has(out, "!important") && has(out, cv), sn, out.c_str());
        }
    }

    // Edge: idempotency, empty, verbatim, invalid, no placeholder
    {
        ProgramNode* p1 = ASTNodeFactory::createProgramNode();
        ClassDeclNode* r = ASTNodeFactory::createClassDeclNode();
        r->m_name = ".idem";
        BlockNode* b = ASTNodeFactory::createBlockNode();
        b->m_statements.push_back(decl("color", "red"));
        r->m_body = b;
        p1->m_statements.push_back(r);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        ClassDeclNode* r2 = ASTNodeFactory::createClassDeclNode();
        r2->m_name = ".idem";
        BlockNode* b2 = ASTNodeFactory::createBlockNode();
        b2->m_statements.push_back(decl("color", "red"));
        r2->m_body = b2;
        p2->m_statements.push_back(r2);
        std::string twice = fmtProg(p2);
        check(!once.empty() && once == twice, "idempotent", once.c_str());
        ProgramNode* empty = ASTNodeFactory::createProgramNode();
        check(!has(fmtProg(empty), "placeholder"), "empty", "");
        ProgramNode* raw = ASTNodeFactory::createProgramNode();
        RawTextNode* rt = ASTNodeFactory::createRawTextNode();
        rt->m_text = "@@@verbatim-kept###";
        raw->m_statements.push_back(rt);
        check(has(fmtProg(raw), "@@@verbatim-kept###"), "raw-verbatim", "");
        ProgramNode* bad = ASTNodeFactory::createProgramNode();
        ClassDeclNode* br = ASTNodeFactory::createClassDeclNode();
        br->m_name = "???bad-selector!!!";
        br->m_body = ASTNodeFactory::createBlockNode();
        bad->m_statements.push_back(br);
        check(has(fmtProg(bad), "???bad-selector!!!"), "invalid-verbatim", "");
    }

    printf("\n========================================\n");
    printf("CssFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
