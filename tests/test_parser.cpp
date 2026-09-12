#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/SourceBuffer.hpp"
#include "fmtpp/ASTPrinter.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/FormatterFactory.hpp"
#include "fmtpp/FormatterVisitor.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"

// Easter egg: testing the parser - where text meets its destiny

extern thread_local MemoryPool* g_pool;

int main()
{
    int errors = 0;
    ProgramNode* program = NULL;

    MemoryPool pool;
    g_pool = &pool;

    /* Test SourceBuffer */
    /* First create a test file */
    FILE* f = NULL;
    f = fopen("test_parser_input.txt", "w");
    if (f != NULL)
    {
        fprintf(f, "const x = 1;\n");
        fclose(f);
    }

    SourceBuffer source("test_parser_input.txt");
    if (source.isValid())
    {
        printf("SourceBuffer: file loaded OK (size=%zu)\n", source.getSize());
    }
    else
    {
        printf("SourceBuffer: file load failed (may be OK if file doesn't exist)\n");
    }

    /* Test Parser */
    Parser parser;
    const char* testSource = "const x = 1;";
    program = parser.parse(testSource, strlen(testSource));

    if (program == NULL)
    {
        printf("FAIL: Parser.parse returned NULL\n");
        errors++;
        goto cleanup;
    }
    printf("Parser: parse OK\n");
    printf("ProgramNode statements: %zu\n", program->m_statements.size());

    /* Print AST */
    {
        printf("\n--- ASTPrinter output ---\n");
        ASTPrinter printer("  ");
        program->accept(printer);
        printf("--- end ---\n\n");
    }

    /* Verify structure */
    if (program->m_statements.size() > 0)
    {
        printf("Parser: AST structure verified OK\n");
    }
    else
    {
        printf("FAIL: ProgramNode has no statements\n");
        errors++;
    }

    /* Test SourceBuffer-based parse */
    program = parser.parse(source);
    if (program != NULL)
    {
        printf("Parser.parse(SourceBuffer): OK\n");
    }
    else
    {
        printf("FAIL: Parser.parse(SourceBuffer) returned NULL\n");
        errors++;
        goto cleanup;
    }

    printf("Parser::isValid(): %s\n", parser.isValid() ? "true" : "false");
    if (!parser.isValid()) { printf("FAIL: parser should be valid\n"); errors++; }

    // Round trips: real source in, structured formatting out.
    // Every case proves identifiers survive and no placeholder leaks.
    {
        struct Case
        {
            const char* language;
            const char* source;
            const char* markers[4];
        };
        Case cases[] = {
            { "js", "import { a } from \"m\";\nfunction add(x, y) {\nreturn x;\n}\n",
                { "import", "function add", "return x", NULL } },
            { "ts", "interface P {\nname: string;\n}\nfunction f(): number {\nreturn 1;\n}\n",
                { "interface P", "name: string", "function f(): number", NULL } },
            { "java", "import java.util.List;\npublic class Dog {\npublic void bark() {\n}\n}\n",
                { "import java.util.List;", "public class Dog", "public void bark()", NULL } },
            { "c", "#include <stdio.h>\nint add(int a, int b) {\nreturn a + b;\n}\n",
                { "#include <stdio.h>", "int add(int a, int b)", "return a + b;", NULL } },
            { "cpp", "namespace n {\nclass C : public B {\n};\n}\n",
                { "namespace n", "class C : public B", "};", NULL } },
            { "cs", "namespace N {\npublic class C {\npublic void M() {\n}\n}\n}\n",
                { "namespace N", "public class C", "public void M()", NULL } },
            { "go", "package main\nimport \"fmt\"\nfunc main() {\nreturn\n}\n",
                { "package main", "import \"fmt\"", "func main()", NULL } },
            { "rs", "use std::io;\nfn main() -> i32 {\nreturn 0;\n}\n",
                { "use std::io;", "fn main() -> i32", "return 0;", NULL } },
            { "kotlin", "package a\nfun add(x: Int): Int {\nreturn x\n}\n",
                { "package a", "fun add(x: Int): Int", "return x", NULL } },
            { "swift", "import Foundation\nfunc f() -> Int {\nreturn 1\n}\n",
                { "import Foundation", "func f() -> Int", "return 1", NULL } },
            { "dart", "import 'dart:math';\nvoid main() {\nreturn;\n}\n",
                { "import 'dart:math';", "void main()", NULL, NULL } },
            { "php", "<?php\nnamespace A;\nfunction f($x) {\nreturn $x;\n}\n",
                { "<?php", "namespace A;", "function f($x)", "return $x;" } },
            { "scala", "package a\ndef add(x: Int): Int = {\nx\n}\n",
                { "package a", "def add(x: Int): Int =", NULL, NULL } },
            { "objc", "#import <F.h>\n@interface Dog : NSObject\n@end\n",
                { "#import <F.h>", "@interface Dog : NSObject", "@end", NULL } },
            { "zig", "const std = @import(\"std\");\npub fn main() void {\n}\n",
                { "const std = @import(\"std\");", "pub fn main() void", NULL, NULL } },
            { "d", "module m;\nimport std.stdio;\nvoid main() {\n}\n",
                { "module m;", "import std.stdio;", "void main()", NULL } },
            { "groovy", "package a\ndef greet(name) {\nprintln name\n}\n",
                { "package a", "def greet(name)", "println name", NULL } },
            { "solidity", "pragma solidity ^0.8.0;\ncontract T {\n}\n",
                { "pragma solidity ^0.8.0;", "contract T", NULL, NULL } },
            { "v", "module main\nfn add(a int) int {\nreturn a\n}\n",
                { "module main", "fn add(a int) int", "return a", NULL } },
            { "glsl", "#version 330 core\nvoid main() {\n}\n",
                { "#version 330 core", "void main()", NULL, NULL } },
            { "html", "<div class=\"a\"><p>hi</p></div>\n",
                { "<div class=\"a\">", "<p>hi</p>", "</div>", NULL } },
            { "css", ".a {\ncolor: red;\n}\n",
                { ".a {", "color: red;", NULL, NULL } },
            { "json", "{\"a\": 1}\n",
                { "\"a\"", "1", NULL, NULL } },
            { "vue", "<template>\n<p>hi</p>\n</template>\n",
                { "<template>", "<p>hi</p>", "</template>", NULL } },
        };
        int roundTrips = sizeof(cases) / sizeof(cases[0]);
        for (int ci = 0; ci < roundTrips; ci++)
        {
            Parser rp;
            ProgramNode* ast = rp.parse(cases[ci].source, strlen(cases[ci].source),
                cases[ci].language);
            if (ast == NULL)
            {
                printf("FAIL: [%s] parse returned NULL\n", cases[ci].language);
                errors++;
                continue;
            }
            OutputBuffer out(8192);
            Config config;
            FormatterVisitor* formatter = FormatterFactory::create(cases[ci].language, out, config);
            formatter->format(ast);
            std::string result = out.toString();
            delete formatter;
            if (result.empty())
            {
                printf("FAIL: [%s] empty output\n", cases[ci].language);
                errors++;
                continue;
            }
            if (result.find("placeholder") != std::string::npos)
            {
                printf("FAIL: [%s] placeholder leaked into output\n", cases[ci].language);
                errors++;
                continue;
            }
            for (int mi = 0; mi < 4 && cases[ci].markers[mi] != NULL; mi++)
            {
                if (result.find(cases[ci].markers[mi]) == std::string::npos)
                {
                    printf("FAIL: [%s] marker missing: %s\n", cases[ci].language,
                        cases[ci].markers[mi]);
                    printf("  output was: %s\n", result.c_str());
                    errors++;
                }
            }
            printf("Round trip [%s]: OK\n", cases[ci].language);
        }
    }

cleanup:
    pool.reset();
    if (errors == 0)
    {
        printf("Phase 5 tests passed.\n");
    }
    else
    {
        printf("\n%d test(s) FAILED.\n", errors);
    }
    return errors;
}