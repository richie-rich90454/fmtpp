// Easter egg: HTML formatter tests - divinely nested assertions
#include <cstdio>
#include <cstring>
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

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: single text child stays inline
    {
        OutputBuffer out(4096);
        Config config;
        HtmlFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        HtmlElementNode* p = ASTNodeFactory::createHtmlElementNode();
        p->m_tag = "p";
        RawTextNode* text = ASTNodeFactory::createRawTextNode();
        text->m_text = "hello";
        p->m_children.push_back(text);
        prog->m_statements.push_back(p);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "<p>hello</p>") == NULL) { printf("  FAIL: missing '<p>hello</p>'\n"); ok = false; }
        if (ok) { printf("Test 1 (inline text): PASSED\n"); passed++; }
        else { printf("Test 1 (inline text): FAILED\n"); failed++; }
    }

    // Test 2: nested elements with attributes indent
    {
        OutputBuffer out(4096);
        Config config;
        HtmlFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        HtmlElementNode* div = ASTNodeFactory::createHtmlElementNode();
        div->m_tag = "div";
        div->m_attrs.push_back("class=\"app\"");
        HtmlElementNode* span = ASTNodeFactory::createHtmlElementNode();
        span->m_tag = "span";
        RawTextNode* text = ASTNodeFactory::createRawTextNode();
        text->m_text = "hi";
        span->m_children.push_back(text);
        div->m_children.push_back(span);
        RawTextNode* tail = ASTNodeFactory::createRawTextNode();
        tail->m_text = "bye";
        div->m_children.push_back(tail);
        prog->m_statements.push_back(div);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "<div class=\"app\">") == NULL) { printf("  FAIL: missing open tag\n"); ok = false; }
        if (strstr(result, "<span>hi</span>") == NULL) { printf("  FAIL: missing nested span\n"); ok = false; }
        if (strstr(result, "</div>") == NULL) { printf("  FAIL: missing close tag\n"); ok = false; }
        if (ok) { printf("Test 2 (nesting): PASSED\n"); passed++; }
        else { printf("Test 2 (nesting): FAILED\n"); failed++; }
    }

    // Test 3: void elements, comments and doctype
    {
        OutputBuffer out(4096);
        Config config;
        HtmlFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* doctype = ASTNodeFactory::createPreprocessorDirectiveNode();
        doctype->m_directive = "<!DOCTYPE html>";
        prog->m_statements.push_back(doctype);
        CommentNode* cmt = ASTNodeFactory::createCommentNode();
        cmt->m_text = " a comment ";
        cmt->m_isBlock = true;
        prog->m_statements.push_back(cmt);
        HtmlElementNode* br = ASTNodeFactory::createHtmlElementNode();
        br->m_tag = "br";
        prog->m_statements.push_back(br);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "<!DOCTYPE html>") == NULL) { printf("  FAIL: missing doctype\n"); ok = false; }
        if (strstr(result, "<!-- a comment -->") == NULL) { printf("  FAIL: comment not in HTML form\n"); ok = false; }
        if (strstr(result, "<br>") == NULL) { printf("  FAIL: missing '<br>'\n"); ok = false; }
        if (strstr(result, "</br>") != NULL) { printf("  FAIL: void element must not close\n"); ok = false; }
        if (ok) { printf("Test 3 (void/comment/doctype): PASSED\n"); passed++; }
        else { printf("Test 3 (void/comment/doctype): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("HtmlFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
