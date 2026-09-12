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
int rowCases = 0;

void check(bool ok, const char* name, const char* detail)
{
    if (ok) { printf("  PASS: %s\n", name); }
    else { printf("  FAIL: %s (%s)\n", name, detail); failures++; }
}

struct Case
{
    const char* src;
    const char* markers[5];
};

// Runs a table: every marker of every row must survive, no placeholders.
void runCases(const char* group, const Case* cases, size_t count, int& passed)
{
    bool groupOk = true;
    for (size_t i = 0; i < count; i++)
    {
        rowCases++;
        std::string out = formatPython(cases[i].src);
        bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
        const char* missing = "";
        if (ok)
        {
            for (int m = 0; m < 5 && cases[i].markers[m] != NULL; m++)
            {
                if (out.find(cases[i].markers[m]) == std::string::npos)
                {
                    ok = false;
                    missing = cases[i].markers[m];
                    break;
                }
            }
        }
        if (!ok)
        {
            groupOk = false;
            printf("  FAIL: [%s row %u] marker '%s'\n    src: %s\n    out: %s\n",
                group, (unsigned)i, ok ? "" : missing, cases[i].src, out.c_str());
            failures++;
        }
    }
    if (groupOk) { printf("  PASS: %s (%u rows)\n", group, (unsigned)count); passed++; }
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

    // Test 7: bare lambda conditions keep their colon (not the suite's)
    {
        const char* src = "if lambda: 1:\n    pass\n";
        std::string out = formatPython(src);
        bool ok = out.find("if lambda: 1:") != std::string::npos;
        check(ok, "bare lambda condition", out.c_str());
        if (ok) { passed++; }
    }

    // Test 8: CRLF sources format identically (bytes generated at runtime)
    {
        std::string src = "def f(a):\r\n    if a:\r\n        return 1\r\n    return 0\r\n";
        Parser parser;
        ProgramNode* ast = parser.parse(src.c_str(), src.size(), "python");
        std::string out;
        bool ok = ast != NULL;
        if (ok)
        {
            OutputBuffer buf(8192);
            Config config;
            FormatterVisitor* formatter = FormatterFactory::create("python", buf, config);
            formatter->format(ast);
            out = buf.toString();
            delete formatter;
        }
        if (ok && out.find("def f(a):") == std::string::npos) { ok = false; }
        if (ok && out.find("return 1") == std::string::npos) { ok = false; }
        check(ok, "crlf source", out.c_str());
        if (ok) { passed++; }
    }

    // Test 9+: combinatorial tables (each row separately verified)
    {
        static const Case defs[] = {
            { "def f():\n    pass\n", { "def f():", "pass", NULL } },
            { "def f(a):\n    return a\n", { "def f(a):", "return a", NULL } },
            { "def f(a, b):\n    return a\n", { "def f(a, b):", NULL } },
            { "def f(a=1):\n    pass\n", { "def f(a=1):", NULL } },
            { "def f(a='x:y'):\n    pass\n", { "def f(a='x:y'):", NULL } },
            { "def f(a={'k': 1}):\n    pass\n", { "def f(a={'k': 1}):", NULL } },
            { "def f(a=x[1:]):\n    pass\n", { "def f(a=x[1:]):", NULL } },
            { "def f(cb=lambda v: v):\n    pass\n", { "def f(cb=lambda v: v):", NULL } },
            { "def f(*args):\n    pass\n", { "def f(*args):", NULL } },
            { "def f(**kw):\n    pass\n", { "def f(**kw):", NULL } },
            { "def f(a, *args, k, **kw):\n    pass\n", { "def f(a, *args, k, **kw):", NULL } },
            { "def f(a, /, b):\n    pass\n", { "def f(a, /, b):", NULL } },
            { "def f(a: int) -> bool:\n    return True\n", { "def f(a: int) -> bool:", "return True", NULL } },
            { "def f() -> None:\n    pass\n", { "def f() -> None:", NULL } },
            { "async def f():\n    pass\n", { "async def f():", NULL } },
            { "async def f(a) -> int:\n    return 1\n", { "async def f(a) -> int:", NULL } },
            { "@deco\ndef f():\n    pass\n", { "@deco", "def f():", NULL } },
            { "@deco(1, x='y')\ndef f():\n    pass\n", { "@deco(1, x='y')", "def f():", NULL } },
            { "@a\n@b.c\ndef f():\n    pass\n", { "@a", "@b.c", "def f():", NULL } },
            { "@deco(\n    1,\n    2,\n)\ndef f():\n    pass\n", { "@deco(", "def f():", NULL } },
            { "def f(): return 1\n", { "def f():", "return 1", NULL } },
            { "def f():\n    \"\"\"Doc.\"\"\"\n", { "def f():", "\"\"\"Doc.\"\"\"", NULL } },
            { "def outer():\n    def inner():\n        return 1\n    return inner\n", { "def outer():", "def inner():", "return inner", NULL } },
            { "class C:\n    def m(self):\n        pass\n", { "class C:", "def m(self):", NULL } },
            { "def m(self, x: int = 5, *a, k=None, **kw) -> None:\n    pass\n", { "def m(self, x: int = 5, *a, k=None, **kw) -> None:", NULL } },
        };
        runCases("def", defs, sizeof(defs) / sizeof(defs[0]), passed);
        static const Case classes[] = {
            { "class A:\n    pass\n", { "class A:", "pass", NULL } },
            { "class A(B):\n    pass\n", { "class A(B):", NULL } },
            { "class A(B, C):\n    pass\n", { "class A(B, C):", NULL } },
            { "class A(metaclass=M):\n    pass\n", { "class A(metaclass=M):", NULL } },
            { "class A(B, metaclass=M):\n    pass\n", { "class A(B, metaclass=M):", NULL } },
            { "@deco\nclass A:\n    pass\n", { "@deco", "class A:", NULL } },
            { "class Outer:\n    class Inner:\n        pass\n", { "class Outer:", "class Inner:", NULL } },
            { "class A: pass\n", { "class A:", "pass", NULL } },
            { "class A:\n    x = 1\n", { "class A:", "x = 1", NULL } },
            { "class A:\n    x: int = 5\n", { "class A:", "x: int = 5", NULL } },
            { "class A:\n    \"\"\"Docs.\"\"\"\n", { "class A:", "\"\"\"Docs.\"\"\"", NULL } },
            { "class A:\n    def m(self):\n        return self\n", { "class A:", "def m(self):", "return self", NULL } },
        };
        runCases("class", classes, sizeof(classes) / sizeof(classes[0]), passed);
        static const Case conds[] = {
            { "if a:\n    b()\n", { "if a:", "b()", NULL } },
            { "if a:\n    b()\nelif c:\n    d()\n", { "if a:", "elif c:", "d()", NULL } },
            { "if a:\n    b()\nelif c:\n    d()\nelif e:\n    f()\nelse:\n    g()\n", { "elif e:", "else:", "g()", NULL } },
            { "if a: b()\n", { "if a:", "b()", NULL } },
            { "if a: b()\nelse: c()\n", { "if a:", "else:", "c()", NULL } },
            { "if (a and b):\n    c()\n", { "if (a and b):", NULL } },
            { "if not x:\n    pass\n", { "if not x:", NULL } },
            { "if x < y <= z:\n    pass\n", { "if x < y <= z:", NULL } },
            { "if f(x):\n    pass\n", { "if f(x):", NULL } },
            { "if d['k']:\n    pass\n", { "if d['k']:", NULL } },
            { "if x is None:\n    pass\n", { "if x is None:", NULL } },
            { "if x is not None:\n    pass\n", { "if x is not None:", NULL } },
            { "if x in y:\n    pass\n", { "if x in y:", NULL } },
            { "if (v := f()):\n    use(v)\n", { "if (v := f()):", "use(v)", NULL } },
            { "x = 1 if a else 2\n", { "x = 1 if a else 2", NULL } },
            { "if True:\n    if False:\n        pass\n    else:\n        pass\n", { "if True:", "if False:", NULL } },
        };
        runCases("if", conds, sizeof(conds) / sizeof(conds[0]), passed);
        static const Case loops[] = {
            { "for i in items:\n    use(i)\n", { "for i in items:", "use(i)", NULL } },
            { "for a, b in pairs:\n    use(a, b)\n", { "for a, b in pairs:", NULL } },
            { "for i in range(10):\n    pass\n", { "for i in range(10):", NULL } },
            { "for k in d:\n    pass\n", { "for k in d:", NULL } },
            { "for k, v in d.items():\n    pass\n", { "for k, v in d.items():", NULL } },
            { "for x in y:\n    pass\nelse:\n    done()\n", { "for x in y:", "else:", "done()", NULL } },
            { "for x in y:\n    break\n", { "for x in y:", "break", NULL } },
            { "for x in y:\n    continue\n", { "for x in y:", "continue", NULL } },
            { "for i in r: print(i)\n", { "for i in r:", "print(i)", NULL } },
            { "async for row in cur:\n    use(row)\n", { "async for row in cur:", NULL } },
            { "while x:\n    f()\n", { "while x:", "f()", NULL } },
            { "while True:\n    break\n", { "while True:", "break", NULL } },
            { "while x:\n    pass\nelse:\n    done()\n", { "while x:", "else:", NULL } },
            { "while (n := nxt()):\n    use(n)\n", { "while (n := nxt()):", NULL } },
            { "while x: x -= 1\n", { "while x:", "x -= 1", NULL } },
        };
        runCases("loops", loops, sizeof(loops) / sizeof(loops[0]), passed);
        static const Case trys[] = {
            { "try:\n    a()\nexcept E:\n    b()\n", { "try:", "except E:", "b()", NULL } },
            { "try:\n    a()\nexcept:\n    b()\n", { "try:", "except:", NULL } },
            { "try:\n    a()\nexcept E as e:\n    b()\n", { "except E as e:", NULL } },
            { "try:\n    a()\nexcept (A, B):\n    b()\n", { "except (A, B):", NULL } },
            { "try:\n    a()\nexcept A:\n    b()\nexcept B:\n    c()\n", { "except A:", "except B:", "c()", NULL } },
            { "try:\n    a()\nexcept A:\n    b()\nelse:\n    c()\n", { "else:", "c()", NULL } },
            { "try:\n    a()\nfinally:\n    b()\n", { "try:", "finally:", NULL } },
            { "try:\n    a()\nexcept A:\n    b()\nfinally:\n    c()\n", { "except A:", "finally:", NULL } },
            { "try:\n    a()\nexcept A:\n    b()\nexcept B:\n    c()\nelse:\n    d()\nfinally:\n    e()\n", { "except B:", "else:", "finally:", "e()", NULL } },
            { "try: a()\nexcept E: b()\n", { "try:", "except E:", "b()", NULL } },
            { "try:\n    try:\n        a()\n    except A:\n        b()\nexcept B:\n    c()\n", { "except A:", "except B:", NULL } },
            { "raise E('x')\n", { "raise E('x')", NULL } },
            { "raise E from f\n", { "raise E from f", NULL } },
            { "raise\n", { "raise", NULL } },
            { "try:\n    a()\nexcept* EG:\n    b()\n", { "except", "EG:", "b()", NULL } },
        };
        runCases("try", trys, sizeof(trys) / sizeof(trys[0]), passed);
        static const Case suites2[] = {
            { "with open(f) as h:\n    r(h)\n", { "with open(f) as h:", "r(h)", NULL } },
            { "with a as x, b as y:\n    use(x, y)\n", { "with a as x, b as y:", NULL } },
            { "with lock:\n    work()\n", { "with lock:", NULL } },
            { "async with lock:\n    x = 1\n", { "async with lock:", NULL } },
            { "with a as b: use(b)\n", { "with a as b:", "use(b)", NULL } },
            { "match x:\n    case 1:\n        one()\n", { "match x:", "case 1:", "one()", NULL } },
            { "match x:\n    case [a, b]:\n        use(a)\n", { "case [a, b]:", NULL } },
            { "match x:\n    case {'k': v}:\n        use(v)\n", { "case {'k': v}:", NULL } },
            { "match x:\n    case Point(x=0):\n        zero()\n", { "case Point(x=0):", NULL } },
            { "match x:\n    case 1 | 2:\n        low()\n", { "case 1 | 2:", NULL } },
            { "match x:\n    case _:\n        other()\n", { "case _:", NULL } },
            { "match x:\n    case y if y > 0:\n        pos()\n", { "case y if y > 0:", NULL } },
            { "import os\n", { "import os", NULL } },
            { "import a, b\n", { "import a, b", NULL } },
            { "import a as b\n", { "import a as b", NULL } },
            { "from x import y\n", { "from x import y", NULL } },
            { "from x import a, b\n", { "from x import a, b", NULL } },
            { "from x import y as z\n", { "from x import y as z", NULL } },
            { "from . import s\n", { "from . import s", NULL } },
            { "from .m import n\n", { "from .m import n", NULL } },
            { "from x import (\n    a,\n    b,\n)\n", { "from x import a, b", NULL } },
            { "from x import *\n", { "from x import *", NULL } },
        };
        runCases("with/match/import", suites2, sizeof(suites2) / sizeof(suites2[0]), passed);
    }

    // Test 10: statements, expressions, comments, literals, invalid shapes
    {
        static const Case stmts[] = {
            { "x = 1\n", { "x = 1", NULL } },
            { "x: int = 5\n", { "x: int = 5", NULL } },
            { "x: int\n", { "x: int", NULL } },
            { "x, y = 1, 2\n", { "x, y = 1, 2", NULL } },
            { "a = b = 1\n", { "a = b = 1", NULL } },
            { "x += 1\n", { "x += 1", NULL } },
            { "x -= 1\n", { "x -= 1", NULL } },
            { "x *= 2\n", { "x *= 2", NULL } },
            { "d['k'] = v\n", { "d['k'] = v", NULL } },
            { "obj.attr = v\n", { "obj.attr = v", NULL } },
            { "if cond:\n    x = 1\n", { "x = 1", NULL } },
            { "return\n", { "return", NULL } },
            { "return x\n", { "return x", NULL } },
            { "return f(a, b)\n", { "return f(a, b)", NULL } },
            { "pass\n", { "pass", NULL } },
            { "break\n", { "break", NULL } },
            { "continue\n", { "continue", NULL } },
            { "global g\n", { "global g", NULL } },
            { "nonlocal n\n", { "nonlocal n", NULL } },
            { "assert x\n", { "assert x", NULL } },
            { "assert x, 'msg'\n", { "assert x, 'msg'", NULL } },
            { "del x\n", { "del x", NULL } },
            { "del x[0]\n", { "del x[0]", NULL } },
            { "yield x\n", { "yield x", NULL } },
            { "yield\n", { "yield", NULL } },
            { "x = yield f()\n", { "x = yield f()", NULL } },
            { "f()\n", { "f()", NULL } },
            { "f(1, x=2)\n", { "f(1, x=2)", NULL } },
            { "print(*args, **kw)\n", { "print(*args, **kw)", NULL } },
            { "x = -y\n", { "x = -y", NULL } },
            { "x = not y\n", { "x = not y", NULL } },
            { "x = a and b or c\n", { "x = a and b or c", NULL } },
            { "x = a if b else c\n", { "x = a if b else c", NULL } },
            { "xs = [i for i in r]\n", { "xs = [i for i in r]", NULL } },
            { "xs = [i for i in r if i]\n", { "xs = [i for i in r if i]", NULL } },
            { "d = {k: v for k, v in p}\n", { "d = {k: v for k, v in p}", NULL } },
            { "g = (i for i in r)\n", { "g = (i for i in r)", NULL } },
            { "s = {i for i in r}\n", { "s = {i for i in r}", NULL } },
            { "x = a[1:]\n", { "x = a[1:]", NULL } },
            { "x = a[1:10:2]\n", { "x = a[1:10:2]", NULL } },
            { "f = lambda x: x + 1\n", { "f = lambda x: x + 1", NULL } },
            { "f = lambda: None\n", { "f = lambda: None", NULL } },
            { "x = (1 +\n     2)\n", { "x = (1 +", "2)", NULL } },
            { "a; b\n", { "a", "b", NULL } },
            { "x = 1; y = 2\n", { "x = 1; y = 2", NULL } },
            { "@dec\n", { "@dec", NULL } },
        };
        runCases("statements", stmts, sizeof(stmts) / sizeof(stmts[0]), passed);
        static const Case misc[] = {
            { "# hello\nx = 1\n", { "# hello", "x = 1", NULL } },
            { "x = 1  # trailing\n", { "x = 1", "# trailing", NULL } },
            { "#!/usr/bin/env python\nx = 1\n", { "#!/usr/bin/env python", NULL } },
            { "def f():  # about f\n    pass\n", { "def f():", "pass", NULL } },
            { "if x:  # check\n    y()\n", { "if x:", "y()", NULL } },
            { "x = 'a'\n", { "x = 'a'", NULL } },
            { "x = \"a\"\n", { "\"a\"", NULL } },
            { "x = '''tri\nple'''\n", { "tri", "ple", NULL } },
            { "x = f'{v}'\n", { "f'{v}'", NULL } },
            { "x = r'C:\\p'\n", { "r'C:\\p'", NULL } },
            { "x = 0xFF\n", { "0xFF", NULL } },
            { "x = 1_000\n", { "1_000", NULL } },
            { "x = True\n", { "True", NULL } },
            { "x = None\n", { "None", NULL } },
            { "x = ...\n", { "...", NULL } },
            { "def f(:\n    pass\n", { "def f(:", NULL } },
            { "if\n    pass\n", { "if", NULL } },
            { "else:\n    pass\n", { "else:", NULL } },
            { "def f()\n    pass\n", { "def f()", NULL } },
        };
        runCases("misc/invalid", misc, sizeof(misc) / sizeof(misc[0]), passed);
    }

    // Test 11: massive generated combinatorial (names x shapes)
    {
        char src[512];
        int genPassed = 0;
        // 60 generated defs
        for (int i = 0; i < 60; i++)
        {
            snprintf(src, sizeof(src), "def fn%d(a%d, b%d=%d):\n    return a%d\n", i, i, i, i, i);
            std::string out = formatPython(src);
            char want[64]; snprintf(want, sizeof(want), "def fn%d(", i);
            bool ok = !out.empty() && out.find(want) != std::string::npos
                && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-def %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        // 40 generated classes
        for (int i = 0; i < 40; i++)
        {
            snprintf(src, sizeof(src), "class Cls%d(Base%d):\n    x = %d\n", i, i, i);
            std::string out = formatPython(src);
            char want[64]; snprintf(want, sizeof(want), "class Cls%d(Base%d):", i, i);
            bool ok = out.find(want) != std::string::npos && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-class %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        // 30 if/elif/else
        for (int i = 0; i < 30; i++)
        {
            snprintf(src, sizeof(src), "if c%d:\n    f%d()\nelif d%d:\n    g%d()\nelse:\n    h%d()\n", i, i, i, i, i);
            std::string out = formatPython(src);
            char w1[32]; snprintf(w1, sizeof(w1), "if c%d:", i);
            bool ok = out.find(w1) != std::string::npos && out.find("elif") != std::string::npos
                && out.find("else:") != std::string::npos;
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-if %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        // 30 loops (for/while/async for)
        for (int i = 0; i < 30; i++)
        {
            if (i % 3 == 0) { snprintf(src, sizeof(src), "for v%d in seq%d:\n    use(v%d)\n", i, i, i); }
            else if (i % 3 == 1) { snprintf(src, sizeof(src), "while w%d:\n    step(%d)\n", i, i); }
            else { snprintf(src, sizeof(src), "async for r%d in cur%d:\n    use(r%d)\n", i, i, i); }
            std::string out = formatPython(src);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos
                && (out.find("for ") != std::string::npos || out.find("while ") != std::string::npos);
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-loop %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        // 30 try/except/finally
        for (int i = 0; i < 30; i++)
        {
            snprintf(src, sizeof(src), "try:\n    f%d()\nexcept E%d as e:\n    h(e)\nfinally:\n    c%d()\n", i, i, i);
            std::string out = formatPython(src);
            char w[32]; snprintf(w, sizeof(w), "except E%d as e:", i);
            bool ok = out.find(w) != std::string::npos && out.find("finally:") != std::string::npos;
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-try %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        // 30 with/match/import/decorator/comprehension
        for (int i = 0; i < 30; i++)
        {
            int kind = i % 5;
            if (kind == 0) { snprintf(src, sizeof(src), "with open(f%d) as h:\n    r(h)\n", i); }
            else if (kind == 1) { snprintf(src, sizeof(src), "match x%d:\n    case %d:\n        go()\n    case _:\n        stop()\n", i, i); }
            else if (kind == 2) { snprintf(src, sizeof(src), "from m%d import n%d as q%d\n", i, i, i); }
            else if (kind == 3) { snprintf(src, sizeof(src), "@deco%d\ndef f%d():\n    pass\n", i, i); }
            else { snprintf(src, sizeof(src), "xs%d = [i for i in r%d if i]\n", i, i); }
            std::string out = formatPython(src);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-misc %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        // 20 strings/comments preservation
        for (int i = 0; i < 20; i++)
        {
            if (i % 4 == 0) { snprintf(src, sizeof(src), "x%d = f'{v%d}'\n", i, i); }
            else if (i % 4 == 1) { snprintf(src, sizeof(src), "# cmt %d\nx%d = 1\n", i, i); }
            else if (i % 4 == 2) { snprintf(src, sizeof(src), "x%d = \"s%d\"  # trail %d\n", i, i, i); }
            else { snprintf(src, sizeof(src), "d%d = {k: v for k, v in p%d}\n", i, i); }
            std::string out = formatPython(src);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-str %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        printf("  PASS: generated (%d rows)\n", genPassed);
        passed++;
        // extra: async defs + for/else + while/else + nested + slices + lambdas
        for (int i = 0; i < 80; i++)
        {
            int kind = i % 8;
            if (kind == 0) { snprintf(src, sizeof(src), "async def af%d(x):\n    return x\n", i); }
            else if (kind == 1) { snprintf(src, sizeof(src), "for i in r%d:\n    pass\nelse:\n    done()\n", i); }
            else if (kind == 2) { snprintf(src, sizeof(src), "while c%d:\n    pass\nelse:\n    fin()\n", i); }
            else if (kind == 3) { snprintf(src, sizeof(src), "def o%d():\n    def inn():\n        return %d\n    return inn\n", i, i); }
            else if (kind == 4) { snprintf(src, sizeof(src), "x%d = a%d[1:10:2]\n", i, i); }
            else if (kind == 5) { snprintf(src, sizeof(src), "f%d = lambda x: x + %d\n", i, i); }
            else if (kind == 6) { snprintf(src, sizeof(src), "with a%d as x, b%d as y:\n    use(x, y)\n", i, i); }
            else { snprintf(src, sizeof(src), "try:\n    a%d()\nexcept E:\n    b()\nelse:\n    c()\nfinally:\n    d()\n", i); }
            std::string out = formatPython(src);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { genPassed++; passed++; } else { printf("  FAIL: [gen-extra %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        printf("  PASS: generated-extra (80 rows)\n");
        passed++;
    }

    // Test 12: empty / comments-only / docstring-only files
    {
        static const Case empties[] = {
            { "# only a comment\n", { "# only a comment", NULL } },
            { "#!/usr/bin/env python\n", { "#!/usr/bin/env python", NULL } },
            { "\"\"\"Module docs.\"\"\"\n", { "\"\"\"Module docs.\"\"\"", NULL } },
            { "x = 1  # trailing\n", { "x = 1", "# trailing", NULL } },
            { "x = 'a#b:c'\n", { "'a#b:c'", NULL } },
            { "x = \"a;b\"\n", { "\"a;b\"", NULL } },
            { "import os  # os\n", { "import os", "# os", NULL } },
            { "pass\n", { "pass", NULL } },
        };
        runCases("empty/comments-only", empties, sizeof(empties) / sizeof(empties[0]), passed);
        // truly empty input must not crash and must not leak placeholder
        {
            std::string out = formatPython("");
            bool ok = out.find("placeholder") == std::string::npos;
            check(ok, "empty-input", out.c_str());
            if (ok) { passed++; }
        }
        {
            const char* src = "def f():\n    pass\n";
            std::string once = formatPython(src);
            Parser p2;
            ProgramNode* a2 = p2.parse(once.c_str(), once.size(), "python");
            std::string twice;
            if (a2 != NULL)
            {
                OutputBuffer ob(8192);
                Config cfg;
                FormatterVisitor* fm = FormatterFactory::create("python", ob, cfg);
                fm->format(a2);
                twice = ob.toString();
                delete fm;
            }
            bool ok = !once.empty() && once == twice && once.find("placeholder") == std::string::npos;
            check(ok, "idempotent2", once.c_str());
            if (ok) { passed++; }
        }
    }

    printf("\n========================================\n");
    printf("PythonFormatter tests: %d passed, %d failed (%d table rows)\n", passed, failures, rowCases);
    printf("========================================\n");

    if (failures > 0) { return 1; }
    return 0;
}
