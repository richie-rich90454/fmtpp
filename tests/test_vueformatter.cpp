// Easter egg: Vue formatter tests - reactively passing, progressively true
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/VueFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/HtmlElementNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: template block formats as HTML through the base path
    {
        OutputBuffer out(4096);
        Config config;
        VueFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        HtmlElementNode* tpl = ASTNodeFactory::createHtmlElementNode();
        tpl->m_tag = "template";
        HtmlElementNode* p = ASTNodeFactory::createHtmlElementNode();
        p->m_tag = "p";
        RawTextNode* text = ASTNodeFactory::createRawTextNode();
        text->m_text = "hi";
        p->m_children.push_back(text);
        tpl->m_children.push_back(p);
        prog->m_statements.push_back(tpl);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "<template>") == NULL) { printf("  FAIL: missing '<template>'\n"); ok = false; }
        if (strstr(result, "<p>hi</p>") == NULL) { printf("  FAIL: missing '<p>hi</p>'\n"); ok = false; }
        if (strstr(result, "</template>") == NULL) { printf("  FAIL: missing '</template>'\n"); ok = false; }
        if (ok) { printf("Test 1 (template): PASSED\n"); passed++; }
        else { printf("Test 1 (template): FAILED\n"); failed++; }
    }

    // Test 2: script block delegates to JS, style block to CSS
    {
        OutputBuffer out(4096);
        Config config;
        VueFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        HtmlElementNode* script = ASTNodeFactory::createHtmlElementNode();
        script->m_tag = "script";
        RawTextNode* js = ASTNodeFactory::createRawTextNode();
        js->m_text = "const x = 1;";
        script->m_children.push_back(js);
        prog->m_statements.push_back(script);
        HtmlElementNode* style = ASTNodeFactory::createHtmlElementNode();
        style->m_tag = "style";
        RawTextNode* css = ASTNodeFactory::createRawTextNode();
        css->m_text = ".a { color: red; }";
        style->m_children.push_back(css);
        prog->m_statements.push_back(style);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "<script>") == NULL) { printf("  FAIL: missing '<script>'\n"); ok = false; }
        if (strstr(result, "const x = 1;") == NULL) { printf("  FAIL: missing formatted JS\n"); ok = false; }
        if (strstr(result, "</script>") == NULL) { printf("  FAIL: missing '</script>'\n"); ok = false; }
        if (strstr(result, "<style>") == NULL) { printf("  FAIL: missing '<style>'\n"); ok = false; }
        if (strstr(result, "color: red;") == NULL) { printf("  FAIL: missing formatted CSS\n"); ok = false; }
        if (strstr(result, "</style>") == NULL) { printf("  FAIL: missing '</style>'\n"); ok = false; }
        if (ok) { printf("Test 2 (script/style): PASSED\n"); passed++; }
        else { printf("Test 2 (script/style): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("VueFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
