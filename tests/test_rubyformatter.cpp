// Easter egg: Ruby formatter tests - end to end, happily ever after
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

std::string formatRuby(const char* source)
{
    Parser parser;
    ProgramNode* ast = parser.parse(source, strlen(source), "ruby");
    if (ast == NULL) { return std::string(); }
    OutputBuffer out(8192);
    Config config;
    FormatterVisitor* formatter = FormatterFactory::create("ruby", out, config);
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
        std::string out = formatRuby(cases[i].src);
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

    // Test 1: def/class/module with ends, endless def
    {
        const char* src =
            "class Dog < Animal\n"
            "  def bark(n)\n"
            "    puts n\n"
            "  end\n"
            "end\n"
            "module Tricks\n"
            "end\n"
            "def endless = 42\n";
        std::string out = formatRuby(src);
        bool ok = true;
        if (out.find("class Dog < Animal") == std::string::npos) { ok = false; }
        if (out.find("def bark(n)") == std::string::npos) { ok = false; }
        if (out.find("module Tricks") == std::string::npos) { ok = false; }
        if (out.find("def endless") == std::string::npos) { ok = false; }
        if (out.find("placeholder") != std::string::npos) { ok = false; }
        check(ok, "def/class/module/endless", out.c_str());
        if (ok) { passed++; }
    }

    // Test 2: if/elsif/else/unless chains with single ends
    {
        const char* src =
            "if a\n"
            "  x\n"
            "elsif b\n"
            "  y\n"
            "else\n"
            "  z\n"
            "end\n"
            "unless q\n"
            "  w\n"
            "end\n";
        std::string out = formatRuby(src);
        bool ok = true;
        if (out.find("if a") == std::string::npos) { ok = false; }
        if (out.find("elsif b") == std::string::npos) { ok = false; }
        if (out.find("unless q") == std::string::npos) { ok = false; }
        // ponytail: one if-chain prints exactly one end
        const char* first = out.c_str();
        int ends = 0;
        for (const char* p = first; (p = strstr(p, "\nend")) != NULL; p++, ends++) { }
        if (ends != 2) { ok = false; }
        check(ok, "if/elsif/else/unless", out.c_str());
        if (ok) { passed++; }
    }

    // Test 3: case/when/else, begin/rescue/else/ensure, method rescue
    {
        const char* src =
            "case x\n"
            "when 1\n"
            "  low\n"
            "else\n"
            "  high\n"
            "end\n"
            "begin\n"
            "  attempt\n"
            "rescue E => e\n"
            "  handle\n"
            "else\n"
            "  ok\n"
            "ensure\n"
            "  clean\n"
            "end\n"
            "def m\n"
            "  work\n"
            "rescue\n"
            "  fix\n"
            "end\n";
        std::string out = formatRuby(src);
        bool ok = true;
        if (out.find("case x") == std::string::npos) { ok = false; }
        if (out.find("when 1") == std::string::npos) { ok = false; }
        if (out.find("begin") == std::string::npos) { ok = false; }
        if (out.find("rescue E => e") == std::string::npos) { ok = false; }
        if (out.find("ensure") == std::string::npos) { ok = false; }
        if (out.find("def m") == std::string::npos) { ok = false; }
        check(ok, "case/begin/method-rescue", out.c_str());
        if (ok) { passed++; }
    }

    // Test 4: while/until/for, do-blocks, require, one-liners
    {
        const char* src =
            "require \"json\"\n"
            "while a\n"
            "  b\n"
            "end\n"
            "until c\n"
            "  d\n"
            "end\n"
            "for f in files\n"
            "  puts f\n"
            "end\n"
            "items.each do |x|\n"
            "  puts x\n"
            "end\n"
            "def h; i; end\n";
        std::string out = formatRuby(src);
        bool ok = true;
        if (out.find("require \"json\"") == std::string::npos) { ok = false; }
        if (out.find("while a") == std::string::npos) { ok = false; }
        if (out.find("until c") == std::string::npos) { ok = false; }
        if (out.find("for f in files") == std::string::npos) { ok = false; }
        if (out.find("items.each do |x|") == std::string::npos) { ok = false; }
        if (out.find("def h") == std::string::npos) { ok = false; }
        check(ok, "loops/do/require/one-liners", out.c_str());
        if (ok) { passed++; }
    }

    // Test 5: heredoc and __END__ tails stay verbatim
    {
        const char* src =
            "y = foo(<<~EOS, 1)\n"
            "  hello;\n"
            "EOS\n"
            "__END__\n"
            "tail;\n";
        std::string out = formatRuby(src);
        bool ok = true;
        if (out.find("<<~EOS") == std::string::npos) { ok = false; }
        if (out.find("hello;") == std::string::npos) { ok = false; }
        if (out.find("__END__") == std::string::npos) { ok = false; }
        if (out.find("tail;") == std::string::npos) { ok = false; }
        check(ok, "heredoc/__END__", out.c_str());
        if (ok) { passed++; }
    }

    // Test 6: output is stable (formatting twice changes nothing)
    {
        const char* src =
            "def f(a)\n"
            "  if a\n"
            "    b\n"
            "  end\n"
            "end\n";
        std::string once = formatRuby(src);
        Parser parser2;
        ProgramNode* ast2 = parser2.parse(once.c_str(), once.size(), "ruby");
        std::string twice;
        if (ast2 != NULL)
        {
            OutputBuffer out(8192);
            Config config;
            FormatterVisitor* formatter = FormatterFactory::create("ruby", out, config);
            formatter->format(ast2);
            twice = out.toString();
            delete formatter;
        }
        bool ok = !once.empty() && once == twice;
        check(ok, "idempotent", once.c_str());
        if (ok) { passed++; }
    }

    // Test 7: %-literals are opaque (= and ; inside never split or assign)
    {
        const char* src = "puts %w[a=b]\n";
        std::string out = formatRuby(src);
        bool ok = out.find("puts %w[a=b]") != std::string::npos;
        check(ok, "percent literal verbatim", out.c_str());
        if (ok) { passed++; }
    }

    // Test 8: CRLF sources format identically (bytes generated at runtime)
    {
        std::string src = "def f(a)\r\n  if a\r\n    b\r\n  end\r\nend\r\n";
        Parser parser;
        ProgramNode* ast = parser.parse(src.c_str(), src.size(), "ruby");
        std::string out;
        bool ok = ast != NULL;
        if (ok)
        {
            OutputBuffer buf(8192);
            Config config;
            FormatterVisitor* formatter = FormatterFactory::create("ruby", buf, config);
            formatter->format(ast);
            out = buf.toString();
            delete formatter;
        }
        if (ok && out.find("def f(a)") == std::string::npos) { ok = false; }
        if (ok && out.find("if a") == std::string::npos) { ok = false; }
        check(ok, "crlf source", out.c_str());
        if (ok) { passed++; }
    }

    // Test 9+: combinatorial tables (each row separately verified)
    {
        static const Case defs[] = {
            { "def f\nend\n", { "def f", "end", NULL } },
            { "def f()\nend\n", { "def f()", NULL } },
            { "def f(a)\n  a\nend\n", { "def f(a)", "a", NULL } },
            { "def f(a, b)\n  a\nend\n", { "def f(a, b)", NULL } },
            { "def f(a = 1)\n  a\nend\n", { "def f(a = 1)", NULL } },
            { "def f(a, b = 1, *c, d:, e: 2, &blk)\n  a\nend\n", { "def f(a, b = 1, *c, d:, e: 2, &blk)", NULL } },
            { "def self.f\n  1\nend\n", { "def self.f", NULL } },
            { "def f=(v)\n  @v = v\nend\n", { "def f=(v)", NULL } },
            { "def f = 42\n", { "def f = 42", NULL } },
            { "def f(x) = x * 2\n", { "def f(x) = x * 2", NULL } },
            { "def f; 42; end\n", { "def f", "42", "end", NULL } },
            { "def outer\n  def inner\n    1\n  end\nend\n", { "def outer", "def inner", NULL } },
            { "def f # about\n  1\nend\n", { "def f", "1", NULL } },
            { "def f(a, // comment\n  b)\n  a\nend\n", { "def f(a,", "b)", NULL } },
        };
        runCases("def", defs, sizeof(defs) / sizeof(defs[0]), passed);
        static const Case classes[] = {
            { "class A\nend\n", { "class A", "end", NULL } },
            { "class A < B\nend\n", { "class A < B", NULL } },
            { "class A::B < C\nend\n", { "class A::B < C", NULL } },
            { "class << self\n  1\nend\n", { "class << self", NULL } },
            { "class A # note\n  1\nend\n", { "class A", NULL } },
            { "module M\nend\n", { "module M", NULL } },
            { "module A::B\nend\n", { "module A::B", NULL } },
            { "class O\n  class I\n  end\nend\n", { "class O", "class I", NULL } },
            { "class A\n  def m\n    1\n  end\nend\n", { "class A", "def m", NULL } },
            { "class K; end\n", { "class K", "end", NULL } },
        };
        runCases("class/module", classes, sizeof(classes) / sizeof(classes[0]), passed);
        static const Case conds[] = {
            { "if a\n  b\nend\n", { "if a", "b", "end", NULL } },
            { "if a\n  b\nelsif c\n  d\nelse\n  e\nend\n", { "if a", "elsif c", "else", NULL } },
            { "unless a\n  b\nend\n", { "unless a", NULL } },
            { "unless a\n  b\nelse\n  c\nend\n", { "unless a", "else", NULL } },
            { "if a then b end\n", { "if a", "b", "end", NULL } },
            { "if a then b else c end\n", { "if a", "else", "c", NULL } },
            { "foo if bar\n", { "foo if bar", NULL } },
            { "foo unless bar\n", { "foo unless bar", NULL } },
            { "x = y ? a : b\n", { "x = y ? a : b", NULL } },
            { "if tired?\n  rest\nend\n", { "if tired?", NULL } },
            { "if (a && b)\n  c\nend\n", { "if (a && b)", NULL } },
            { "if !x\n  y\nend\n", { "if !x", NULL } },
            { "if defined?(Foo)\n  1\nend\n", { "if defined?(Foo)", NULL } },
            { "if a\n  if b\n    c\n  end\nend\n", { "if a", "if b", NULL } },
        };
        runCases("if", conds, sizeof(conds) / sizeof(conds[0]), passed);
        static const Case cases[] = {
            { "case x\nwhen 1\n  a\nend\n", { "case x", "when 1", "a", NULL } },
            { "case x\nwhen 1, 2\n  a\nend\n", { "when 1, 2", NULL } },
            { "case x\nwhen 1 then a\nend\n", { "when 1", "a", NULL } },
            { "case x\nwhen 1\n  a\nwhen 2\n  b\nelse\n  c\nend\n", { "when 2", "else", "c", NULL } },
            { "case\nwhen x > 1\n  big\nend\n", { "case", "when x > 1", NULL } },
            { "case x\nin [a, b]\n  use(a)\nend\n", { "in [a, b]", NULL } },
            { "case x\nin {k:} then use(k)\nend\n", { "in {k:}", "use(k)", NULL } },
            { "case x\nin Integer => n\n  use(n)\nend\n", { "in Integer => n", NULL } },
            { "case x\nelse\n  d\nend\n", { "case x", "else", "d", NULL } },
        };
        runCases("case", cases, sizeof(cases) / sizeof(cases[0]), passed);
        static const Case begins[] = {
            { "begin\n  a\nrescue\n  b\nend\n", { "begin", "rescue", "b", NULL } },
            { "begin\n  a\nrescue E\n  b\nend\n", { "rescue E", NULL } },
            { "begin\n  a\nrescue E => e\n  b\nend\n", { "rescue E => e", NULL } },
            { "begin\n  a\nrescue E1\n  b\nrescue E2\n  c\nend\n", { "rescue E1", "rescue E2", NULL } },
            { "begin\n  a\nrescue\n  b\nelse\n  c\nend\n", { "rescue", "else", "c", NULL } },
            { "begin\n  a\nensure\n  b\nend\n", { "begin", "ensure", NULL } },
            { "begin\n  a\nrescue\n  b\nensure\n  c\nend\n", { "rescue", "ensure", NULL } },
            { "begin\n  a\nend\n", { "begin", "a", "end", NULL } },
            { "begin; a; rescue; b; end\n", { "begin", "rescue", "b", "end", NULL } },
            { "def m\n  work\nrescue\n  fix\nend\n", { "def m", "rescue", "fix", NULL } },
            { "def m\n  work\nrescue E\n  fix\nelse\n  ok\nensure\n  done\nend\n", { "rescue E", "else", "ensure", NULL } },
            { "def m\n  work\nensure\n  always\nend\n", { "def m", "ensure", NULL } },
            { "begin # note\n  a\nend\n", { "begin", "a", NULL } },
        };
        runCases("begin", begins, sizeof(begins) / sizeof(begins[0]), passed);
        static const Case loops[] = {
            { "while a\n  b\nend\n", { "while a", "b", NULL } },
            { "until a\n  b\nend\n", { "until a", NULL } },
            { "while a do b end\n", { "while a", "b", NULL } },
            { "for x in y\n  use(x)\nend\n", { "for x in y", NULL } },
            { "for x in y do use(x) end\n", { "for x in y", "use(x)", NULL } },
            { "foo while bar\n", { "foo while bar", NULL } },
            { "foo until bar\n", { "foo until bar", NULL } },
            { "while true\n  break\nend\n", { "while true", "break", NULL } },
            { "loop do\n  break\nend\n", { "loop do", "break", "end", NULL } },
            { "3.times do |i|\n  puts i\nend\n", { "3.times do |i|", "puts i", NULL } },
            { "items.each do |x|\n  use(x)\nend\n", { "items.each do |x|", NULL } },
            { "m.each do\n  1\nend\n", { "m.each do", NULL } },
            { "outer do\n  inner do |x|\n    use(x)\n  end\nend\n", { "outer do", "inner do |x|", NULL } },
            { "[1].each { |i| puts i }\n", { "[1].each { |i| puts i }", NULL } },
            { "y = list.map { |i| i + 1 }\n", { "y = list.map { |i| i + 1 }", NULL } },
        };
        runCases("loops/blocks", loops, sizeof(loops) / sizeof(loops[0]), passed);
        static const Case stmts[] = {
            { "require \"json\"\n", { "require \"json\"", NULL } },
            { "require('digest')\n", { "require \"digest\"", NULL } },
            { "require_relative \"helper\"\n", { "require_relative \"helper\"", NULL } },
            { "x = 1\n", { "x = 1", NULL } },
            { "X = 1\n", { "X = 1", NULL } },
            { "@x = 1\n", { "@x = 1", NULL } },
            { "@@x = 1\n", { "@@x = 1", NULL } },
            { "$x = 1\n", { "$x = 1", NULL } },
            { "x, y = 1, 2\n", { "x, y = 1, 2", NULL } },
            { "x += 1\n", { "x += 1", NULL } },
            { "x ||= 1\n", { "x ||= 1", NULL } },
            { "x &&= 1\n", { "x &&= 1", NULL } },
            { "a.b = 1\n", { "a.b = 1", NULL } },
            { "a[0] = 1\n", { "a[0] = 1", NULL } },
            { "return\n", { "return", NULL } },
            { "return x\n", { "return x", NULL } },
            { "return x if y\n", { "return x if y", NULL } },
            { "raise E\n", { "raise E", NULL } },
            { "raise E, 'msg'\n", { "raise E, 'msg'", NULL } },
            { "raise\n", { "raise", NULL } },
            { "throw :ball\n", { "throw :ball", NULL } },
            { "break\n", { "break", NULL } },
            { "next\n", { "next", NULL } },
            { "redo\n", { "redo", NULL } },
            { "retry\n", { "retry", NULL } },
            { "yield x\n", { "yield x", NULL } },
            { "super\n", { "super", NULL } },
            { "super(x)\n", { "super(x)", NULL } },
            { "defined?(Foo)\n", { "defined?(Foo)", NULL } },
            { "puts x\n", { "puts x", NULL } },
            { "puts(x)\n", { "puts(x)", NULL } },
            { "foo :bar\n", { "foo :bar", NULL } },
            { "foo a: 1\n", { "foo a: 1", NULL } },
            { "private\n", { "private", NULL } },
            { "private :m\n", { "private :m", NULL } },
            { "attr_reader :a, :b\n", { "attr_reader :a, :b", NULL } },
            { "include M\n", { "include M", NULL } },
            { "extend self\n", { "extend self", NULL } },
            { "alias a b\n", { "alias a b", NULL } },
            { "alias_method :a, :b\n", { "alias_method :a, :b", NULL } },
            { "foo\n", { "foo", NULL } },
            { "foo.bar\n", { "foo.bar", NULL } },
            { "x = foo rescue bar\n", { "x = foo rescue bar", NULL } },
        };
        runCases("statements", stmts, sizeof(stmts) / sizeof(stmts[0]), passed);
        static const Case strs[] = {
            { "x = \"a\"\n", { "\"a\"", NULL } },
            { "x = 'a'\n", { "'a'", NULL } },
            { "x = \"a#{b}c\"\n", { "\"a#{b}c\"", NULL } },
            { "x = 'a#{b}'\n", { "'a#{b}'", NULL } },
            { "x = \"semi; inside\"\n", { "\"semi; inside\"", NULL } },
            { "x = %w[a b]\n", { "%w[a b]", NULL } },
            { "x = %w[a=b;c]\n", { "%w[a=b;c]", NULL } },
            { "x = %w[then do end]\n", { "%w[then do end]", NULL } },
            { "x = %w[<<E]\n", { "%w[<<E]", NULL } },
            { "x = %w[(n)]\n", { "%w[(n)]", NULL } },
            { "x = %i[a b]\n", { "%i[a b]", NULL } },
            { "x = %q(a;b)\n", { "%q(a;b)", NULL } },
            { "x = %Q{a#{b}}\n", { "%Q{a#{b}}", NULL } },
            { "x = %r{a;b}i\n", { "%r{a;b}i", NULL } },
            { "x = %s<sym>\n", { "%s<sym>", NULL } },
            { "x = %x{echo}\n", { "%x{echo}", NULL } },
            { "x = %!bang!\n", { "%!bang!", NULL } },
            { "x = :sym\n", { ":sym", NULL } },
            { "x = :sym?\n", { ":sym?", NULL } },
            { "x = :\"q s\"\n", { ":\"q s\"", NULL } },
            { "x = /re/\n", { "/re/", NULL } },
            { "x = /a;b/im\n", { "/a;b/im", NULL } },
            { "x = a % b\n", { "a % b", NULL } },
            { "x = \"multi\nline\"\n", { "\"multi", "line\"", NULL } },
        };
        runCases("strings", strs, sizeof(strs) / sizeof(strs[0]), passed);
        static const Case misc[] = {
            { "# hello\nx = 1\n", { "# hello", "x = 1", NULL } },
            { "x = 1  # trailing\n", { "x = 1", "# trailing", NULL } },
            { "#!/usr/bin/env ruby\nx = 1\n", { "#!/usr/bin/env ruby", NULL } },
            { "def f # about\n  1\nend\n", { "def f", "1", "# about", NULL } },
            { "x = 0xFF\n", { "0xFF", NULL } },
            { "x = 1_000\n", { "1_000", NULL } },
            { "x = nil\n", { "nil", NULL } },
            { "x = true\n", { "true", NULL } },
            { "x = [1,\n     2]\n", { "[1,", "2]", NULL } },
            { "x = { a: 1 }\n", { "{ a: 1 }", NULL } },
            { "foo; bar\n", { "foo", "bar", NULL } },
            { "x = 1; y = 2\n", { "x = 1", "y = 2", NULL } },
            { "end\n", { "end", NULL } },
            { "else\n", { "else", NULL } },
            { "when 1\n", { "when 1", NULL } },
            { "rescue\n", { "rescue", NULL } },
            { "x.end\n", { "x.end", NULL } },
            { "trend\n", { "trend", NULL } },
            { "def f\n  x\n", { "def f", "x", "end", NULL } },
        };
        runCases("misc/invalid", misc, sizeof(misc) / sizeof(misc[0]), passed);
    }

    // Massive generated combinatorial
    {
        char src[512];
        int gen = 0;
        for (int i = 0; i < 60; i++)
        {
            snprintf(src, sizeof(src), "def m%d(a%d, b%d = %d)\n  a%d\nend\n", i, i, i, i, i);
            std::string out = formatRuby(src);
            char w[32]; snprintf(w, sizeof(w), "def m%d(", i);
            bool ok = !out.empty() && out.find(w) != std::string::npos && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-def %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        for (int i = 0; i < 40; i++)
        {
            snprintf(src, sizeof(src), "class C%d < B%d\n  def m\n    1\n  end\nend\n", i, i);
            std::string out = formatRuby(src);
            char w[48]; snprintf(w, sizeof(w), "class C%d < B%d", i, i);
            bool ok = out.find(w) != std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-class %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        for (int i = 0; i < 30; i++)
        {
            snprintf(src, sizeof(src), "if c%d\n  f%d\nelsif d%d\n  g%d\nelse\n  h%d\nend\n", i, i, i, i, i);
            std::string out = formatRuby(src);
            bool ok = out.find("elsif") != std::string::npos && out.find("else") != std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-if %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        for (int i = 0; i < 30; i++)
        {
            int k = i % 5;
            if (k == 0) { snprintf(src, sizeof(src), "case x%d\nwhen %d\n  a\nelse\n  b\nend\n", i, i); }
            else if (k == 1) { snprintf(src, sizeof(src), "begin\n  a%d\nrescue E%d => e\n  b\nensure\n  c\nend\n", i, i); }
            else if (k == 2) { snprintf(src, sizeof(src), "while w%d\n  s%d\nend\n", i, i); }
            else if (k == 3) { snprintf(src, sizeof(src), "items%d.each do |x|\n  use(x)\nend\n", i); }
            else { snprintf(src, sizeof(src), "y%d = list.map { |j| j + %d }\n", i, i); }
            std::string out = formatRuby(src);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-ctl %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        for (int i = 0; i < 40; i++)
        {
            int k = i % 4;
            if (k == 0) { snprintf(src, sizeof(src), "x%d = \"s%d#{v%d}\"\n", i, i, i); }
            else if (k == 1) { snprintf(src, sizeof(src), "x%d = %%w[a%d b%d]\n", i, i, i); }
            else if (k == 2) { snprintf(src, sizeof(src), "x%d = :sym%d\n", i, i); }
            else { snprintf(src, sizeof(src), "foo%d a: %d\n", i, i); }
            std::string out = formatRuby(src);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-str %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        for (int i = 0; i < 30; i++)
        {
            int k = i % 3;
            if (k == 0) { snprintf(src, sizeof(src), "# c%d\nx%d = 1\n", i, i); }
            else if (k == 1) { snprintf(src, sizeof(src), "x%d = %d  # trail %d\n", i, i, i); }
            else { snprintf(src, sizeof(src), "require \"lib%d\"\n", i); }
            std::string out = formatRuby(src);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-cmt %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        // heredoc + percent + operators sampled
        for (int i = 0; i < 20; i++)
        {
            snprintf(src, sizeof(src), "y = foo(<<~EOS%d, %d)\n  hello%d;\nEOS%d\n", i, i, i, i);
            std::string out = formatRuby(src);
            char w[32]; snprintf(w, sizeof(w), "<<~EOS%d", i);
            bool ok = out.find(w) != std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-heredoc %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        for (int i = 0; i < 20; i++)
        {
            snprintf(src, sizeof(src), "x = a%d %% b%d\n", i, i);
            std::string out = formatRuby(src);
            bool ok = out.find("%") != std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-op %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        printf("  PASS: generated (%d rows)\n", gen);
        passed++;
        for (int i = 0; i < 60; i++)
        {
            char src2[256];
            int k = i % 6;
            if (k == 0) { snprintf(src2, sizeof(src2), "def s%d\n  super(%d)\nend\n", i, i); }
            else if (k == 1) { snprintf(src2, sizeof(src2), "x%d ||= %d\n", i, i); }
            else if (k == 2) { snprintf(src2, sizeof(src2), "puts \"v%d\"\n", i); }
            else if (k == 3) { snprintf(src2, sizeof(src2), "attr_reader :a%d, :b%d\n", i, i); }
            else if (k == 4) { snprintf(src2, sizeof(src2), "loop do\n  break if c%d\nend\n", i); }
            else { snprintf(src2, sizeof(src2), "x%d = [1,\n     %d]\n", i, i); }
            std::string out = formatRuby(src2);
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            rowCases++;
            if (ok) { gen++; passed++; } else { printf("  FAIL: [gen-extra %d] out=[%s]\n", i, out.c_str()); failures++; }
        }
        printf("  PASS: generated-extra (60 rows)\n");
        passed++;
    }

    // Empty / comments-only / invalid verbatim
    {
        static const Case empties[] = {
            { "# only\n", { "# only", NULL } },
            { "#!/usr/bin/env ruby\n", { "#!/usr/bin/env ruby", NULL } },
            { "=begin\nDoc.\n=end\nx = 1\n", { "x = 1", NULL } },
            { "x = 1  # t\n", { "x = 1", "# t", NULL } },
            { "__END__\ntail;\n", { "__END__", "tail;", NULL } },
            { "x = nil\n", { "nil", NULL } },
            { "foo; bar\n", { "foo", "bar", NULL } },
            { "end\n", { "end", NULL } },
        };
        runCases("empty/comments-only", empties, sizeof(empties) / sizeof(empties[0]), passed);
        {
            std::string out = formatRuby("");
            bool ok = out.find("placeholder") == std::string::npos;
            check(ok, "empty-input", out.c_str());
            if (ok) { passed++; }
        }
        {
            std::string out = formatRuby("def f(:\n  1\nend\n");
            bool ok = !out.empty() && out.find("placeholder") == std::string::npos;
            check(ok, "invalid-verbatim", out.c_str());
            if (ok) { passed++; }
        }
        {
            const char* s = "def f(a)\n  if a\n    b\n  end\nend\n";
            std::string once = formatRuby(s);
            Parser p2;
            ProgramNode* a2 = p2.parse(once.c_str(), once.size(), "ruby");
            std::string twice;
            if (a2 != NULL)
            {
                OutputBuffer ob(8192);
                Config cfg;
                FormatterVisitor* fm = FormatterFactory::create("ruby", ob, cfg);
                fm->format(a2);
                twice = ob.toString();
                delete fm;
            }
            bool ok = !once.empty() && once == twice;
            check(ok, "idempotent2", once.c_str());
            if (ok) { passed++; }
        }
    }

    printf("\n========================================\n");
    printf("RubyFormatter tests: %d passed, %d failed (%d table rows)\n", passed, failures, rowCases);
    printf("========================================\n");

    if (failures > 0) { return 1; }
    return 0;
}
