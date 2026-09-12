// Easter egg: Vue formatter tests - reactively passing, progressively true
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/VueFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/HtmlElementNode.hpp"

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
    VueFormatter formatter(out, config);
    formatter.format(prog);
    const char* d = out.getData();
    return d ? std::string(d) : std::string();
}
bool has(const std::string& s, const char* m) { return s.find(m) != std::string::npos; }
RawTextNode* txt(const char* t)
{
    RawTextNode* r = ASTNodeFactory::createRawTextNode();
    r->m_text = t;
    return r;
}
HtmlElementNode* el(const char* tag)
{
    HtmlElementNode* e = ASTNodeFactory::createHtmlElementNode();
    e->m_tag = tag;
    return e;
}
}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    // Template blocks (tags x attrs x text)
    {
        const char* tags[] = {"p", "div", "span", "button", "ul", "li", "h1", "section"};
        const char* texts[] = {"hi", "hello", "click", "item", "title", "body", "x", "y"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* tpl = el("template");
            HtmlElementNode* e = el(tags[i]);
            if (i % 2 == 0) { e->m_attrs.push_back("class=\"app\""); }
            if (i % 3 == 0) { e->m_attrs.push_back("v-if=\"ok\""); }
            e->m_children.push_back(txt(texts[i]));
            tpl->m_children.push_back(e);
            prog->m_statements.push_back(tpl);
            std::string out = fmtProg(prog);
            char nb[64]; snprintf(nb, sizeof(nb), "tpl %s", tags[i]);
            check(has(out, "<template>") && has(out, texts[i]) && has(out, "</template>") && !has(out, "placeholder"), nb, out.c_str());
        }
        // directives gallery
        const char* dirs[] = {"v-if=\"ok\"", "v-for=\"x in list\"", "v-model=\"val\"", ":prop=\"x\"", "@click=\"go\"", "v-show=\"vis\""};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* tpl = el("template");
            HtmlElementNode* e = el("div");
            e->m_attrs.push_back(dirs[i]);
            e->m_children.push_back(txt("d"));
            tpl->m_children.push_back(e);
            prog->m_statements.push_back(tpl);
            std::string out = fmtProg(prog);
            check(has(out, dirs[i]), dirs[i], out.c_str());
        }
        // nesting
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* tpl = el("template");
            HtmlElementNode* outer = el("div");
            outer->m_attrs.push_back("class=\"wrap\"");
            HtmlElementNode* inner = el("p");
            char t[32]; snprintf(t, sizeof(t), "leaf%d", i);
            inner->m_children.push_back(txt(t));
            outer->m_children.push_back(inner);
            tpl->m_children.push_back(outer);
            prog->m_statements.push_back(tpl);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "nest %d", i);
            check(has(out, t) && has(out, "<div"), nb, out.c_str());
        }
    }

    // Script blocks (js variants + ts lang)
    {
        const char* snippets[] = {"const x = 1;", "let y = 2;", "function f() {}", "export default {}", "import a from 'b';", "const f = () => {};"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* s = el("script");
            s->m_children.push_back(txt(snippets[i]));
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            char nb[64]; snprintf(nb, sizeof(nb), "script %d", i);
            check(has(out, "<script>") && has(out, "</script>"), nb, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* s = el("script");
            s->m_attrs.push_back("lang=\"ts\"");
            char code[64]; snprintf(code, sizeof(code), "const v%d: number = %d;", i, i);
            s->m_children.push_back(txt(code));
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "script-ts %d", i);
            check(has(out, "<script") && has(out, "lang=\"ts\""), nb, out.c_str());
        }
        // scoped/setup attrs
        const char* sattrs[] = {"setup", "setup lang=\"ts\"", "scoped", "module"};
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* s = el(i < 2 ? "script" : "style");
            s->m_attrs.push_back(sattrs[i]);
            s->m_children.push_back(txt(i < 2 ? "const a = 1;" : ".a { color: red; }"));
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            check(has(out, sattrs[i]), sattrs[i], out.c_str());
        }
    }

    // Style blocks (css variants + scoped)
    {
        const char* snippets[] = {".a { color: red; }", ".b { margin: 0; }", "#x { display: flex; }", "p { font-size: 12px; }"};
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* s = el("style");
            if (i % 2 == 0) { s->m_attrs.push_back("scoped"); }
            s->m_children.push_back(txt(snippets[i]));
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "style %d", i);
            check(has(out, "<style") && has(out, "</style>"), nb, out.c_str());
        }
    }

    // Full SFC (template + script + style)
    {
        for (int i = 0; i < 10; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* tpl = el("template");
            HtmlElementNode* p = el("p");
            char t[32]; snprintf(t, sizeof(t), "sfc%d", i);
            p->m_children.push_back(txt(t));
            tpl->m_children.push_back(p);
            prog->m_statements.push_back(tpl);
            HtmlElementNode* sc = el("script");
            char js[48]; snprintf(js, sizeof(js), "const n%d = %d;", i, i);
            sc->m_children.push_back(txt(js));
            prog->m_statements.push_back(sc);
            HtmlElementNode* st = el("style");
            if (i % 2 == 0) { st->m_attrs.push_back("scoped"); }
            st->m_children.push_back(txt(".a { color: red; }"));
            prog->m_statements.push_back(st);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "sfc %d", i);
            check(has(out, "<template>") && has(out, t) && has(out, "<script>") && has(out, "<style"), nb, out.c_str());
        }
    }

    // Edge: idempotency, empty, verbatim, invalid, no placeholder
    {
        ProgramNode* p1 = ASTNodeFactory::createProgramNode();
        HtmlElementNode* t1 = el("template");
        HtmlElementNode* pp = el("p");
        pp->m_children.push_back(txt("idem"));
        t1->m_children.push_back(pp);
        p1->m_statements.push_back(t1);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        HtmlElementNode* t2 = el("template");
        HtmlElementNode* pp2 = el("p");
        pp2->m_children.push_back(txt("idem"));
        t2->m_children.push_back(pp2);
        p2->m_statements.push_back(t2);
        std::string twice = fmtProg(p2);
        check(!once.empty() && once == twice, "idempotent", once.c_str());
        ProgramNode* empty = ASTNodeFactory::createProgramNode();
        check(!has(fmtProg(empty), "placeholder"), "empty", "");
        ProgramNode* raw = ASTNodeFactory::createProgramNode();
        RawTextNode* rt = ASTNodeFactory::createRawTextNode();
        rt->m_text = "@@@verbatim-kept###";
        raw->m_statements.push_back(rt);
        check(has(fmtProg(raw), "@@@verbatim-kept###"), "raw-verbatim", "");
        ProgramNode* unk = ASTNodeFactory::createProgramNode();
        HtmlElementNode* ue = el("my-widget99");
        ue->m_attrs.push_back(":prop=\"x\"");
        ue->m_children.push_back(txt("kept"));
        unk->m_statements.push_back(ue);
        check(has(fmtProg(unk), "my-widget99"), "unknown-tag", "");
    }

    printf("\n========================================\n");
    printf("VueFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
