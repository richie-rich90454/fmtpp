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

    printf("\n========================================\n");
    printf("RubyFormatter tests: %d passed, %d failed\n", passed, failures);
    printf("========================================\n");

    if (failures > 0) { return 1; }
    return 0;
}
