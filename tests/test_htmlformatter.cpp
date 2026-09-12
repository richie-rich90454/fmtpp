// Easter egg: HTML formatter tests - divinely nested assertions
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/HtmlFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
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
    HtmlFormatter formatter(out, config);
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

    // Tags inline (single text child stays inline)
    {
        const char* tags[] = {"p", "span", "h1", "h2", "a", "b", "i", "em", "strong", "code", "li", "td"};
        const char* texts[] = {"hello", "hi", "title", "link", "bold", "x", "y", "z", "t", "u", "v", "w"};
        for (int i = 0; i < 12; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* e = el(tags[i]);
            e->m_children.push_back(txt(texts[i]));
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            char want[64]; snprintf(want, sizeof(want), "<%s>%s</%s>", tags[i], texts[i], tags[i]);
            check(has(out, want) && !has(out, "placeholder"), want, out.c_str());
        }
    }

    // Attributes (1-3 attrs per tag)
    {
        const char* tags[] = {"div", "span", "a", "img", "input", "form", "button", "p", "section", "article"};
        const char* attrs[] = {"class=\"app\"", "id=\"main\"", "href=\"/x\"", "src=\"a.png\"", "type=\"text\"", "disabled", "data-x=\"1\"", "title=\"t\"", "role=\"btn\"", "lang=\"en\""};
        for (int i = 0; i < 10; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* e = el(tags[i]);
            e->m_attrs.push_back(attrs[i]);
            if (i % 2 == 0) { e->m_attrs.push_back(attrs[(i + 3) % 10]); }
            if (i % 3 == 0) { e->m_children.push_back(txt("body")); }
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            char nb[64]; snprintf(nb, sizeof(nb), "attr %s", tags[i]);
            check(has(out, tags[i]) && has(out, attrs[i]), nb, out.c_str());
        }
    }

    // Nesting depth 2-4
    {
        const char* outers[] = {"div", "section", "main", "article", "nav", "aside"};
        const char* inners[] = {"span", "p", "a", "ul", "li", "b"};
        for (int i = 0; i < 6; i++)
            for (int d = 0; d < 4; d++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                HtmlElementNode* outer = el(outers[i]);
                outer->m_attrs.push_back("class=\"wrap\"");
                HtmlElementNode* cur = outer;
                for (int k = 0; k <= d; k++)
                {
                    HtmlElementNode* ch = el(inners[(i + k) % 6]);
                    if (k == d) { ch->m_children.push_back(txt("leaf")); }
                    cur->m_children.push_back(ch);
                    cur = ch;
                }
                prog->m_statements.push_back(outer);
                std::string out = fmtProg(prog);
                char nb[64]; snprintf(nb, sizeof(nb), "nest %s/%d", outers[i], d);
                check(has(out, outers[i]) && has(out, "leaf") && has(out, "</") && !has(out, "placeholder"), nb, out.c_str());
            }
    }

    // Void elements never close
    {
        const char* voids[] = {"br", "img", "input", "hr", "meta", "link", "source", "col", "embed", "track", "wbr", "area"};
        for (int i = 0; i < 12; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* e = el(voids[i]);
            if (i % 3 == 0) { e->m_attrs.push_back("src=\"a.png\""); }
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            char open[32]; snprintf(open, sizeof(open), "<%s", voids[i]);
            char close[32]; snprintf(close, sizeof(close), "</%s>", voids[i]);
            bool ok = has(out, open) && !has(out, close) && !has(out, "placeholder");
            char nb[32]; snprintf(nb, sizeof(nb), "void %s", voids[i]);
            check(ok, nb, out.c_str());
        }
    }

    // Comments + doctypes
    {
        const char* doctypes[] = {"<!DOCTYPE html>", "<!doctype html>", "<!DOCTYPE HTML>"};
        for (int i = 0; i < 3; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            PreprocessorDirectiveNode* d = ASTNodeFactory::createPreprocessorDirectiveNode();
            d->m_directive = doctypes[i];
            prog->m_statements.push_back(d);
            std::string out = fmtProg(prog);
            check(has(out, doctypes[i]), doctypes[i], out.c_str());
        }
        for (int i = 0; i < 10; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            CommentNode* c = ASTNodeFactory::createCommentNode();
            char t[32]; snprintf(t, sizeof(t), " cmt %d ", i);
            c->m_text = t;
            c->m_isBlock = true;
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "html-cmt %d", i);
            check(has(out, "<!--") && has(out, t) && has(out, "-->"), nb, out.c_str());
        }
    }

    // Full documents (doctype + html/head/body nesting)
    {
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            PreprocessorDirectiveNode* d = ASTNodeFactory::createPreprocessorDirectiveNode();
            d->m_directive = "<!DOCTYPE html>";
            prog->m_statements.push_back(d);
            HtmlElementNode* html = el("html");
            HtmlElementNode* head = el("head");
            HtmlElementNode* title = el("title");
            char tt[32]; snprintf(tt, sizeof(tt), "T%d", i);
            title->m_children.push_back(txt(tt));
            head->m_children.push_back(title);
            HtmlElementNode* body = el("body");
            HtmlElementNode* div = el("div");
            div->m_attrs.push_back("class=\"app\"");
            HtmlElementNode* p = el("p");
            char pb[32]; snprintf(pb, sizeof(pb), "para%d", i);
            p->m_children.push_back(txt(pb));
            div->m_children.push_back(p);
            body->m_children.push_back(div);
            html->m_children.push_back(head);
            html->m_children.push_back(body);
            prog->m_statements.push_back(html);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "doc %d", i);
            check(has(out, "<!DOCTYPE html>") && has(out, tt) && has(out, pb) && has(out, "</html>"), nb, out.c_str());
        }
    }

    // Self-closing + mixed text/element children
    {
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            HtmlElementNode* div = el("div");
            div->m_attrs.push_back("class=\"app\"");
            HtmlElementNode* span = el("span");
            char st[32]; snprintf(st, sizeof(st), "hi%d", i);
            span->m_children.push_back(txt(st));
            div->m_children.push_back(span);
            div->m_children.push_back(txt("bye"));
            prog->m_statements.push_back(div);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "mixed %d", i);
            check(has(out, st) && has(out, "bye") && has(out, "</div>"), nb, out.c_str());
        }
    }

    // Edge: idempotency, empty, verbatim, invalid, no placeholder
    {
        ProgramNode* p1 = ASTNodeFactory::createProgramNode();
        HtmlElementNode* e1 = el("p");
        e1->m_children.push_back(txt("idem"));
        p1->m_statements.push_back(e1);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        HtmlElementNode* e2 = el("p");
        e2->m_children.push_back(txt("idem"));
        p2->m_statements.push_back(e2);
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
        HtmlElementNode* ue = el("custom-tag99");
        ue->m_attrs.push_back("data-z=\"9\"");
        ue->m_children.push_back(txt("kept"));
        unk->m_statements.push_back(ue);
        check(has(fmtProg(unk), "custom-tag99") && has(fmtProg(unk), "kept"), "unknown-tag", "");
    }

    printf("\n========================================\n");
    printf("HtmlFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
