// Easter egg: Ruby end-to-end - real files in, proof out
//
// Every tests/e2e/ruby/*.rb file is read from disk, parsed and
// formatted. Each file self-describes its markers in leading
// "# e2e-markers: a, b" lines. Three checks per file:
//   1. markers present, no "placeholder" leak,
//   2. token multiset preservation (the formatter invents no text;
//      dropped "then"/"do" header words are forgiven),
//   3. idempotency (formatting twice is byte-identical).
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/FormatterFactory.hpp"
#include "fmtpp/FormatterVisitor.hpp"
#include "fmtpp/ast/ProgramNode.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

namespace {

std::string corpusDir()
{
    std::string self(__FILE__);
    size_t slash = self.find_last_of("/\\");
    std::string dir = (slash == std::string::npos) ? std::string(".") : self.substr(0, slash);
    return dir + "/e2e/ruby";
}

bool listFiles(const std::string& dir, const char* ext, std::vector<std::string>& out)
{
#ifdef _WIN32
    std::string pattern = dir + "/*" + ext;
    WIN32_FIND_DATAA found;
    HANDLE handle = FindFirstFileA(pattern.c_str(), &found);
    if (handle == INVALID_HANDLE_VALUE) { return false; }
    do
    {
        out.push_back(dir + "/" + found.cFileName);
    } while (FindNextFileA(handle, &found));
    FindClose(handle);
#else
    DIR* dp = opendir(dir.c_str());
    if (dp == NULL) { return false; }
    struct dirent* entry;
    while ((entry = readdir(dp)) != NULL)
    {
        std::string name = entry->d_name;
        if (name.size() > strlen(ext)
            && name.compare(name.size() - strlen(ext), strlen(ext), ext) == 0)
        {
            out.push_back(dir + "/" + name);
        }
    }
    closedir(dp);
#endif
    std::sort(out.begin(), out.end());
    return true;
}

bool readFile(const std::string& path, std::string& content)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (f == NULL) { return false; }
    char buf[8192];
    size_t got;
    while ((got = fread(buf, 1, sizeof(buf), f)) > 0) { content.append(buf, got); }
    fclose(f);
    return true;
}

std::string formatSource(const char* source, size_t len)
{
    Parser parser;
    ProgramNode* ast = parser.parse(source, len, "ruby");
    if (ast == NULL) { return std::string(); }
    OutputBuffer out(131072);
    Config config;
    FormatterVisitor* formatter = FormatterFactory::create("ruby", out, config);
    formatter->format(ast);
    std::string result = out.toString();
    delete formatter;
    return result;
}

bool isWordChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '_' || (c & 0x80) != 0;
}

// Word-ish tokens: idents/numbers, @/$-sigils, :-symbols, string
// literals with one quote layer stripped ('x' and "x" match), and
// Ruby %-literals as atomic spans. Operators, punctuation and
// whitespace never count.
bool isAlphaNum(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

bool percentToken(const std::string& src, size_t& i, std::map<std::string, int>& counts)
{
    // caller guarantees src[i] == '%'; mirrors the parser's skipper
    size_t n = src.size();
    size_t q = i + 1;
    if (q < n && src[q] == '%') { counts["%%"]++; i += 2; return true; }
    if (q < n && isAlphaNum(src[q]) && !(src[q] >= '0' && src[q] <= '9'))
    {
        char m = src[q];
        if (m != 'q' && m != 'Q' && m != 'w' && m != 'W' && m != 'i'
            && m != 'I' && m != 'r' && m != 's' && m != 'x' && m != 'X')
        {
            return false;
        }
        q++;
    }
    if (q >= n) { return false; }
    char open = src[q];
    if (open == ' ' || open == '\t' || open == '\n' || open == '\r'
        || open == '=' || isAlphaNum(open) || open == '_')
    {
        return false;
    }
    char close = open;
    if (open == '(') { close = ')'; }
    else if (open == '[') { close = ']'; }
    else if (open == '{') { close = '}'; }
    else if (open == '<') { close = '>'; }
    int depth = 0;
    q++;
    while (q < n)
    {
        char ch = src[q];
        if (ch == '\\' && q + 1 < n) { q += 2; continue; }
        if (open == close)
        {
            if (ch == close) { counts[src.substr(i, q + 1 - i)]++; i = q + 1; return true; }
        }
        else if (ch == open) { depth++; }
        else if (ch == close)
        {
            if (depth == 0) { counts[src.substr(i, q + 1 - i)]++; i = q + 1; return true; }
            depth--;
        }
        q++;
    }
    counts[src.substr(i)]++;
    i = q;
    return true;
}

void tokenize(const std::string& src, std::map<std::string, int>& counts)
{
    size_t i = 0;
    size_t n = src.size();
    while (i < n)
    {
        char c = src[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f')
        {
            i++;
            continue;
        }
        if (c == '%' && percentToken(src, i, counts)) { continue; }
        if (c == '"' || c == '\'' || c == '`')
        {
            // ponytail: naive triple scan is enough; both sides tokenize
            // the same text the same way, escapes included. A glued
            // alpha prefix (f"...", r'...') belongs to the string token.
            char q = c;
            std::string prefix;
            size_t back = i;
            while (back > 0 && (src[back - 1] == 'r' || src[back - 1] == 'R'
                || src[back - 1] == 'b' || src[back - 1] == 'B'
                || src[back - 1] == 'f' || src[back - 1] == 'F'
                || src[back - 1] == 'u' || src[back - 1] == 'U'))
            {
                back--;
            }
            if (back < i && (back == 0 || !isWordChar(src[back - 1])))
            {
                prefix = src.substr(back, i - back);
            }
            bool triple = (i + 2 < n && src[i + 1] == q && src[i + 2] == q);
            size_t j = i + (triple ? 3 : 1);
            std::string inner;
            while (j < n)
            {
                if (src[j] == '\\' && j + 1 < n) { inner += src[j]; inner += src[j + 1]; j += 2; continue; }
                if (src[j] == q)
                {
                    if (triple)
                    {
                        if (j + 2 < n && src[j + 1] == q && src[j + 2] == q)
                        {
                            j += 3;
                            break;
                        }
                        inner += q;
                        j++;
                        continue;
                    }
                    j++;
                    break;
                }
                inner += src[j];
                j++;
            }
            // ponytail: unterminated strings still count (both sides agree)
            counts[prefix + "\x01" + inner]++;
            i = j;
            continue;
        }        if (c == '#')
        {
            // ponytail: comment words count (comments print verbatim-ish)
            i++;
            while (i < n && src[i] != '\n')
            {
                if (isWordChar(src[i]))
                {
                    size_t s = i;
                    while (i < n && isWordChar(src[i])) { i++; }
                    counts[src.substr(s, i - s)]++;
                }
                else { i++; }
            }
            continue;
        }
        if ((c == '@' || c == '$') && i + 1 < n && isWordChar(src[i + 1]))
        {
            size_t s = i;
            i++;
            while (i < n && isWordChar(src[i])) { i++; }
            counts[src.substr(s, i - s)]++;
            continue;
        }
        if (c == ':' && i + 1 < n && isWordChar(src[i + 1])
            && !(i > 0 && src[i - 1] == ':'))
        {
            size_t s = i;
            i++;
            while (i < n && isWordChar(src[i])) { i++; }
            counts[src.substr(s, i - s)]++;
            continue;
        }
        if (isWordChar(c))
        {
            size_t s = i;
            while (i < n && isWordChar(src[i])) { i++; }
            // ponytail: alpha prefixes glued to a quote belong to strings
            if (i < n && (src[i] == '"' || src[i] == '\''))
            {
                bool allPrefix = true;
                for (size_t k = s; k < i; k++)
                {
                    char pc = src[k];
                    if (pc != 'r' && pc != 'R' && pc != 'b' && pc != 'B'
                        && pc != 'f' && pc != 'F' && pc != 'u' && pc != 'U')
                    {
                        allPrefix = false;
                        break;
                    }
                }
                if (allPrefix) { continue; }
            }
            counts[src.substr(s, i - s)]++;
            continue;
        }
        i++;
    }
}

int failures = 0;

bool isEdgeName(const std::string& name)
{
    return name.find("empty") != std::string::npos
        || name.find("comment") != std::string::npos
        || name.find("invalid") != std::string::npos
        || name.find("edge") != std::string::npos;
}

// Code-only view for keyword probes: strips strings and #-comments.
std::string codeOnly(const std::string& src)
{
    std::string out;
    out.reserve(src.size());
    size_t i = 0;
    size_t n = src.size();
    while (i < n)
    {
        char c = src[i];
        if (c == '#')
        {
            while (i < n && src[i] != '\n') { i++; }
            continue;
        }
        if (c == '"' || c == '\'' || c == '`')
        {
            char q = c;
            bool triple = (i + 2 < n && src[i + 1] == q && src[i + 2] == q);
            i += triple ? 3 : 1;
            while (i < n)
            {
                if (src[i] == '\\' && i + 1 < n) { i += 2; continue; }
                if (src[i] == q)
                {
                    if (triple)
                    {
                        if (i + 2 < n && src[i + 1] == q && src[i + 2] == q) { i += 3; break; }
                        i++;
                        continue;
                    }
                    i++;
                    break;
                }
                i++;
            }
            continue;
        }
        out += c;
        i++;
    }
    return out;
}

void check(bool ok, const std::string& file, const std::string& detail)
{
    if (ok) { printf("  PASS: %s\n", file.c_str()); }
    else { printf("  FAIL: %s (%s)\n", file.c_str(), detail.c_str()); failures++; }
}

}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    std::vector<std::string> files;
    if (!listFiles(corpusDir(), ".rb", files) || files.empty())
    {
        printf("FAIL: no corpus files in %s\n", corpusDir().c_str());
        return 1;
    }

    for (size_t fi = 0; fi < files.size(); fi++)
    {
        const std::string& path = files[fi];
        size_t slash = path.find_last_of("/\\");
        std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
        std::string source;
        if (!readFile(path, source))
        {
            check(false, name, "unreadable");
            continue;
        }
        if (source.empty())
        {
            std::string out = formatSource("", 0);
            check(out.empty(), name, "empty file should format to empty");
            if (out.empty()) { passed++; }
            continue;
        }
        std::string out = formatSource(source.c_str(), source.size());
        bool ok = true;
        std::string why;
        if (out.empty()) { ok = false; why = "empty output"; }
        if (ok && out.find("placeholder") != std::string::npos)
        {
            ok = false;
            why = "placeholder leak";
        }
        // markers self-described in leading "# e2e-markers: a, b" lines
        if (ok)
        {
            size_t pos = 0;
            while (pos < source.size())
            {
                size_t eol = source.find('\n', pos);
                std::string line = source.substr(pos, eol == std::string::npos ? eol : eol - pos);
                std::string t = line;
                size_t a = t.find_first_not_of(" \t\r");
                if (a != std::string::npos) { t = t.substr(a); }
                const char* prefix = "# e2e-markers:";
                if (t.compare(0, strlen(prefix), prefix) == 0)
                {
                    std::string rest = t.substr(strlen(prefix));
                    size_t s = 0;
                    while (s <= rest.size())
                    {
                        size_t c = rest.find(',', s);
                        std::string m = rest.substr(s, c == std::string::npos ? c : c - s);
                        size_t b = m.find_first_not_of(" \t\r");
                        size_t e = m.find_last_not_of(" \t\r");
                        if (b != std::string::npos) { m = m.substr(b, e - b + 1); }
                        else { m.clear(); }
                        if (!m.empty() && out.find(m) == std::string::npos)
                        {
                            ok = false;
                            why = std::string("marker missing: ") + m;
                            break;
                        }
                        if (c == std::string::npos) { break; }
                        s = c + 1;
                    }
                    if (!ok) { break; }
                }
                else if (!t.empty() && t[0] != '#') { break; }
                if (eol == std::string::npos) { break; }
                pos = eol + 1;
            }
        }
        // keyword presence: code words survive the round trip
        if (ok && !isEdgeName(name))
        {
            std::string code = codeOnly(source);
            static const char* kws[] = { "def", "class", "end", "require" };
            for (size_t k = 0; k < sizeof(kws) / sizeof(kws[0]); k++)
            {
                if (code.find(kws[k]) == std::string::npos) { continue; }
                if (out.find(kws[k]) == std::string::npos)
                {
                    ok = false;
                    why = std::string("keyword dropped: ") + kws[k];
                    break;
                }
            }
        }
        // token multiset preservation: output invents nothing and drops
        // nothing but the redundant header words "then" and "do". An added
        // "end" for unclosed constructs is forgiven (intended repair).
        if (ok)
        {
            std::map<std::string, int> inCounts;
            std::map<std::string, int> outCounts;
            tokenize(source, inCounts);
            tokenize(out, outCounts);
            for (std::map<std::string, int>::const_iterator it = inCounts.begin();
                it != inCounts.end(); ++it)
            {
                std::map<std::string, int>::const_iterator f = outCounts.find(it->first);
                int have = (f == outCounts.end()) ? 0 : f->second;
                if (have < it->second && it->first != "then" && it->first != "do")
                {
                    ok = false;
                    why = std::string("dropped token: ") + it->first;
                    break;
                }
            }
            if (ok)
            {
                for (std::map<std::string, int>::const_iterator it = outCounts.begin();
                    it != outCounts.end(); ++it)
                {
                    std::map<std::string, int>::const_iterator f = inCounts.find(it->first);
                    int have = (f == inCounts.end()) ? 0 : f->second;
                    if (it->second > have && it->first != "end")
                    {
                        ok = false;
                        why = std::string("invented token: ") + it->first;
                        break;
                    }
                }
            }
        }
        // idempotency: formatting twice is byte-identical
        if (ok)
        {
            std::string twice = formatSource(out.c_str(), out.size());
            if (twice != out)
            {
                ok = false;
                why = "not idempotent";
            }
        }
        check(ok, name, why);
        if (ok) { passed++; }
    }

    printf("\n========================================\n");
    printf("Ruby e2e: %d passed, %d failed\n", passed, failures);
    printf("========================================\n");

    if (failures > 0) { return 1; }
    return 0;
}
