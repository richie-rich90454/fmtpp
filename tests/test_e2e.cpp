// End-to-end over every language - real files in, proof out.
//
// Phase A: explicit fixture table, tests/fixtures/sample.<ext>.
// Phase B: auto-discovered scan of tests/fixtures/* + tests/e2e/**/*
//          (new files are picked up without code changes).
// Phase C: CLI-level run of the built fmtpp binary on temp copies.
//
// Per normal file: non-empty, no-placeholder, idempotency, keyword
// presence, brace-balance sanity, wrong-language non-crash.
// Edge files (empty / comments-only / invalid / edge) only prove
// graceful handling: no crash, no placeholder, idempotent.
#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdlib>
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
#include <sys/stat.h>
#endif

namespace {

std::string repoDir()
{
    std::string self(__FILE__);
    size_t slash = self.find_last_of("/\\");
    std::string dir = (slash == std::string::npos) ? std::string(".") : self.substr(0, slash);
    size_t slash2 = dir.find_last_of("/\\");
    if (slash2 != std::string::npos) { dir = dir.substr(0, slash2); }
    return dir;
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

bool writeFile(const std::string& path, const std::string& content)
{
    FILE* f = fopen(path.c_str(), "wb");
    if (f == NULL) { return false; }
    if (!content.empty()) { fwrite(content.c_str(), 1, content.size(), f); }
    fclose(f);
    return true;
}

std::string formatSource(const char* source, size_t len, const std::string& language)
{
    Parser parser;
    ProgramNode* ast = parser.parse(source, len, language);
    if (ast == NULL) { return std::string(); }
    OutputBuffer out(131072);
    Config config;
    FormatterVisitor* formatter = FormatterFactory::create(language, out, config);
    formatter->format(ast);
    std::string result = out.toString();
    delete formatter;
    return result;
}

std::string safeFormat(const std::string& source, const std::string& language, bool& crashed)
{
    crashed = false;
    try
    {
        return formatSource(source.c_str(), source.size(), language);
    }
    catch (...)
    {
        crashed = true;
        return std::string();
    }
}

std::string detectLanguage(const std::string& path)
{
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) { return std::string(); }
    std::string ext = path.substr(dot + 1);
    for (size_t i = 0; i < ext.size(); i++) { ext[i] = (char)tolower((unsigned char)ext[i]); }
    if (ext == "js" || ext == "mjs" || ext == "cjs" || ext == "jsx") { return "js"; }
    if (ext == "ts" || ext == "tsx" || ext == "mts" || ext == "cts") { return "ts"; }
    if (ext == "java") { return "java"; }
    if (ext == "c" || ext == "h") { return "c"; }
    if (ext == "cpp" || ext == "cc" || ext == "cxx" || ext == "hpp"
        || ext == "hh" || ext == "hxx" || ext == "ino" || ext == "cu") { return "cpp"; }
    if (ext == "cs") { return "cs"; }
    if (ext == "go") { return "go"; }
    if (ext == "rs") { return "rs"; }
    if (ext == "kt" || ext == "kts") { return "kotlin"; }
    if (ext == "swift") { return "swift"; }
    if (ext == "dart") { return "dart"; }
    if (ext == "php" || ext == "phtml") { return "php"; }
    if (ext == "scala" || ext == "sc") { return "scala"; }
    if (ext == "m" || ext == "mm") { return "objc"; }
    if (ext == "zig" || ext == "zon") { return "zig"; }
    if (ext == "d" || ext == "di") { return "d"; }
    if (ext == "groovy" || ext == "gvy") { return "groovy"; }
    if (ext == "sol") { return "solidity"; }
    if (ext == "v" || ext == "vsh" || ext == "vv") { return "v"; }
    if (ext == "glsl" || ext == "vert" || ext == "frag" || ext == "geom"
        || ext == "comp" || ext == "tesc" || ext == "tese") { return "glsl"; }
    if (ext == "html" || ext == "htm" || ext == "xhtml" || ext == "xml") { return "html"; }
    if (ext == "css" || ext == "scss" || ext == "less") { return "css"; }
    if (ext == "json") { return "json"; }
    if (ext == "vue") { return "vue"; }
    if (ext == "py" || ext == "pyw" || ext == "pyi") { return "python"; }
    if (ext == "rb") { return "ruby"; }
    return std::string();
}

std::vector<std::string> keywordsFor(const std::string& language)
{
    std::vector<std::string> kws;
    const char* list = NULL;
    if (language == "c") { list = "int|return|for|while|if|include"; }
    else if (language == "cpp") { list = "class|return|include|namespace|int"; }
    else if (language == "cs") { list = "class|using|namespace|public|return"; }
    else if (language == "java") { list = "class|public|return|import|void"; }
    else if (language == "js") { list = "function|return|import|export|const"; }
    else if (language == "ts") { list = "function|return|interface|export|type"; }
    else if (language == "go") { list = "package|func|import|return|for"; }
    else if (language == "rs") { list = "fn|let|return|struct|match"; }
    else if (language == "kotlin") { list = "fun|class|val|return|package"; }
    else if (language == "swift") { list = "func|class|struct|let|return"; }
    else if (language == "dart") { list = "class|import|void|return|for"; }
    else if (language == "php") { list = "function|class|return|namespace|echo"; }
    else if (language == "scala") { list = "class|def|object|import|val"; }
    else if (language == "objc") { list = "interface|implementation|import|return"; }
    else if (language == "zig") { list = "fn|const|var|return"; }
    else if (language == "d") { list = "module|import|class|void|return"; }
    else if (language == "groovy") { list = "class|def|import|println"; }
    else if (language == "solidity") { list = "contract|function|pragma|import|return"; }
    else if (language == "v") { list = "fn|struct|import|return|module"; }
    else if (language == "glsl") { list = "void|float|vec3|uniform|return"; }
    else if (language == "html") { list = "html|head|body|div|script"; }
    else if (language == "css") { list = "color|margin|import|media"; }
    else if (language == "json") { list = "name|version|dependencies|format"; }
    else if (language == "vue") { list = "template|script|style|import"; }
    else if (language == "python") { list = "def|class|import|return"; }
    else if (language == "ruby") { list = "def|class|end|require"; }
    if (list == NULL) { return kws; }
    std::string s = list;
    size_t start = 0;
    while (start <= s.size())
    {
        size_t bar = s.find('|', start);
        kws.push_back(s.substr(start, bar == std::string::npos ? bar : bar - start));
        if (bar == std::string::npos) { break; }
        start = bar + 1;
    }
    return kws;
}

std::string wrongLang(const std::string& language)
{
    if (language == "python") { return "ruby"; }
    if (language == "ruby") { return "python"; }
    if (language == "js") { return "ts"; }
    if (language == "ts") { return "js"; }
    return "js";
}

// Strip strings and comments so keyword/brace probes only see code.
// Triple-quote aware; #-comments only for python/ruby (else it would
// eat C-style preprocessor lines).
std::string stripNoise(const std::string& src, bool hashComments)
{
    std::string out;
    out.reserve(src.size());
    size_t i = 0;
    size_t n = src.size();
    while (i < n)
    {
        char c = src[i];
        if (c == '/' && i + 1 < n && src[i + 1] == '/')
        {
            while (i < n && src[i] != '\n') { i++; }
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*')
        {
            i += 2;
            while (i + 1 < n && !(src[i] == '*' && src[i + 1] == '/')) { i++; }
            i = (i + 1 < n) ? i + 2 : n;
            continue;
        }
        if (c == '<' && i + 3 < n && src[i + 1] == '!'
            && src[i + 2] == '-' && src[i + 3] == '-')
        {
            i += 4;
            while (i + 2 < n && !(src[i] == '-' && src[i + 1] == '-' && src[i + 2] == '>')) { i++; }
            i = (i + 2 < n) ? i + 3 : n;
            continue;
        }
        if (hashComments && c == '#')
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

bool isEdgeFile(const std::string& name)
{
    std::string lower = name;
    for (size_t i = 0; i < lower.size(); i++) { lower[i] = (char)tolower((unsigned char)lower[i]); }
    return lower.find("empty") != std::string::npos
        || lower.find("comment") != std::string::npos
        || lower.find("invalid") != std::string::npos
        || lower.find("edge") != std::string::npos;
}

void listFilesRec(const std::string& dir, std::vector<std::string>& out)
{
#ifdef _WIN32
    std::string pattern = dir + "\\*";
    WIN32_FIND_DATAA found;
    HANDLE handle = FindFirstFileA(pattern.c_str(), &found);
    if (handle == INVALID_HANDLE_VALUE) { return; }
    do
    {
        std::string name = found.cFileName;
        if (name == "." || name == "..") { continue; }
        std::string full = dir + "/" + name;
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { listFilesRec(full, out); }
        else if (!detectLanguage(full).empty()) { out.push_back(full); }
    } while (FindNextFileA(handle, &found));
    FindClose(handle);
#else
    DIR* dp = opendir(dir.c_str());
    if (dp == NULL) { return; }
    struct dirent* entry;
    while ((entry = readdir(dp)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..") { continue; }
        std::string full = dir + "/" + name;
        DIR* sub = opendir(full.c_str());
        if (sub != NULL)
        {
            closedir(sub);
            listFilesRec(full, out);
        }
        else if (!detectLanguage(full).empty()) { out.push_back(full); }
    }
    closedir(dp);
#endif
}

struct Fixture
{
    const char* file;
    const char* language;
};

int failures = 0;
long checksTotal = 0;
long checksPassed = 0;
int filesTotal = 0;
int filesPassed = 0;

void tally(bool ok)
{
    checksTotal++;
    if (ok) { checksPassed++; }
}

bool checkFile(const std::string& path, const std::string& display, const std::string& language)
{
    filesTotal++;
    std::string source;
    if (!readFile(path, source))
    {
        printf("  FAIL: %s (unreadable)\n", display.c_str());
        failures++;
        return false;
    }
    size_t slash = path.find_last_of("/\\");
    std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    bool edge = isEdgeFile(name) || source.empty();

    bool crashed = false;
    std::string out = safeFormat(source, language, crashed);
    if (crashed)
    {
        printf("  FAIL: %s (formatter crashed)\n", display.c_str());
        failures++;
        return false;
    }
    bool ok = true;
    std::string why;

    bool noPlaceholder = (out.find("placeholder") == std::string::npos);
    tally(noPlaceholder);
    if (!noPlaceholder) { ok = false; why = "placeholder leak"; }

    std::string twice;
    bool idem = false;
    if (ok || !edge)
    {
        bool crashed2 = false;
        twice = safeFormat(out, language, crashed2);
        idem = !crashed2 && twice == out;
        tally(idem);
        if (!idem) { ok = false; why = crashed2 ? "reformat crashed" : "not idempotent"; }
    }

    if (!edge)
    {
        bool nonEmpty = !out.empty();
        tally(nonEmpty);
        if (!nonEmpty) { ok = false; why = "empty output"; }

        if (ok)
        {
            bool hash = (language == "python" || language == "ruby");
            std::string code = stripNoise(source, hash);
            std::vector<std::string> kws = keywordsFor(language);
            for (size_t k = 0; k < kws.size(); k++)
            {
                if (code.find(kws[k]) == std::string::npos) { continue; }
                bool kept = (out.find(kws[k]) != std::string::npos);
                tally(kept);
                if (!kept) { ok = false; why = std::string("keyword dropped: ") + kws[k]; break; }
            }
        }

        if (ok)
        {
            std::string clean = stripNoise(out, false);
            long open = 0;
            long close = 0;
            for (size_t i = 0; i < clean.size(); i++)
            {
                if (clean[i] == '{') { open++; }
                else if (clean[i] == '}') { close++; }
            }
            bool balanced = (open == close);
            tally(balanced);
            if (!balanced) { ok = false; why = "unbalanced braces in output"; }
        }
    }

    if (ok)
    {
        bool crashed3 = false;
        safeFormat(source, wrongLang(language), crashed3);
        tally(!crashed3);
        if (crashed3) { ok = false; why = "wrong-language format crashed"; }
    }

    if (ok) { printf("  PASS: %s\n", display.c_str()); filesPassed++; }
    else { printf("  FAIL: %s (%s)\n", display.c_str(), why.c_str()); failures++; }
    return ok;
}

int runCliPhase(const std::vector<std::pair<std::string, std::string> >& fixtures)
{
    std::string repo = repoDir();
#ifdef _WIN32
    std::string bin = repo + "/build/fmtpp.exe";
#else
    std::string bin = repo + "/build/fmtpp";
#endif
    FILE* probe = fopen(bin.c_str(), "rb");
    if (probe == NULL)
    {
        printf("  SKIP: CLI phase (no binary at %s)\n", bin.c_str());
        return 0;
    }
    fclose(probe);
#ifdef _WIN32
    // ponytail: cmd.exe wants backslashes; the repo dir may mix separators
    std::string shellBin = bin;
    for (size_t i = 0; i < shellBin.size(); i++)
    {
        if (shellBin[i] == '/') { shellBin[i] = '\\'; }
    }
#else
    std::string shellBin = bin;
#endif
    int passed = 0;
    for (size_t i = 0; i < fixtures.size(); i++)
    {
        std::string src = repo + "/tests/fixtures/" + fixtures[i].first;
        size_t dot = fixtures[i].first.find_last_of('.');
        std::string ext = (dot == std::string::npos) ? std::string("txt") : fixtures[i].first.substr(dot + 1);
        char tmpIn[512];
        char tmpOut[512];
        snprintf(tmpIn, sizeof(tmpIn), "%s/build/e2e_cli_tmp_%u.%s",
            repo.c_str(), (unsigned)i, ext.c_str());
        snprintf(tmpOut, sizeof(tmpOut), "%s.stdout", tmpIn);
        std::string content;
        bool ok = readFile(src, content) && writeFile(tmpIn, content);
        std::string cmd;
        if (ok)
        {
#ifdef _WIN32
            std::string shIn = tmpIn;
            std::string shOut = tmpOut;
            for (size_t q = 0; q < shIn.size(); q++)
            {
                if (shIn[q] == '/') { shIn[q] = '\\'; }
            }
            for (size_t q = 0; q < shOut.size(); q++)
            {
                if (shOut[q] == '/') { shOut[q] = '\\'; }
            }
#else
            std::string shIn = tmpIn;
            std::string shOut = tmpOut;
#endif
            cmd = std::string("call \"") + shellBin + "\" format \"" + shIn + "\" > \""
                + shOut + "\" 2>&1";
            fflush(stdout);
            int rc = system(cmd.c_str());
            tally(rc == 0);
            ok = (rc == 0);
            if (!ok) { printf("  FAIL: cli %s (exit %d)\n", fixtures[i].first.c_str(), rc); }
        }
        if (ok)
        {
            std::string captured;
            bool got = readFile(tmpOut, captured);
            bool nonEmpty = got && !captured.empty();
            tally(nonEmpty);
            ok = nonEmpty;
            if (!ok) { printf("  FAIL: cli %s (no stdout)\n", fixtures[i].first.c_str()); }
            else
            {
                bool cleanOut = (captured.find("placeholder") == std::string::npos);
                tally(cleanOut);
                ok = cleanOut;
                if (!ok) { printf("  FAIL: cli %s (placeholder leak)\n", fixtures[i].first.c_str()); }
            }
        }
        remove(tmpIn);
        remove(tmpOut);
        if (ok) { passed++; }
        else { failures++; }
    }
    printf("  CLI phase: %d/%u fixtures via fmtpp binary\n", passed, (unsigned)fixtures.size());
    return (passed == (int)fixtures.size()) ? 0 : 1;
}

}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    static const Fixture fixtures[] = {
        { "sample.js", "js" },
        { "sample.ts", "ts" },
        { "sample.java", "java" },
        { "sample.c", "c" },
        { "sample.cpp", "cpp" },
        { "sample.cs", "cs" },
        { "sample.go", "go" },
        { "sample.rs", "rs" },
        { "sample.kt", "kotlin" },
        { "sample.swift", "swift" },
        { "sample.dart", "dart" },
        { "sample.php", "php" },
        { "sample.scala", "scala" },
        { "sample.m", "objc" },
        { "sample.zig", "zig" },
        { "sample.d", "d" },
        { "sample.groovy", "groovy" },
        { "sample.sol", "solidity" },
        { "sample.v", "v" },
        { "sample.glsl", "glsl" },
        { "sample.html", "html" },
        { "sample.css", "css" },
        { "sample.json", "json" },
        { "sample.vue", "vue" },
        { "sample.py", "python" },
        { "sample.rb", "ruby" },
    };
    size_t count = sizeof(fixtures) / sizeof(fixtures[0]);
    std::string repo = repoDir();
    std::set<std::string> seen;

    printf("Phase A: fixtures\n");
    std::vector<std::pair<std::string, std::string> > cliList;
    for (size_t i = 0; i < count; i++)
    {
        std::string path = repo + "/tests/fixtures/" + fixtures[i].file;
        seen.insert(path);
        cliList.push_back(std::make_pair(fixtures[i].file, fixtures[i].language));
        checkFile(path, std::string("fixtures/") + fixtures[i].file, fixtures[i].language);
    }

    printf("Phase B: auto-discovered scan\n");
    std::vector<std::string> found;
    listFilesRec(repo + "/tests/fixtures", found);
    listFilesRec(repo + "/tests/e2e", found);
    std::sort(found.begin(), found.end());
    int scanned = 0;
    for (size_t i = 0; i < found.size(); i++)
    {
        if (seen.find(found[i]) != seen.end()) { continue; }
        std::string lang = detectLanguage(found[i]);
        if (lang.empty()) { continue; }
        std::string display = found[i];
        size_t cut = display.find("tests/");
        if (cut != std::string::npos) { display = display.substr(cut); }
        checkFile(found[i], display, lang);
        scanned++;
    }
    printf("  scanned %d additional files\n", scanned);

    printf("Phase C: CLI\n");
    int cliRc = runCliPhase(cliList);

    printf("\n========================================\n");
    printf("E2E: %d/%d files passed, %ld/%ld checks passed\n",
        filesPassed, filesTotal, checksPassed, checksTotal);
    printf("========================================\n");

    if (failures > 0 || cliRc != 0) { return 1; }
    return 0;
}
