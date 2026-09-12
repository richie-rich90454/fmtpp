// Easter egg: Python formatter tests - significant whitespace, significant beauty
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/FormatterFactory.hpp"
#include "fmtpp/FormatterVisitor.hpp"
#include "fmtpp/ast/ProgramNode.hpp"

namespace {

std::string formatPython(const char* source)
{
    Parser parser;
    ProgramNode* ast = parser.parse(source, strlen(source), "python");
    if (ast == NULL) { return std::string(); }
    OutputBuffer out(8192);
    Config config;
    FormatterVisitor* formatter = FormatterFactory::create("python", out, config);
    formatter->format(ast);
    std::string result = out.toString();
    delete formatter;
    return result;
}

int failures = 0;

void check(bool ok, const char* name, const char* detail)
{
    if (ok) { printf("  PASS: %s\n", name); }
    else { printf("  FAIL: %s (%s)\n", name, detail); failures++; }
}

}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;

    // Test 1: def/class with suites, decorators, async
    {
        const char* src =
            "@dataclass\n"
            "class Dog(Animal):\n"
            "    def bark(self, times: int = 1) -> str:\n"
            "        return \"woof\"\n"
            "async def fetch(url):\n"
            "    return url\n";
        std::string out = formatPython(src);
        bool ok = true;
        if (out.find("class Dog(Animal):") == std::string::npos) { ok = false; }
        if (out.find("def bark(self, times: int = 1) -> str:") == std::string::npos) { ok = false; }
        if (out.find("async def fetch(url):") == std::string::npos) { ok = false; }
        if (out.find("placeholder") != std::string::npos) { ok = false; }
        check(ok, "def/class/async", out.c_str());
        if (ok) { passed++; }
    }

    // Test 2: if/elif/else chain stays chained (no duplication, no loss)
    {
        const char* src =
            "if a:\n"
            "    x()\n"
            "elif b:\n"
            "    y()\n"
            "else:\n"
            "    z()\n";
        std::string out = formatPython(src);
        bool ok = true;
        if (out.find("if a:") == std::string::npos) { ok = false; }
        if (out.find("elif b:") == std::string::npos) { ok = false; }
        if (out.find("else:") == std::string::npos) { ok = false; }
        // ponytail: the chain prints once; a doubled header means double-push
        size_t first = out.find("if a:");
        size_t last = out.rfind("if a:");
        if (first == std::string::npos || first != last) { ok = false; }
        check(ok, "if/elif/else chain", out.c_str());
        if (ok) { passed++; }
    }

    // Test 3: for/else, while, try/except/else/finally with two catches
    {
        const char* src =
            "for i in items:\n"
            "    use(i)\n"
            "else:\n"
            "    done()\n"
            "try:\n"
            "    risky()\n"
            "except IOError as e:\n"
            "    handle(e)\n"
            "except ValueError:\n"
            "    fix()\n"
            "else:\n"
            "    ok()\n"
            "finally:\n"
            "    cleanup()\n";
        std::string out = formatPython(src);
        bool ok = true;
        if (out.find("for i in items:") == std::string::npos) { ok = false; }
        if (out.find("except IOError as e:") == std::string::npos) { ok = false; }
        if (out.find("except ValueError:") == std::string::npos) { ok = false; }
        if (out.find("finally:") == std::string::npos) { ok = false; }
        // ponytail: one try header for the whole chain, never try-per-catch
        int tries = 0;
        size_t at = 0;
        while ((at = out.find("try:", at)) != std::string::npos) { tries++; at += 4; }
        if (tries != 1) { ok = false; }
        check(ok, "for/else + try chain", out.c_str());
        if (ok) { passed++; }
    }

    // Test 4: with, match, imports, inline suites
    {
        const char* src =
            "import os\n"
            "from sys import argv as av\n"
            "with open(f) as h:\n"
            "    data = h.read()\n"
            "match cmd:\n"
            "    case \"go\":\n"
            "        run()\n"
            "    case _:\n"
            "        stop()\n"
            "if x: y()\n";
        std::string out = formatPython(src);
        bool ok = true;
        if (out.find("import os") == std::string::npos) { ok = false; }
        if (out.find("from sys import argv as av") == std::string::npos) { ok = false; }
        if (out.find("with open(f) as h:") == std::string::npos) { ok = false; }
        if (out.find("match cmd:") == std::string::npos) { ok = false; }
        if (out.find("case \"go\":") == std::string::npos) { ok = false; }
        if (out.find("case _:") == std::string::npos) { ok = false; }
        check(ok, "with/match/imports", out.c_str());
        if (ok) { passed++; }
    }

    // Test 5: unclosed input still formats (suite runs to EOF, pass added)
    {
        const char* src = "def f():\n    x = 1\n";
        std::string out = formatPython(src);
        bool ok = !out.empty()
            && out.find("def f():") != std::string::npos
            && out.find("x = 1") != std::string::npos;
        check(ok, "unclosed suite", out.c_str());
        if (ok) { passed++; }
    }

    // Test 6: output is stable (formatting twice changes nothing)
    {
        const char* src =
            "def f(a, b=1):\n"
            "    if a:\n"
            "        return b\n"
            "    return None\n";
        std::string once = formatPython(src);
        Parser parser2;
        ProgramNode* ast2 = parser2.parse(once.c_str(), once.size(), "python");
        std::string twice;
        if (ast2 != NULL)
        {
            OutputBuffer out(8192);
            Config config;
            FormatterVisitor* formatter = FormatterFactory::create("python", out, config);
            formatter->format(ast2);
            twice = out.toString();
            delete formatter;
        }
        bool ok = !once.empty() && once == twice;
        check(ok, "idempotent", once.c_str());
        if (ok) { passed++; }
    }

    printf("\n========================================\n");
    printf("PythonFormatter tests: %d passed, %d failed\n", passed, failures);
    printf("========================================\n");

    if (failures > 0) { return 1; }
    return 0;
}
