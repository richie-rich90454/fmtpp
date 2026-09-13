#include "fmtpp/Parser.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/HtmlElementNode.hpp"
#include "fmtpp/ast/WithNode.hpp"
#include "fmtpp/ast/SwitchCaseNode.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// Easter egg: the parser finally parses - no more placeholders on this watch
//
// Design: one shared engine, per-language profiles. The scanner is fully
// comment/string/char/regex aware with paren/brace/bracket depth tracking.
// Anything the engine cannot classify passes through as RawTextNode, so
// formatting restructures only what it understands and never corrupts code.

namespace {

// --- Language profiles ---

enum FuncSyntax
{
    FS_CPREFIX, // [mods] Ret Name(...) — also the generic paren+brace rule
    FS_BARE,    // function Name(...) (js, ts, php, solidity)
    FS_COLON,   // fun|def|def Name(...): Ret (kotlin, scala, groovy)
    FS_ARROW,   // fn|func Name(...) -> Ret (rust, swift)
    FS_TRAIL,   // func|fn Name(...) Ret (go, zig, v)
    FS_RAW      // whole header is the name (objc methods)
};

struct LangProfile
{
    const char* code;
    bool semicolons;
    bool regexLiterals;
    bool hashDirectives;
    bool attrLines;
    bool colonProperties;
    const char* importWords;
    const char* classWords;
    const char* ifaceWords;
    const char* enumWords;
    const char* funcWords;
    int funcSyntax;
    const char* namespaceWord;
    const char* switchWord;
    const char* typeWord;
    bool stripCaseComma;
};

const LangProfile g_profiles[] = {
    // code      semi regex hash attr colon imports                   classes              ifaces      enums  funcs      syntax    ns       switch   type  comma
    { "js",       1,    1,    0,    1,    0,    "import",                "class",             "interface",  "enum",  "function",  FS_BARE,  NULL,      "switch",  NULL,  0 },
    { "ts",       1,    1,    0,    1,    0,    "import",                "class",             "interface",  "enum",  "function",  FS_COLON, "namespace","switch",  NULL,  0 },
    { "java",     1,    0,    0,    1,    0,    "import",                "class",             "interface",  "enum",  "",          FS_CPREFIX,"package", "switch",  NULL,  0 },
    { "c",        1,    0,    1,    1,    0,    "",                      "",                  "",           "enum",  "",          FS_CPREFIX,NULL,      "switch",  NULL,  0 },
    { "cpp",      1,    0,    1,    1,    0,    "",                      "class struct union","",           "enum",  "",          FS_CPREFIX,"namespace","switch",  NULL,  0 },
    { "cs",       1,    0,    1,    1,    0,    "",                      "class struct record","interface", "enum",  "",          FS_CPREFIX,"namespace","switch",  NULL,  0 },
    { "go",       0,    0,    0,    1,    0,    "import",                "",                  "interface",  "",      "func",      FS_TRAIL, "package", "switch",  "type",0 },
    { "rs",       1,    0,    0,    1,    0,    "use",                   "struct union impl",   "trait",      "enum",  "fn",        FS_ARROW, "mod",     "match",   NULL,  1 },
    { "kotlin",   0,    0,    0,    1,    0,    "import",                "class object",        "interface",  "enum",  "fun",       FS_COLON, "package", "when",    NULL,  0 },
    { "swift",    0,    0,    0,    1,    0,    "import",                "class struct actor",  "",           "enum",  "func",      FS_ARROW, NULL,      "switch",  NULL,  0 },
    { "dart",     1,    0,    0,    1,    0,    "import",                "class",             "interface",  "enum",  "",          FS_CPREFIX,NULL,      "switch",  NULL,  0 },
    { "php",      1,    0,    0,    1,    0,    "use",                   "class interface trait enum","",    "",      "function",  FS_COLON, "namespace","switch", NULL,  0 },
    { "scala",    0,    0,    0,    1,    0,    "import",                "class object trait",  "",           "enum",  "def",       FS_COLON, "package", "match",   NULL,  0 },
    { "objc",     1,    0,    1,    1,    0,    "",                      "@interface @implementation","@protocol","enum","",      FS_RAW,   NULL,      "switch",  NULL,  0 },
    { "zig",      1,    0,    0,    1,    0,    "",                      "",                  "",           "",      "fn",        FS_TRAIL, NULL,      "switch",  NULL,  0 },
    { "d",        1,    0,    1,    1,    0,    "import",                "class struct union",  "interface",  "enum",  "",          FS_CPREFIX,"module",  "switch",  NULL,  0 },
    { "groovy",   0,    0,    0,    1,    0,    "import",                "class",             "interface",  "enum",  "def",       FS_COLON, "package", "switch",  NULL,  0 },
    { "solidity", 1,    0,    0,    1,    0,    "import",                "contract interface library","",    "",      "function",  FS_BARE,  NULL,      NULL,      NULL,  0 },
    { "v",        0,    0,    1,    1,    0,    "import",                "struct",              "interface",  "enum",  "fn",        FS_TRAIL, "module",  "match",   "type",0 },
    { "glsl",     1,    0,    1,    1,    0,    "",                      "struct",              "",           "",      "",          FS_CPREFIX,NULL,      "switch",  NULL,  0 },
    { "css",      1,    0,    0,    0,    1,    "@import @charset @namespace","","",            "",      "",          FS_CPREFIX,NULL,      NULL,      NULL,  0 },
    { "python",   0,    0,    0,    1,    0,    "import from",           "class",              "",          "",      "def",       FS_COLON, NULL,      "match",   NULL,  0 },
    { "ruby",     0,    1,    0,    1,    0,    "require",               "class",              "",          "",      "def",       FS_BARE,  NULL,      "case",    NULL,  0 },
    { NULL,       0,    0,    0,    0,    0,    NULL,                    NULL,                  NULL,         NULL,    NULL,        FS_CPREFIX,NULL,      NULL,      NULL,  0 }
};

const LangProfile* profileFor(const std::string& language)
{
    for (int i = 0; g_profiles[i].code != NULL; i++)
    {
        if (language == g_profiles[i].code) { return &g_profiles[i]; }
    }
    return &g_profiles[0];
}

// Words stripped as function modifiers (global set; return types survive
// because they are never in this list).
const char* kModifiers =
    " static virtual inline constexpr consteval constinit friend extern mutable"
    " explicit public private protected internal sealed abstract override async"
    " pub mutating open suspend implicit lazy final new required partial readonly"
    " unsafe volatile synchronized native transient strictfp export default"
    " declare";

// --- Character classes (ASCII-only; bytes >= 0x80 count as identifier chars) ---

bool isHSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f'; }
bool isNl(char c) { return c == '\n' || c == '\r'; }
bool isIdent(char c)
{
    unsigned char u = static_cast<unsigned char>(c);
    return u >= 0x80 || c == '_' || c == '$'
        || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}
bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

std::string trimStr(const std::string& s)
{
    size_t a = 0;
    while (a < s.size() && (isHSpace(s[a]) || s[a] == '\n')) { a++; }
    size_t b = s.size();
    while (b > a && (isHSpace(s[b - 1]) || s[b - 1] == '\n')) { b--; }
    return s.substr(a, b - a);
}

bool hasWord(const char* list, const std::string& word)
{
    if (list == NULL || word.empty()) { return false; }
    std::string padded = std::string(" ") + list + " ";
    return padded.find(std::string(" ") + word + " ") != std::string::npos;
}

bool startsWord(const std::string& text, const char* word)
{
    size_t n = strlen(word);
    if (text.size() < n || text.compare(0, n, word) != 0) { return false; }
    return text.size() == n || !isIdent(text[n]);
}

bool isLabelLine(const std::string& line)
{
    // single identifier followed by a colon: labels, access specifiers,
    // case/default outside switches, C++ access blocks
    size_t i = 0;
    while (i < line.size() && (isHSpace(line[i]))) { i++; }
    size_t start = i;
    while (i < line.size() && (isIdent(line[i]) && line[i] != '$')) { i++; }
    if (i == start) { return false; }
    while (i < line.size() && isHSpace(line[i])) { i++; }
    return i + 1 == line.size() && line[i] == ':';
}

bool isMacroLine(const std::string& line)
{
    // ALL_CAPS single word: Qt-style macros (Q_OBJECT), guards, etc.
    std::string t = trimStr(line);
    if (t.empty()) { return false; }
    for (size_t i = 0; i < t.size(); i++)
    {
        char c = t[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) { return false; }
    }
    return isAlpha(t[0]) || t[0] == '_';
}

// CSS has no // comments (protocol-relative URLs contain them).
bool hasSlashComment(const LangProfile* prof)
{
    return strcmp(prof->code, "css") != 0;
}

// In js/ts/php/solidity a bare name(){} is a method, not a declaration,
// so it passes through verbatim instead of gaining a keyword.
bool bareFuncIsDecl(const LangProfile* prof)
{
    const char* c = prof->code;
    return strcmp(c, "js") != 0 && strcmp(c, "ts") != 0
        && strcmp(c, "php") != 0 && strcmp(c, "solidity") != 0;
}

// --- Cursor ---

struct Cursor
{
    const char* d;
    size_t n;
    size_t p;
    const LangProfile* prof;
};

char peekAt(const Cursor& c, size_t pos) { return pos < c.n ? c.d[pos] : '\0'; }

void skipHSpace(Cursor& c)
{
    while (c.p < c.n && isHSpace(c.d[c.p])) { c.p++; }
}

void skipWs(Cursor& c)
{
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
}

bool atLineStart(const Cursor& c)
{
    if (c.p == 0) { return true; }
    return c.d[c.p - 1] == '\n';
}

// Normalizes CRLF/CR line endings to LF. Scanners assume '\n' everywhere;
// a stray '\r' would ride into verbatim text and resurface as mixed
// endings in formatted output. Zero-copy when no '\r' is present.
bool normalizeNewlines(const char* src, size_t len, std::string& out)
{
    if (src == NULL || memchr(src, '\r', len) == NULL) { return false; }
    out.clear();
    out.reserve(len);
    for (size_t i = 0; i < len; i++)
    {
        if (src[i] == '\r')
        {
            out.push_back('\n');
            if (i + 1 < len && src[i + 1] == '\n') { i++; }
        }
        else { out.push_back(src[i]); }
    }
    return true;
}

// Blank lines separating previous content from the next statement
// (capped at 9). Measures from the end of previous content (not c.p:
// line-based statements already consumed their terminator newline,
// which would otherwise undercount the gap by one) to the next token.
int blankGapBefore(Cursor& c)
{
    size_t back = c.p;
    while (back > 0 && (c.d[back - 1] == ' ' || c.d[back - 1] == '\t' || c.d[back - 1] == '\n')) { back--; }
    size_t s = c.p;
    while (s < c.n && (isHSpace(c.d[s]) || c.d[s] == '\n')) { s++; }
    int nl = 0;
    for (size_t k = back; k < s; k++) { if (c.d[k] == '\n') { nl++; } }
    if (nl < 2) { return 0; }
    int gaps = nl - 1;
    return gaps > 9 ? 9 : gaps;
}

// Reads backward over horizontal space; returns the char before, or '\0'.
char prevSignificant(const Cursor& c, size_t pos)
{
    while (pos > 0 && isHSpace(c.d[pos - 1])) { pos--; }
    return pos > 0 ? c.d[pos - 1] : '\0';
}

// Reads the identifier ending just before pos (for regex-vs-division).
std::string prevWord(const Cursor& c, size_t pos)
{
    while (pos > 0 && isHSpace(c.d[pos - 1])) { pos--; }
    size_t end = pos;
    while (pos > 0 && isIdent(c.d[pos - 1])) { pos--; }
    if (end == pos) { return std::string(); }
    return std::string(c.d + pos, end - pos);
}

bool isRegexStart(const Cursor& c, size_t slashPos)
{
    char prev = prevSignificant(c, slashPos);
    if (prev == '\0' || prev == '\n') { return true; }
    if (isIdent(prev) || prev == ')' || prev == ']' || prev == '"' || prev == '\'')
    {
        if (!isIdent(prev)) { return false; }
        std::string w = prevWord(c, slashPos);
        return w == "return" || w == "typeof" || w == "case" || w == "do"
            || w == "in" || w == "of" || w == "new" || w == "delete"
            || w == "void" || w == "yield" || w == "await" || w == "instanceof";
    }
    return true;
}

// Skips one string starting at the quote (handles escapes; backticks nest ${}).
// Ruby "" nests #{}, Kotlin/Groovy "" nest ${}, Python f-strings nest {},
// triple-quoted Python strings span lines. Bytes are skipped, never parsed.
void skipBalanced(Cursor& c);
void skipBalancedInner(Cursor& c);
void stepUnit(Cursor& c, int& paren, int& brace, int& brack);

void skipString(Cursor& c)
{
    char quote = c.d[c.p];
    std::string code = c.prof->code;
    int interp = 0;
    char marker = '$';
    bool triple = false;
    if (quote == '`')
    {
        interp = 1;
        marker = '$';
    }
    else if (code == "ruby" && quote == '"')
    {
        interp = 1;
        marker = '#';
    }
    else if ((code == "kotlin" || code == "groovy") && quote == '"')
    {
        interp = 1;
        marker = '$';
    }
    else if (code == "python" && (quote == '"' || quote == '\''))
    {
        if (c.p + 2 < c.n && c.d[c.p + 1] == quote && c.d[c.p + 2] == quote)
        {
            triple = true;
        }
        size_t j = c.p;
        while (j > 0 && isAlpha(c.d[j - 1])) { j--; }
        for (size_t k = j; k < c.p; k++)
        {
            if (c.d[k] == 'f' || c.d[k] == 'F') { interp = 2; break; }
        }
    }
    c.p++;
    if (triple) { c.p += 2; }
    int braceDepth = 0;
    while (c.p < c.n)
    {
        char ch = c.d[c.p];
        if (ch == '\\' && c.p + 1 < c.n) { c.p += 2; continue; }
        if (interp == 1 && ch == marker && c.p + 1 < c.n && c.d[c.p + 1] == '{')
        {
            c.p += 2;
            skipBalanced(c);
            continue;
        }
        if (interp == 2 && ch == '{')
        {
            if (c.p + 1 < c.n && c.d[c.p + 1] == '{') { c.p += 2; continue; }
            c.p++;
            skipBalancedInner(c);
            continue;
        }
        if (interp == 2 && ch == '}')
        {
            c.p++;
            continue;
        }
        if (quote == '`' && ch == '$' && c.p + 1 < c.n && c.d[c.p + 1] == '{')
        {
            braceDepth++;
            c.p += 2;
            continue;
        }
        if (quote == '`' && ch == '}' && braceDepth > 0) { braceDepth--; c.p++; continue; }
        c.p++;
        if (ch == quote && braceDepth == 0)
        {
            if (triple)
            {
                if (c.p + 1 < c.n && c.d[c.p] == quote && c.d[c.p + 1] == quote)
                {
                    c.p += 2;
                    break;
                }
                continue;
            }
            break;
        }
    }
}

// Advances past a balanced {...} starting just past '{'.
void skipBalancedInner(Cursor& c)
{
    int paren = 0, brace = 1, brack = 0;
    while (c.p < c.n && brace > 0)
    {
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
    }
}

// Advances past a balanced {...} starting at '{'.
void skipBalanced(Cursor& c)
{
    int paren = 0, brace = 0, brack = 0;
    while (c.p < c.n)
    {
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
        if (brace == 0 && before > 0) { return; }
    }
}

// Skips one lexical unit, tracking bracket depth. Returns false at EOF.
void stepUnit(Cursor& c, int& paren, int& brace, int& brack)
{
    if (c.p >= c.n) { return; }
    char ch = c.d[c.p];
    // line comment (absent in css, where // can appear in url(...))
    if (ch == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '/'
        && hasSlashComment(c.prof))
    {
        while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
        return;
    }
    // block comment
    if (ch == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '*')
    {
        c.p += 2;
        int depth = 1;
        bool nest = (strcmp(c.prof->code, "rs") == 0);
        while (c.p < c.n && depth > 0)
        {
            if (nest && c.d[c.p] == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '*') { depth++; c.p += 2; }
            else if (c.d[c.p] == '*' && c.p + 1 < c.n && c.d[c.p + 1] == '/') { depth--; c.p += 2; }
            else { c.p++; }
        }
        return;
    }
    if (ch == '"' || ch == '\'' || ch == '`')
    {
        // ponytail: apos in some languages is a char literal; skipping is identical
        skipString(c);
        return;
    }
    if (ch == '/' && c.prof->regexLiterals && c.p + 1 < c.n
        && c.d[c.p + 1] != '/' && c.d[c.p + 1] != '*' && c.d[c.p + 1] != '=')
    {
        if (isRegexStart(c, c.p))
        {
            c.p++;
            bool inClass = false;
            while (c.p < c.n)
            {
                char r = c.d[c.p];
                if (r == '\\' && c.p + 1 < c.n) { c.p += 2; continue; }
                if (r == '[') { inClass = true; }
                else if (r == ']') { inClass = false; }
                else if (r == '/' && !inClass) { break; }
                else if (r == '\n') { break; }
                c.p++;
            }
            if (c.p < c.n && c.d[c.p] == '/')
            {
                c.p++;
                while (c.p < c.n && isAlpha(c.d[c.p])) { c.p++; }
            }
            return;
        }
    }
    if (ch == '(') { paren++; }
    else if (ch == ')') { paren--; }
    else if (ch == '[') { brack++; }
    else if (ch == ']') { brack--; }
    else if (ch == '{') { brace++; }
    else if (ch == '}') { brace--; }
    c.p++;
}

// Finds the next {, ;, } (or newline in no-semi languages) at depth 0.
// Returns it with c.p left AT the terminator so callers decide consumption.
char scanTerminator(Cursor& c, size_t& termPos)
{
    int paren = 0, brace = 0, brack = 0;
    while (c.p < c.n)
    {
        char ch = c.d[c.p];
        if ((ch == '{' || ch == ';' || ch == '}') && paren == 0 && brace == 0 && brack == 0)
        {
            termPos = c.p;
            return ch;
        }
        if (ch == '\n' && !c.prof->semicolons && paren == 0 && brace == 0 && brack == 0)
        {
            termPos = c.p;
            return '\n';
        }
        // ponytail: braces inside parens/brackets (initializers, lambdas)
        // belong to the expression, not the block structure
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
        // ponytail: closers without openers (unbalanced input) clamp to 0
        if (paren < 0) { paren = 0; }
        if (brack < 0) { brack = 0; }
        if (brace < 0) { brace = 0; }
    }
    termPos = c.n;
    return 0;
}

// Finds char want at depth 0 (string/comment aware). npos on failure.
size_t findTopLevel(const std::string& s, char want)
{
    int paren = 0, brack = 0, brace = 0;
    char quote = 0;
    for (size_t i = 0; i < s.size(); i++)
    {
        char ch = s[i];
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
        // ponytail: match test comes first, so bracket targets can hit
        if (ch == want && paren == 0 && brack == 0 && brace == 0) { return i; }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
    }
    return std::string::npos;
}

// Splits on delim at depth 0 (paren/bracket/brace + string aware).
void splitTopLevel(const std::string& s, char delim, std::vector<std::string>& out)
{
    size_t start = 0;
    int paren = 0, brack = 0, brace = 0;
    char quote = 0;
    for (size_t i = 0; i <= s.size(); i++)
    {
        char ch = i < s.size() ? s[i] : '\0';
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if ((ch == delim || ch == '\0') && paren == 0 && brack == 0 && brace == 0)
        {
            out.push_back(trimStr(s.substr(start, i - start)));
            start = i + 1;
        }
    }
}

std::string firstWord(const std::string& s)
{
    size_t i = 0;
    while (i < s.size() && (isHSpace(s[i]) || s[i] == '\n')) { i++; }
    size_t start = i;
    while (i < s.size() && (isIdent(s[i]) || s[i] == '@' || s[i] == '#')) { i++; }
    return s.substr(start, i - start);
}

// --- Node builders (raw slices stay verbatim, matching house convention) ---

IdentifierNode* mkIdent(const std::string& text)
{
    IdentifierNode* node = ASTNodeFactory::createIdentifierNode();
    if (node != NULL) { node->m_name = text; }
    return node;
}

LiteralNode* mkNumber(const std::string& text)
{
    LiteralNode* node = ASTNodeFactory::createLiteralNode();
    if (node != NULL)
    {
        node->m_type = LiteralNode::TYPE_NUMBER;
        node->m_value = text;
    }
    return node;
}

LiteralNode* mkBool(const std::string& text)
{
    LiteralNode* node = ASTNodeFactory::createLiteralNode();
    if (node != NULL)
    {
        node->m_type = LiteralNode::TYPE_BOOLEAN;
        node->m_value = text;
    }
    return node;
}

LiteralNode* mkNull()
{
    LiteralNode* node = ASTNodeFactory::createLiteralNode();
    if (node != NULL)
    {
        node->m_type = LiteralNode::TYPE_NULL;
        node->m_value = "null";
    }
    return node;
}

LiteralNode* mkStr(const std::string& inner)
{
    LiteralNode* node = ASTNodeFactory::createLiteralNode();
    if (node != NULL)
    {
        node->m_type = LiteralNode::TYPE_STRING;
        node->m_value = inner;
    }
    return node;
}

ASTNode* mkValue(const std::string& text, bool keepQuotes = false)
{
    std::string t = trimStr(text);
    if (t.empty()) { return NULL; }
    // number?
    bool isNum = !t.empty();
    bool dotSeen = false;
    size_t i = (t[0] == '-' || t[0] == '+') ? 1 : 0;
    if (i >= t.size()) { isNum = false; }
    for (; isNum && i < t.size(); i++)
    {
        if (t[i] == '.' && !dotSeen) { dotSeen = true; }
        else if (!isDigit(t[i])) { isNum = false; }
    }
    if (isNum) { return mkNumber(t); }
    if (t == "true" || t == "false") { return mkBool(t); }
    if (t == "null" || t == "nil" || t == "None" || t == "nilptr" || t == "nullptr") { return mkNull(); }
    if (t.size() >= 2 && ((t[0] == '"' && t[t.size() - 1] == '"')
        || (t[0] == '\'' && t[t.size() - 1] == '\'')))
    {
        // ponytail: py/rb keep prefixes, triples and raw bytes intact
        if (keepQuotes) { return mkStr(t); }
        return mkStr(t.substr(1, t.size() - 2));
    }
    // ponytail: prefixed/triple strings never reach here stripped;
    // keepQuotes callers pass them whole (quotes intact) for verbatim print
    if (keepQuotes && (t.find('"') != std::string::npos || t.find('\'') != std::string::npos))
    {
        return mkStr(t);
    }
    return mkIdent(t);
}

RawTextNode* mkRaw(const std::string& text)
{
    RawTextNode* node = ASTNodeFactory::createRawTextNode();
    if (node != NULL) { node->m_text = trimStr(text); }
    return node;
}

// RawText for statement position: the terminator belongs to the text,
// because formatters never add one to verbatim nodes.
RawTextNode* mkRawSemi(const std::string& text, bool hadSemi)
{
    std::string t = trimStr(text);
    if (hadSemi && !t.empty() && t[t.size() - 1] != ';') { t += ";"; }
    return mkRaw(t);
}

// Consumes one logical line (backslash continuations included).
std::string consumeLogicalLine(Cursor& c)
{
    size_t start = c.p;
    while (c.p < c.n)
    {
        if (c.d[c.p] == '\n') { c.p++; break; }
        if (c.d[c.p] == '\\' && c.p + 1 < c.n && c.d[c.p + 1] == '\n') { c.p += 2; continue; }
        // ponytail: \r\n continuations end with \n; lone \r ends the line too
        if (c.d[c.p] == '\r')
        {
            c.p++;
            if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
            break;
        }
        c.p++;
    }
    return std::string(c.d + start, c.p - start);
}

// Reads a (...) group: pos at '(' on entry, past ')' on success.
bool readParenGroup(Cursor& c, std::string& inner)
{
    if (c.p >= c.n || c.d[c.p] != '(') { return false; }
    size_t start = c.p + 1;
    c.p++;
    int depth = 1;
    while (c.p < c.n && depth > 0)
    {
        char ch = c.d[c.p];
        if (ch == '"' || ch == '\'' || ch == '`'
            || (ch == '/' && c.p + 1 < c.n
                && (c.d[c.p + 1] == '/' || c.d[c.p + 1] == '*')))
        {
            int dummy1 = 0, dummy2 = 0, dummy3 = 0;
            stepUnit(c, dummy1, dummy2, dummy3);
            continue;
        }
        if (ch == '(') { depth++; }
        else if (ch == ')')
        {
            depth--;
            if (depth == 0)
            {
                inner = std::string(c.d + start, c.p - start);
                c.p++;
                return true;
            }
        }
        c.p++;
    }
    return false;
}

void skipTrivia(Cursor& c, std::vector<ASTNode*>* sink);

// Forward declarations for the recursive engine
void parseItems(Cursor& c, std::vector<ASTNode*>& out);
void parseOne(Cursor& c, std::vector<ASTNode*>& out, bool& end);
BlockNode* parseBlock(Cursor& c, bool rawMembers = false);
ASTNode* parseStatementText(const LangProfile* prof, const std::string& text);
void parseHeaderBlock(Cursor& c, std::vector<ASTNode*>& out, const std::string& header, size_t stmtStart);

std::string peekWord(Cursor& c)
{
    size_t i = c.p;
    while (i < c.n && isHSpace(c.d[i])) { i++; }
    size_t start = i;
    while (i < c.n && (isIdent(c.d[i]) || c.d[i] == '@' || c.d[i] == '#')) { i++; }
    return std::string(c.d + start, i - start);
}

bool atWord(Cursor& c, const char* word)
{
    size_t n = strlen(word);
    if (c.p + n > c.n) { return false; }
    if (strncmp(c.d + c.p, word, n) != 0) { return false; }
    return c.p + n >= c.n || !isIdent(c.d[c.p + n]);
}

// Skips whitespace and comments; comments go to sink when given.
void skipTrivia(Cursor& c, std::vector<ASTNode*>* sink)
{
    for (;;)
    {
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        if (c.p + 1 < c.n && c.d[c.p] == '/' && c.d[c.p + 1] == '/'
            && hasSlashComment(c.prof))
        {
            size_t start = c.p;
            while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
            if (sink != NULL)
            {
                CommentNode* node = ASTNodeFactory::createCommentNode();
                node->m_text = std::string(c.d + start + 2, c.p - start - 2);
                node->m_isBlock = false;
                sink->push_back(node);
            }
            continue;
        }
        if (c.p + 1 < c.n && c.d[c.p] == '/' && c.d[c.p + 1] == '*')
        {
            size_t start = c.p + 2;
            c.p += 2;
            int depth = 1;
            bool nest = (strcmp(c.prof->code, "rs") == 0);
            while (c.p < c.n && depth > 0)
            {
                if (nest && c.d[c.p] == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '*') { depth++; c.p += 2; }
                else if (c.d[c.p] == '*' && c.p + 1 < c.n && c.d[c.p + 1] == '/') { depth--; c.p += 2; }
                else { c.p++; }
            }
            if (sink != NULL)
            {
                CommentNode* node = ASTNodeFactory::createCommentNode();
                size_t end = (c.p >= 2) ? c.p - 2 : c.p;
                node->m_text = std::string(c.d + start, end > start ? end - start : 0);
                node->m_isBlock = true;
                sink->push_back(node);
            }
            continue;
        }
        break;
    }
}

bool lineIsAt(Cursor& c)
{
    size_t i = c.p;
    while (i > 0 && (c.d[i - 1] == ' ' || c.d[i - 1] == '\t')) { i--; }
    return i == 0 || c.d[i - 1] == '\n';
}

// Consumes through the first depth-0 ';' (string/depth aware).
std::string consumeThroughSemi(Cursor& c)
{
    size_t start = c.p;
    int paren = 0, brace = 0, brack = 0;
    while (c.p < c.n)
    {
        char ch = c.d[c.p];
        if (ch == ';' && paren == 0 && brace == 0 && brack == 0)
        {
            c.p++;
            break;
        }
        if (ch == '\n' && paren == 0 && brace == 0 && brack == 0)
        {
            // ponytail: no-semi languages end statements at the newline
            if (!c.prof->semicolons) { break; }
        }
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
        if (paren < 0) { paren = 0; }
        if (brack < 0) { brack = 0; }
        if (brace < 0) { brace = 0; }
    }
    return std::string(c.d + start, c.p - start);
}

// Advances past a balanced {...} starting at '{'. False when unbalanced.
bool consumeBalancedBraces(Cursor& c)
{
    if (c.p >= c.n || c.d[c.p] != '{') { return false; }
    int paren = 0, brace = 0, brack = 0;
    while (c.p < c.n)
    {
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
        if (brace == 0 && before > 0)
        {
            return true;
        }
        if (paren < 0) { paren = 0; }
        if (brack < 0) { brack = 0; }
    }
    return brace == 0;
}

// --- Statement builders ---

// Assignment '=' that is not ==, !=, >=, <=, =>, += and friends.
// Defined with the Ruby engine below; declared here so the shared
// assignment scanner can treat %-literals as opaque units too.
bool skipRubyPercent(const char* d, size_t n, size_t& p);

size_t findAssign(const std::string& s)
{
    char quote = 0;
    int paren = 0, brack = 0, brace = 0;
    for (size_t i = 0; i < s.size(); i++)
    {
        char ch = s[i];
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if (ch == '=' && paren == 0 && brack == 0 && brace == 0)
        {
            char prev = i > 0 ? s[i - 1] : '\0';
            char next = i + 1 < s.size() ? s[i + 1] : '\0';
            if (prev == '=' || next == '=') { continue; }
            if (prev == '!' || prev == '<' || prev == '>' || prev == '~'
                || prev == '+' || prev == '-' || prev == '*' || prev == '/'
                || prev == '%' || prev == '&' || prev == '|' || prev == '^'
                || prev == ':' || prev == '?' || prev == '.') { continue; }
            if (next == '>') { continue; }
            return i;
        }
    }
    return std::string::npos;
}

// Splits "kind... name" (C shape). Leading *& stay on the name so the
// caller can tell declarators (*p) from plain names.
void splitKindName(const std::string& left, std::string& kind, std::string& name)
{
    std::string t = trimStr(left);
    size_t end = t.size();
    while (end > 0 && (isHSpace(t[end - 1]))) { end--; }
    size_t start = end;
    while (start > 0 && !isHSpace(t[start - 1])) { start--; }
    name = t.substr(start, end - start);
    kind = trimStr(t.substr(0, start));
}

VariableDeclNode* buildVarDecl(const LangProfile* prof, const std::string& text)
{
    (void)prof;
    std::string t = trimStr(text);
    size_t eq = findAssign(t);
    std::string left = (eq == std::string::npos) ? t : trimStr(t.substr(0, eq));
    std::string init = (eq == std::string::npos) ? std::string() : trimStr(t.substr(eq + 1));

    // walrus := is always a short declaration
    size_t walrus = left.find(":=");
    std::string kind;
    std::string nameType = left;
    std::string walrusInit;
    bool isWalrus = false;
    if (walrus != std::string::npos)
    {
        std::string before = trimStr(left.substr(0, walrus));
        size_t lsp = before.find_last_of(" \t\n");
        std::string lastWord = (lsp == std::string::npos) ? before : before.substr(lsp + 1);
        std::string firstPart = (lsp == std::string::npos) ? std::string() : trimStr(before.substr(0, lsp));
        if (lastWord == "mut" || firstPart == "mut")
        {
            // ponytail: V mut decl (mut x := v); mut anywhere else is invalid
            if (lastWord == "mut" && firstPart.empty())
            {
                kind = "mut";
                nameType = trimStr(left.substr(walrus + 2));
                walrusInit.clear();
            }
            else if (firstPart == "mut" && lastWord.find_first_of(" \t\n") == std::string::npos)
            {
                kind = "mut";
                nameType = lastWord;
                walrusInit = trimStr(left.substr(walrus + 2));
            }
            else
            {
                return NULL;
            }
        }
        else if (before.find(' ') == std::string::npos && !before.empty())
        {
            kind.clear();
            nameType = before;
            walrusInit = trimStr(left.substr(walrus + 2));
            isWalrus = true;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        // leading declaration word? (let/var/const/mut/final)
        size_t sp = nameType.find_first_of(" \t\n");
        std::string first = (sp == std::string::npos) ? nameType : nameType.substr(0, sp);
        if (first == "let" || first == "var" || first == "const"
            || first == "mut" || first == "final")
        {
            kind = first;
            nameType = trimStr(sp == std::string::npos ? std::string() : nameType.substr(sp + 1));
            if (nameType.empty()) { return NULL; }
            // ponytail: "let mut x" keeps mut on the kind (Rust); a bare
            // "mut" stays the name ("let mut = 5" prints back identically).
            // same for val/var pairs ("const val K", Kotlin): only glued
            // when more follows, so plain names never get eaten.
            if (first != "mut" && nameType.size() > 4
                && nameType.compare(0, 4, "mut ") == 0)
            {
                kind += " mut";
                nameType = trimStr(nameType.substr(4));
                if (nameType.empty()) { return NULL; }
            }
            else if (nameType.size() > 4
                && (nameType.compare(0, 4, "val ") == 0 || nameType.compare(0, 4, "var ") == 0)
                && first != nameType.substr(0, 3))
            {
                kind += std::string(" ") + nameType.substr(0, 3);
                nameType = trimStr(nameType.substr(4));
                if (nameType.empty()) { return NULL; }
            }
        }
    }

    std::string name;
    std::string type;
    bool hadColon = false;
    size_t colon = findTopLevel(nameType, ':');
    // ponytail: '::' is scope syntax, not a type annotation
    if (colon != std::string::npos
        && !(colon + 1 < nameType.size() && nameType[colon + 1] == ':')
        && !(colon > 0 && nameType[colon - 1] == ':'))
    {
        hadColon = true;
        name = trimStr(nameType.substr(0, colon));
        type = trimStr(nameType.substr(colon + 1));
    }
    else if (!kind.empty())
    {
        // declaration word present: Go/Swift spell "name type" (split
        // first); C-likes spell "type... name" (split last, keeping the
        // kind: "const char* s" must not become name=const-char*-as-...).
        // "const x"/"static x" read the same either way.
        bool nameFirst = (strcmp(prof->code, "go") == 0 || strcmp(prof->code, "swift") == 0);
        if (nameFirst)
        {
            size_t sp2 = nameType.find_first_of(" \t\n");
            if (sp2 == std::string::npos) { name = nameType; }
            else
            {
                name = trimStr(nameType.substr(0, sp2));
                type = trimStr(nameType.substr(sp2 + 1));
            }
        }
        else
        {
            std::string k2;
            splitKindName(nameType, k2, name);
            if (!k2.empty())
            {
                kind += " ";
                kind += k2;
            }
        }
    }
    else
    {
        splitKindName(nameType, kind, name);
    }

    if (name.empty() || name.find_first_of(" \t\n(){};,") != std::string::npos) { return NULL; }
    // ponytail: a bare left side (x = 5, a.b = 5) is a reassignment, not a
    // declaration — keep it verbatim instead of inventing := or kinds
    if (kind.empty() && !hadColon && !isWalrus
        && left.find_first_of(" \t\n") == std::string::npos) { return NULL; }
    // ponytail: operator words in the kind (total +=) are expressions too
    if (!kind.empty())
    {
        for (size_t ki = 0; ki < kind.size(); ki++)
        {
            char kc = kind[ki];
            bool okc = isIdent(kc) || kc == ' ' || kc == '\t' || kc == '\n'
                || kc == '*' || kc == '&' || kc == '[' || kc == ']'
                || kc == '<' || kc == '>' || kc == ',' || kc == '.'
                || kc == ':' || kc == '?' || kc == '$';
            if (!okc) { return NULL; }
        }
    }
    // ponytail: dereference/address expressions (*p, &x) are statements,
    // never declarations — keep them verbatim instead of rewriting them
    if (name[0] == '*' || name[0] == '&') { return NULL; }
    // ponytail: ++/-- expressions are statements, never declarations
    if (name.size() >= 2)
    {
        std::string tail = name.substr(name.size() - 2);
        std::string head = name.substr(0, 2);
        if (tail == "++" || tail == "--" || head == "++" || head == "--") { return NULL; }
    }

    VariableDeclNode* node = ASTNodeFactory::createVariableDeclNode();
    node->m_kind = kind;
    node->m_name = name;
    node->m_type = type;
    // ponytail: walrus initializers live after :=, not after =
    std::string useInit = init.empty() ? walrusInit : init;
    if (!useInit.empty()) { node->m_initializer = mkValue(useInit); }
    return node;
}

// A prototype/definition shaped header: Name(...) with optional
// return type and modifiers. Empty trailer required.
FunctionDeclNode* tryCFunction(const std::string& header, bool requireRet)
{
    size_t lp = findTopLevel(header, '(');
    if (lp == std::string::npos) { return NULL; }
    // balanced params?
    int depth = 0;
    char quote = 0;
    size_t rp = std::string::npos;
    for (size_t i = lp; i < header.size(); i++)
    {
        char ch = header[i];
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
        if (ch == '(') { depth++; }
        else if (ch == ')')
        {
            depth--;
            if (depth == 0) { rp = i; break; }
        }
    }
    if (rp == std::string::npos) { return NULL; }
    std::string trailer = trimStr(header.substr(rp + 1));
    // ponytail: trailing specs (const, noexcept, = default, : inits,
    // throws) mean "hands off", handled by the RawText escape above
    if (!trailer.empty()) { return NULL; }
    std::string prefix = trimStr(header.substr(0, lp));
    if (prefix.empty()) { return NULL; }
    // name is the last word; * & attach to the type side
    size_t end = prefix.size();
    size_t start = end;
    while (start > 0 && !isHSpace(prefix[start - 1])) { start--; }
    std::string name = prefix.substr(start, end - start);
    while (!name.empty() && (name[0] == '*' || name[0] == '&')) { name = name.substr(1); }
    if (name.empty() || name.find_first_of("(){};,") != std::string::npos) { return NULL; }
    if (name == "if" || name == "for" || name == "while" || name == "switch"
        || name == "catch" || name == "return" || name == "sizeof") { return NULL; }
    std::string pre = trimStr(prefix.substr(0, start));
    // strip leading modifiers; whatever remains is the return type
    std::string modifiers;
    std::string rest = pre;
    for (;;)
    {
        size_t sp = rest.find_first_of(" \t\n");
        std::string first = (sp == std::string::npos) ? rest : rest.substr(0, sp);
        if (first.empty() || !hasWord(kModifiers, first)) { break; }
        if (!modifiers.empty()) { modifiers += " "; }
        modifiers += first;
        rest = (sp == std::string::npos) ? std::string() : trimStr(rest.substr(sp + 1));
    }
    // statement prototypes need a return type (int foo();), so bare calls
    // (foo();), allocations (new X();) and modifier-only headers pass
    // through to the verbatim paths instead of gaining structure
    if (modifiers.empty() && rest.empty()) { return NULL; }
    if (requireRet && rest.empty()) { return NULL; }
    FunctionDeclNode* node = ASTNodeFactory::createFunctionDeclNode();
    node->m_name = name;
    node->m_modifiers = modifiers;
    node->m_returnType = rest;
    std::string params = trimStr(header.substr(lp + 1, rp - lp - 1));
    if (!params.empty())
    {
        node->m_params.push_back(mkIdent(params));
    }
    return node;
}

} // namespace

namespace {

// --- Recursive engine ---

void parseItems(Cursor& c, std::vector<ASTNode*>& out);
void parseOne(Cursor& c, std::vector<ASTNode*>& out, bool& end);
BlockNode* parseBlock(Cursor& c, bool rawMembers);
ASTNode* parseStatementText(const LangProfile* prof, const std::string& text);
void parseHeaderBlock(Cursor& c, std::vector<ASTNode*>& out, const std::string& header, size_t stmtStart);
void parseSwitchArms(Cursor& c, SwitchStmtNode* node);
bool consumeBalancedParens(Cursor& c);
std::string parseCondRest(Cursor& c);
std::string consumeLogicalLinePreview(Cursor& c);

bool isIndentLang(const LangProfile* prof)
{
    return strcmp(prof->code, "python") == 0;
}

bool isEndLang(const LangProfile* prof)
{
    return strcmp(prof->code, "ruby") == 0;
}

// Strips a trailing # comment (string- and %-literal-aware). Returns
// the comment after pos.
std::string stripTrailingComment(const std::string& text, std::string& code)
{
    char quote = 0;
    for (size_t i = 0; i < text.size(); i++)
    {
        char ch = text[i];
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'') { quote = ch; continue; }
        if (ch == '%')
        {
            // ponytail: %-literals are opaque here too; on plain % (or a
            // failed open) fall through and advance one like any char
            size_t j = i;
            if (skipRubyPercent(text.c_str(), text.size(), j))
            {
                i = j - 1;
                continue;
            }
        }
        if (ch == '#')
        {
            code = trimStr(text.substr(0, i));
            return trimStr(text.substr(i));
        }
    }
    code = trimStr(text);
    return std::string();
}

// Column of the first non-whitespace char (tabs count 8, like Python).
int measureIndent(const char* d, size_t n, size_t pos)
{
    int col = 0;
    while (pos < n && (d[pos] == ' ' || d[pos] == '\t'))
    {
        col += (d[pos] == '\t') ? 8 : 1;
        pos++;
    }
    return col;
}

bool rawStrings(const LangProfile* prof)
{
    return strcmp(prof->code, "python") == 0 || strcmp(prof->code, "ruby") == 0;
}

// Advances past a balanced (...) starting at '('. False when unbalanced.
bool consumeBalancedParens(Cursor& c)
{
    if (c.p >= c.n || c.d[c.p] != '(') { return false; }
    int paren = 0, brace = 0, brack = 0;
    while (c.p < c.n)
    {
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
        if (paren == 0 && before > 0) { return true; }
        if (paren < 0) { paren = 0; }
        if (brack < 0) { brack = 0; }
        if (brace < 0) { brace = 0; }
    }
    return paren == 0;
}

// Skips one balanced {...} group (c.p is AT '{'); strings, comments and
// nested groups travel opaquely. False at EOF/unbalanced.
bool skipBalancedBraces(Cursor& c)
{
    if (c.p >= c.n || c.d[c.p] != '{') { return false; }
    int paren = 0, brace = 0, brack = 0;
    bool first = true;
    while (c.p < c.n)
    {
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
        if (!first && brace <= 0 && paren <= 0 && brack <= 0) { return true; }
        first = false;
        if (paren < 0) { paren = 0; }
        if (brack < 0) { brack = 0; }
        if (brace < 0) { brace = 0; }
    }
    return false;
}

// Reads a condition: (...) group when present, else raw text to '{'/newline.
std::string parseCondRest(Cursor& c)
{
    skipHSpace(c);
    if (c.p < c.n && c.d[c.p] == '(')
    {
        std::string inner;
        if (readParenGroup(c, inner)) { return trimStr(inner); }
        return std::string();
    }
    size_t start = c.p;
    int paren = 0, brace = 0, brack = 0;
    while (c.p < c.n && c.d[c.p] != '{' && c.d[c.p] != '\n')
    {
        size_t before = c.p;
        stepUnit(c, paren, brace, brack);
        if (c.p == before) { c.p++; }
    }
    return trimStr(std::string(c.d + start, c.p - start));
}

// After a then-block: else if / else attachment. Comments between break
// the chain (they parse as siblings); only whitespace may intervene.
void attachElse(Cursor& c, IfStmtNode* ifNode)
{
    size_t save = c.p;
    bool sawNewline = false;
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n'))
    {
        if (c.d[c.p] == '\n') { sawNewline = true; }
        c.p++;
    }
    size_t wstart = c.p;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    std::string w(c.d + wstart, c.p - wstart);
    // ponytail: single-word elseif (PHP) chains like else-if; the brace
    // requirement below keeps it from eating identifiers elsewhere
    if (w == "elseif")
    {
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        std::string cond = parseCondRest(c);
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        if (c.p < c.n && c.d[c.p] == '{')
        {
            IfStmtNode* nested = ASTNodeFactory::createIfStmtNode();
            nested->m_condition = mkValue(cond);
            nested->m_thenBranch = parseBlock(c);
            // ponytail: single-word elseif keeps its keyword so the
            // printer can reproduce it when joinElseIf is off
            nested->m_keyword = "elseif";
            ifNode->m_elseBranch = nested;
            if (sawNewline) { ifNode->m_elseOnNewLine = true; }
            // ponytail: chains continue (elseif/else after elseif)
            attachElse(c, nested);
            return;
        }
        c.p = save;
        return;
    }
    if (w != "else")
    {
        c.p = save;
        return;
    }
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    size_t w2 = c.p;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    if (std::string(c.d + w2, c.p - w2) == "if")
    {
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        std::string cond = parseCondRest(c);
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        if (c.p < c.n && c.d[c.p] == '{')
        {
            IfStmtNode* nested = ASTNodeFactory::createIfStmtNode();
            nested->m_condition = mkValue(cond);
            nested->m_thenBranch = parseBlock(c);
            ifNode->m_elseBranch = nested;
            if (sawNewline) { ifNode->m_elseOnNewLine = true; }
            // ponytail: chains continue (else after else-if)
            attachElse(c, nested);
            return;
        }
        c.p = save;
        return;
    }
    if (c.p < c.n && c.d[c.p] == '{')
    {
        ifNode->m_elseBranch = parseBlock(c);
        if (sawNewline) { ifNode->m_elseOnNewLine = true; }
        return;
    }
    c.p = save;
}

// Bracket-balanced chunk reader for switch arms and similar: consumes
// whole lines, extending over unbalanced opens, stopping before a close
// that belongs to the parent. String- and comment-aware throughout.
std::string readBalancedChunk(Cursor& c)
{
    std::string chunk;
    int paren = 0, brace = 0, brack = 0;
    while (c.p < c.n)
    {
        if (!chunk.empty() && c.d[c.p] == '}' && brace <= 0 && paren <= 0 && brack <= 0)
        {
            break;
        }
        size_t lineStart = c.p;
        while (c.p < c.n && c.d[c.p] != '\n')
        {
            char ch = c.d[c.p];
            if (ch == '"' || ch == '\'' || ch == '`')
            {
                Cursor tmp = c;
                skipString(tmp);
                c.p = tmp.p;
                continue;
            }
            if (ch == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '/'
                && hasSlashComment(c.prof))
            {
                while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
                break;
            }
            if (ch == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '*')
            {
                c.p += 2;
                while (c.p + 1 < c.n && !(c.d[c.p] == '*' && c.d[c.p + 1] == '/')) { c.p++; }
                if (c.p + 1 < c.n) { c.p += 2; }
                continue;
            }
            if (ch == '(') { paren++; }
            else if (ch == ')') { if (paren > 0) { paren--; } }
            else if (ch == '[') { brack++; }
            else if (ch == ']') { if (brack > 0) { brack--; } }
            else if (ch == '{') { brace++; }
            else if (ch == '}')
            {
                if (brace > 0) { brace--; }
                else if (paren <= 0 && brack <= 0) { break; }
            }
            c.p++;
        }
        if (!chunk.empty()) { chunk += "\n"; }
        chunk += std::string(c.d + lineStart, c.p - lineStart);
        if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
        if (paren <= 0 && brace <= 0 && brack <= 0) { break; }
    }
    return chunk;
}

void parseItems(Cursor& c, std::vector<ASTNode*>& out)
{
    // ponytail: a 2+ newline gap before a statement means the source had
    // a blank line; stamp it so printers re-emit exactly one (prettier
    // preserves one, drops the rest)
    bool first = true;
    for (;;)
    {
        size_t n0 = out.size();
        int wantBlanks = first ? 0 : blankGapBefore(c);
        first = false;
        bool end = false;
        size_t before = c.p;
        parseOne(c, out, end);
        if (end) { break; }
        if (wantBlanks > 0 && out.size() > n0) { out[n0]->m_blankLinesBefore = wantBlanks; }
        if (c.p == before)
        {
            // ponytail: unreachable safety net; keeps one byte verbatim
            // rather than looping forever or dropping it
            if (c.p < c.n)
            {
                out.push_back(mkRaw(std::string(1, c.d[c.p])));
                c.p++;
            }
            else { break; }
        }
    }
}

BlockNode* parseBlock(Cursor& c, bool rawMembers)
{
    BlockNode* block = ASTNodeFactory::createBlockNode();
    skipHSpace(c);
    if (c.p < c.n && c.d[c.p] == '{') { c.p++; }
    if (rawMembers)
    {
        // ponytail: struct/interface bodies in go/v/rust keep every
        // member line verbatim (fields, tags, embedded names, methods)
        bool first = true;
        for (;;)
        {
            size_t n0 = block->m_statements.size();
            int wantBlanks = first ? 0 : blankGapBefore(c);
            first = false;
            skipTrivia(c, &block->m_statements);
            if (c.p >= c.n) { break; }
            if (c.d[c.p] == '}') { c.p++; break; }
            std::string member = trimStr(readBalancedChunk(c));
            if (member.empty()) { continue; }
            VariableDeclNode* var = ASTNodeFactory::createVariableDeclNode();
            var->m_name = member;
            block->m_statements.push_back(var);
            if (wantBlanks > 0 && block->m_statements.size() > n0) { block->m_statements[n0]->m_blankLinesBefore = wantBlanks; }
        }
        return block;
    }
    bool first = true;
    for (;;)
    {
        size_t n0 = block->m_statements.size();
        int wantBlanks = first ? 0 : blankGapBefore(c);
        first = false;
        skipTrivia(c, &block->m_statements);
        if (c.p >= c.n) { break; }
        if (c.d[c.p] == '}') { c.p++; break; }
        bool end = false;
        size_t before = c.p;
        parseOne(c, block->m_statements, end);
        if (end) { break; }
        if (wantBlanks > 0 && block->m_statements.size() > n0) { block->m_statements[n0]->m_blankLinesBefore = wantBlanks; }
        if (c.p == before)
        {
            if (c.p < c.n && c.d[c.p] != '}')
            {
                block->m_statements.push_back(mkRaw(std::string(1, c.d[c.p])));
                c.p++;
            }
            else { break; }
        }
    }
    return block;
}

bool parseNamespaceLine(Cursor& c, std::vector<ASTNode*>& out, const std::string& word)
{
    size_t save = c.p;
    (void)word;
    c.p += 0;
    // consume the keyword
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    skipHSpace(c);
    // ponytail: a dotted name (namespace.foo()) is a call, not a
    // declaration; rewind untouched so statement paths keep it verbatim
    if (c.p < c.n && c.d[c.p] == '.') { c.p = save; return false; }
    size_t start = c.p;
    while (c.p < c.n && c.d[c.p] != '{' && c.d[c.p] != ';' && c.d[c.p] != '\n')
    {
        if (c.d[c.p] == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '/'
            && hasSlashComment(c.prof)) { break; }
        c.p++;
    }
    std::string name = trimStr(std::string(c.d + start, c.p - start));
    if (!name.empty() && name[name.size() - 1] == ';') { name = trimStr(name.substr(0, name.size() - 1)); }
    NamespaceNode* node = ASTNodeFactory::createNamespaceNode();
    node->m_name = name;
    skipHSpace(c);
    if (c.p < c.n && c.d[c.p] == ';')
    {
        // ponytail: file-scoped declaration (C# namespace N;) ends here
        c.p++;
    }
    else
    {
        // ponytail: the body brace may open on the next line; newlines
        // are significant only as the absence of both ; and {
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        if (c.p < c.n && c.d[c.p] == '{')
        {
            node->m_body = parseBlock(c);
        }
    }
    out.push_back(node);
    return true;
}

void parseImportLine(Cursor& c, std::vector<ASTNode*>& out, const std::string& word)
{
    size_t stmtStart = c.p;
    c.p += word.size();
    skipHSpace(c);
    // grouped imports (Go): import ( ... ) passes through verbatim
    if (c.p < c.n && c.d[c.p] == '(')
    {
        if (consumeBalancedParens(c))
        {
            out.push_back(mkRaw(std::string(c.d + stmtStart, c.p - stmtStart)));
            return;
        }
        c.p = stmtStart + word.size();
    }
    std::string rest;
    if (c.prof->semicolons)
    {
        rest = consumeThroughSemi(c);
        if (!rest.empty() && rest[rest.size() - 1] == ';')
        {
            rest = trimStr(rest.substr(0, rest.size() - 1));
        }
        else
        {
            rest = trimStr(rest);
        }
    }
    else
    {
        size_t s = c.p;
        while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
        rest = trimStr(std::string(c.d + s, c.p - s));
        if (c.p < c.n) { c.p++; }
    }
    std::string fullLine(c.d + stmtStart, c.p - stmtStart);
    std::string code = c.prof->code;
    // js/ts structured imports
    if ((code == "js" || code == "ts") && !rest.empty())
    {
        ImportDeclNode* node = ASTNodeFactory::createImportDeclNode();
        std::string r = rest;
        // side-effect import: "mod";
        if ((r[0] == '"' || r[0] == '\'') && r.size() >= 2 && r[r.size() - 1] == r[0])
        {
            node->m_source = r.substr(1, r.size() - 2);
            out.push_back(node);
            return;
        }
        // default import, namespace import, or { specifiers }
        if (r[0] != '{' && r[0] != '*')
        {
            size_t i = 0;
            while (i < r.size() && !isHSpace(r[i]) && r[i] != ',') { i++; }
            node->m_defaultImport = mkIdent(r.substr(0, i));
            r = trimStr(r.substr(i));
            if (!r.empty() && r[0] == ',') { r = trimStr(r.substr(1)); }
        }
        else if (r[0] == '*')
        {
            size_t f = r.find("from");
            if (f == std::string::npos)
            {
                out.push_back(mkRaw(fullLine));
                return;
            }
            node->m_defaultImport = mkIdent(trimStr(r.substr(0, f)));
            r = trimStr(r.substr(f + 4));
        }
        if (!r.empty() && r[0] == '{')
        {
            int depth = 0;
            size_t k = 0;
            for (; k < r.size(); k++)
            {
                if (r[k] == '{') { depth++; }
                else if (r[k] == '}')
                {
                    depth--;
                    if (depth == 0) { break; }
                }
            }
            if (depth != 0)
            {
                out.push_back(mkRaw(fullLine));
                return;
            }
            std::vector<std::string> specs;
            splitTopLevel(r.substr(1, k - 1), ',', specs);
            for (size_t s2 = 0; s2 < specs.size(); s2++)
            {
                if (!specs[s2].empty()) { node->m_specifiers.push_back(mkIdent(specs[s2])); }
            }
            r = trimStr(r.substr(k + 1));
        }
        if (r.compare(0, 4, "from") == 0)
        {
            std::string src = trimStr(r.substr(4));
            if (src.size() >= 2 && (src[0] == '"' || src[0] == '\'')
                && src[src.size() - 1] == src[0])
            {
                node->m_source = src.substr(1, src.size() - 2);
                out.push_back(node);
                return;
            }
        }
        else if (r.empty() && (node->m_defaultImport != NULL || !node->m_specifiers.empty()))
        {
            // ponytail: sourceless import (invalid input) keeps its structure
            out.push_back(node);
            return;
        }
        out.push_back(mkRaw(fullLine));
        return;
    }
    // rust use a::b::C
    if (code == "rs")
    {
        size_t cut = rest.rfind("::");
        ImportDeclNode* node = ASTNodeFactory::createImportDeclNode();
        if (cut != std::string::npos)
        {
            node->m_source = trimStr(rest.substr(0, cut));
            std::string tail = trimStr(rest.substr(cut + 2));
            if (!tail.empty() && tail[0] == '{')
            {
                std::vector<std::string> specs;
                splitTopLevel(tail.substr(1, tail.size() - 2), ',', specs);
                for (size_t s2 = 0; s2 < specs.size(); s2++)
                {
                    if (!specs[s2].empty()) { node->m_specifiers.push_back(mkIdent(specs[s2])); }
                }
            }
            else if (!tail.empty())
            {
                node->m_specifiers.push_back(mkIdent(tail));
            }
        }
        else
        {
            node->m_source = rest;
        }
        out.push_back(node);
        return;
    }
    // go/v quoted imports (aliases pass through verbatim)
    if (code == "go" || code == "v")
    {
        size_t q1 = rest.find('"');
        ImportDeclNode* node = ASTNodeFactory::createImportDeclNode();
        if (q1 != std::string::npos)
        {
            std::string prefix = trimStr(rest.substr(0, q1));
            size_t q2 = rest.find('"', q1 + 1);
            if (q2 == std::string::npos)
            {
                out.push_back(mkRaw(fullLine));
                return;
            }
            if (!prefix.empty())
            {
                out.push_back(mkRaw(fullLine));
                return;
            }
            node->m_source = rest.substr(q1 + 1, q2 - q1 - 1);
            out.push_back(node);
            return;
        }
        node->m_source = rest;
        out.push_back(node);
        return;
    }
    // plain "keyword rest" imports (java, dart, php use, d, kotlin...)
    // css/solidity sources arrive quoted; those formatters re-add quotes
    ImportDeclNode* node = ASTNodeFactory::createImportDeclNode();
    if ((strcmp(code.c_str(), "css") == 0 || strcmp(code.c_str(), "solidity") == 0)
        && rest.size() >= 2
        && (rest[0] == '"' || rest[0] == '\'')
        && rest[rest.size() - 1] == rest[0])
    {
        node->m_source = rest.substr(1, rest.size() - 2);
    }
    else
    {
        node->m_source = rest;
    }
    out.push_back(node);
}

void parseExportLine(Cursor& c, std::vector<ASTNode*>& out)
{
    size_t stmtStart = c.p;
    c.p += 6; // "export"
    skipHSpace(c);
    // ponytail: only bare declarations sub-parse (their shape is known).
    // default, *, {...}, type and everything else passes through verbatim,
    // because splitting them risks dropping words like `default` or `type`.
    size_t wstart = c.p;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    std::string w(c.d + wstart, c.p - wstart);
    bool subParse = (w == "class" || w == "function" || w == "const"
        || w == "let" || w == "var");
    if (!subParse && w == "async")
    {
        size_t astart = c.p;
        skipHSpace(c);
        size_t bstart = c.p;
        while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
        if (std::string(c.d + bstart, c.p - bstart) == "function")
        {
            subParse = true;
        }
        else
        {
            c.p = astart;
        }
    }
    if (!subParse)
    {
        std::string rest;
        if (c.prof->semicolons)
        {
            rest = consumeThroughSemi(c);
        }
        else
        {
            size_t s = c.p;
            while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
            rest = std::string(c.d + s, c.p - s);
            if (c.p < c.n) { c.p++; }
        }
        (void)rest;
        out.push_back(mkRaw(std::string(c.d + stmtStart, c.p - stmtStart)));
        return;
    }
    // export class/function/const ... — sub-parse the declaration itself
    {
        c.p = wstart;
        std::vector<ASTNode*> inner;
        bool end = false;
        parseOne(c, inner, end);
        if (!inner.empty() && inner[0] != NULL)
        {
            ExportDeclNode* node = ASTNodeFactory::createExportDeclNode();
            node->m_declaration = inner[0];
            out.push_back(node);
            return;
        }
    }
    out.push_back(mkRaw(std::string(c.d + stmtStart, c.p - stmtStart)));
}

ASTNode* parseStatementText(const LangProfile* prof, const std::string& text)
{
    std::string t = trimStr(text);
    if (t.empty()) { return NULL; }
    // ponytail: one trailing semicolon belongs to the statement; verbatim
    // fallbacks below reattach it, structured nodes reprint their own
    bool hadSemi = false;
    if (!t.empty() && t[t.size() - 1] == ';')
    {
        hadSemi = true;
        t = trimStr(t.substr(0, t.size() - 1));
    }
    if (t.empty()) { return NULL; }
    // ObjC method prototypes keep the whole line as their signature
    if (strcmp(prof->code, "objc") == 0 && (t[0] == '-' || t[0] == '+'))
    {
        FunctionDeclNode* m = ASTNodeFactory::createFunctionDeclNode();
        m->m_name = t;
        return m;
    }
    std::string w = firstWord(t);
    // return / throw
    if (w == "return" || w == "throw")
    {
        std::string rest = trimStr(t.substr(w.size()));
        ASTNode* arg = rest.empty() ? NULL : mkValue(rest);
        if (w == "return")
        {
            ReturnStmtNode* node = ASTNodeFactory::createReturnStmtNode();
            node->m_argument = arg;
            return node;
        }
        ThrowStmtNode* node = ASTNodeFactory::createThrowStmtNode();
        node->m_expression = arg;
        return node;
    }
    // case/default outside switches, labels
    if (w == "case" || w == "default" || isLabelLine(t)) { return mkRawSemi(t, hadSemi); }    // C# using directives pass through (using statements have blocks)
    if (strcmp(prof->code, "cs") == 0 && w == "using") { return mkRawSemi(t, hadSemi); }
    // universal type alias (ts, rust)
    if (w == "type" && (strcmp(prof->code, "ts") == 0 || strcmp(prof->code, "rs") == 0))
    {
        size_t eq = findTopLevel(t, '=');
        if (eq == std::string::npos) { return mkRawSemi(t, hadSemi); }
        std::string left = trimStr(t.substr(4, eq - 4));
        size_t sp = left.find_first_of(" \t\n");
        std::string name = (sp == std::string::npos) ? left : trimStr(left.substr(0, sp));
        if (name.empty()) { return mkRawSemi(t, hadSemi); }
        TypeAliasNode* node = ASTNodeFactory::createTypeAliasNode();
        node->m_name = name;
        node->m_typeAnnotation = mkIdent(trimStr(t.substr(eq + 1)));
        return node;
    }
    // css-style name: value declarations
    if (prof->colonProperties)
    {
        size_t colon = findTopLevel(t, ':');
        if (colon != std::string::npos && colon > 0 && t[colon - 1] != ':'
            && !(colon + 1 < t.size() && t[colon + 1] == ':'))
        {
            PropertyNode* node = ASTNodeFactory::createPropertyNode();
            node->m_key = mkIdent(trimStr(t.substr(0, colon)));
            node->m_value = mkValue(trimStr(t.substr(colon + 1)));
            return node;
        }
        return mkRawSemi(t, hadSemi);
    }
    // assignment or declaration?
    size_t eq = findAssign(t);
    if (eq != std::string::npos)
    {
        VariableDeclNode* var = buildVarDecl(prof, t);
        if (var != NULL) { return var; }
        return mkRawSemi(t, hadSemi);
    }
    // prototype without return type is a call: pass through.
    // indent/end languages never declare C prototypes (their f(x) lines
    // are calls), so skip the prototype guess there entirely.
    // ponytail: a class/interface/enum/object word before the parens means
    // a declaration with a primary constructor (kotlin data class), never
    // a C prototype — those keep verbatim instead of gaining fun/ret.
    {
        size_t lp0 = findTopLevel(t, '(');
        if (lp0 != std::string::npos)
        {
            std::string pre0 = trimStr(t.substr(0, lp0));
            size_t wi0 = 0;
            bool hitType = false;
            while (wi0 < pre0.size() && !hitType)
            {
                while (wi0 < pre0.size() && (isHSpace(pre0[wi0]) || pre0[wi0] == '\n')) { wi0++; }
                size_t ws0 = wi0;
                while (wi0 < pre0.size() && isIdent(pre0[wi0])) { wi0++; }
                if (ws0 < wi0)
                {
                    std::string word0 = pre0.substr(ws0, wi0 - ws0);
                    if (hasWord(prof->classWords, word0) || hasWord(prof->ifaceWords, word0)
                        || hasWord(prof->enumWords, word0)) { hitType = true; }
                }
                if (ws0 == wi0) { wi0++; }
            }
            if (hitType) { return mkRawSemi(t, hadSemi); }
        }
    }
    // ponytail: Solidity event/error declarations are not functions
    // ("function Sent(...) event" would be invalid); keep them verbatim.
    // Assignments and bare calls never reach here shaped this way.
    {
        std::string w0 = firstWord(t);
        if ((w0 == "event" || w0 == "error") && strcmp(prof->code, "solidity") == 0)
        {
            return mkRawSemi(t, hadSemi);
        }
    }
    FunctionDeclNode* proto = NULL;
    // ponytail: js/ts have no prototype syntax at all (`await g()` is a
    // unary expression, not a declaration), same as python/ruby above.
    if (strcmp(prof->code, "python") != 0 && strcmp(prof->code, "ruby") != 0
        && strcmp(prof->code, "js") != 0 && strcmp(prof->code, "ts") != 0)
    {
        proto = tryCFunction(t, true);
    }
    if (proto != NULL) { return proto; }
    // single bare words (break, continue, calls without args shape...)
    // pass through; kind/name splits rejoin byte-identical anyway
    VariableDeclNode* var = buildVarDecl(prof, t);
    if (var != NULL)
    {
        // ponytail: guard the degenerate shapes buildVarDecl can't see
        if (var->m_kind.empty() && var->m_initializer == NULL)
        {
            bool bare = true;
            for (size_t i = 0; i < var->m_name.size() && bare; i++)
            {
                char ch = var->m_name[i];
                if (!isIdent(ch) && ch != '$') { bare = false; }
            }
            if (bare) { return mkRawSemi(t, hadSemi); }
        }
        return var;
    }
    return mkRawSemi(t, hadSemi);
}

// Splits a brace header at depth-0 newlines; leading chunks that are
// decorators, labels, macros or access specifiers flush as RawText,
// anything else joins the final chunk.
bool splitHeaderChunks(const std::string& header, std::vector<std::string>& leading, std::string& last)
{
    std::vector<std::string> chunks;
    size_t start = 0;
    int paren = 0, brack = 0, brace = 0;
    char quote = 0;
    for (size_t i = 0; i <= header.size(); i++)
    {
        char ch = i < header.size() ? header[i] : '\0';
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if ((ch == '\n' || ch == '\0') && paren == 0 && brack == 0 && brace == 0)
        {
            chunks.push_back(header.substr(start, i - start));
            start = i + 1;
        }
    }
    if (chunks.empty()) { last = header; return true; }
    for (size_t i = 0; i + 1 < chunks.size(); i++)
    {
        std::string t = trimStr(chunks[i]);
        if (t.empty()) { continue; }
        if (t[0] == '@' || isLabelLine(t) || isMacroLine(t)
            || ((t == "public:" || t == "private:" || t == "protected:"
                || t == "signals:" || t == "slots:") ))
        {
            leading.push_back(t);
        }
        else
        {
            // ponytail: wrapped headers (Allman bases, split decls) rejoin
            last += (last.empty() ? "" : "\n") + t;
        }
    }
    last += (last.empty() ? "" : "\n") + trimStr(chunks.back());
    last = trimStr(last);
    return true;
}

bool startsWithWord(const std::string& text, const char* word)
{
    size_t n = strlen(word);
    size_t i = 0;
    while (i < text.size() && (isHSpace(text[i]) || text[i] == '\n')) { i++; }
    if (text.compare(i, n, word) != 0) { return false; }
    size_t j = i + n;
    return j >= text.size() || !isIdent(text[j]);
}

// An expression brace (initializer, return of literal, new, unbalanced
// call) belongs to the statement, not the block structure.
bool headerIsExpression(const std::string& header)
{
    std::string t = trimStr(header);
    // ponytail: a top-level = counts only with a right-hand side;
    // a trailing = (Scala `def f: T = {`) is syntax, not assignment
    {
        char quote = 0;
        int paren = 0, brack = 0, brace = 0;
        for (size_t i = 0; i < t.size(); i++)
        {
            char ch = t[i];
            if (quote)
            {
                if (ch == '\\') { i++; }
                else if (ch == quote) { quote = 0; }
                continue;
            }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
        if (ch == '%')
        {
            // ponytail: Ruby %-literals are opaque (= signs inside are not
            // assignments); anything else steps over plainly, so C-style
            // modulo and format verbs never notice
            size_t j = i;
            if (skipRubyPercent(t.c_str(), t.size(), j)) { i = j - 1; continue; }
        }
        if (ch == '(') { paren++; }
            else if (ch == ')') { if (paren > 0) paren--; }
            else if (ch == '[') { brack++; }
            else if (ch == ']') { if (brack > 0) brack--; }
            else if (ch == '{') { brace++; }
            else if (ch == '}') { if (brace > 0) brace--; }
            else if (ch == '=' && paren == 0 && brack == 0 && brace == 0)
            {
                char prev = i > 0 ? t[i - 1] : '\0';
                char next = i + 1 < t.size() ? t[i + 1] : '\0';
                if (prev == '=' || next == '=') { continue; }
                if (prev == '!' || prev == '<' || prev == '>' || prev == '~'
                    || prev == '+' || prev == '-' || prev == '*' || prev == '/'
                    || prev == '%' || prev == '&' || prev == '|' || prev == '^'
                    || prev == ':' || prev == '?' || prev == '.') { continue; }
                if (next == '>') { continue; }
                if (trimStr(t.substr(i + 1)).empty()) { continue; }
                return true;
            }
            else if (ch == ':' && paren == 0 && brack == 0 && brace == 0
                && i + 1 < t.size() && t[i + 1] == '=')
            {
                // ponytail: := struct literals (p := Point{...}) stay whole
                return true;
            }
        }
    }
    const char* words[] = { "return", "throw", "new", "synchronized", "else", NULL };
    for (int i = 0; words[i] != NULL; i++)
    {
        if (startsWithWord(t, words[i])) { return true; }
    }
    // unbalanced open paren: callback-style calls like foo(fn() {
    int depth = 0;
    char quote = 0;
    for (size_t i = 0; i < t.size(); i++)
    {
        char ch = t[i];
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
        if (ch == '(') { depth++; }
        else if (ch == ')') { if (depth > 0) depth--; }
    }
    return depth > 0;
}

void parseFunctionHead(Cursor& c, std::vector<ASTNode*>& out, const std::string& header);

// Statements whose early words name a function keyword (pub fn foo();)
// parse as functions; everything else goes to the splitter.
void pushStatement(Cursor& c, std::vector<ASTNode*>& out, const std::string& text)
{
    std::string t = trimStr(text);
    size_t wi = 0;
    for (int k = 0; k < 3 && wi < t.size(); k++)
    {
        while (wi < t.size() && (isHSpace(t[wi]) || t[wi] == '\n')) { wi++; }
        size_t ws = wi;
        while (wi < t.size() && isIdent(t[wi])) { wi++; }
        if (ws == wi) { break; }
        if (hasWord(c.prof->funcWords, t.substr(ws, wi - ws)))
        {
            // ponytail: only when no assignment precedes the params;
            // `def dog = new Dog()` is a declaration, not a function
            size_t lp = findTopLevel(t, '(');
            size_t eq = findAssign(t);
            if (lp != std::string::npos && (eq == std::string::npos || lp < eq))
            {
                std::string noSemi = t;
                if (!noSemi.empty() && noSemi[noSemi.size() - 1] == ';')
                {
                    noSemi = trimStr(noSemi.substr(0, noSemi.size() - 1));
                }
                parseFunctionHead(c, out, noSemi);
                return;
            }
            break;
        }
    }
    ASTNode* stmt = parseStatementText(c.prof, text);
    if (stmt != NULL) { out.push_back(stmt); }
}
void parseTypeHead(Cursor& c, std::vector<ASTNode*>& out, const std::string& header,
    const std::string& kind, int kindClass);

// Rejoins angle-bracket template lists split on inner commas.
void rejoinAngles(std::vector<std::string>& parts)
{
    for (size_t i = 0; i + 1 < parts.size(); i++)
    {
        int depth = 0;
        for (size_t k = 0; k < parts[i].size(); k++)
        {
            if (parts[i][k] == '<') { depth++; }
            else if (parts[i][k] == '>') { if (depth > 0) depth--; }
        }
        if (depth > 0)
        {
            parts[i] = parts[i] + ", " + parts[i + 1];
            parts.erase(parts.begin() + i + 1);
            i--;
        }
    }
}

void parseHeaderBlock(Cursor& c, std::vector<ASTNode*>& out, const std::string& header, size_t stmtStart)
{
    (void)stmtStart;
    std::string H = trimStr(header);
    if (H.empty())
    {
        out.push_back(parseBlock(c));
        return;
    }
    if (headerIsExpression(H))
    {
        // expression containing a block: consume it, keep scanning
        if (c.p < c.n && c.d[c.p] == '{') { consumeBalancedBraces(c); }
        if (c.prof->semicolons)
        {
            size_t termPos = 0;
            char term = scanTerminator(c, termPos);
            std::string full(c.d + stmtStart, termPos - stmtStart + (term == ';' ? 1 : 0));
            if (term == ';') { c.p = termPos + 1; }
            else { c.p = termPos; }
            pushStatement(c, out, full);
        }
        else
        {
            size_t s = c.p;
            while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
            std::string full(c.d + stmtStart, c.p - stmtStart);
            if (c.p < c.n) { c.p++; }
            pushStatement(c, out, full);
        }
        return;
    }
    // chunk split: decorators/labels flush, the rest joins
    std::vector<std::string> leading;
    std::string last;
    splitHeaderChunks(H, leading, last);
    for (size_t i = 0; i < leading.size(); i++)
    {
        out.push_back(mkRaw(leading[i]));
    }
    H = last;
    // function keywords anywhere before the params
    // (empty funcWords simply never matches below)
    {
        size_t lp = findTopLevel(H, '(');
        std::string pre = (lp == std::string::npos) ? H : H.substr(0, lp);
        size_t wi = 0;
        bool foundFunc = false;
        while (wi < pre.size())
        {
            while (wi < pre.size() && (isHSpace(pre[wi]) || pre[wi] == '\n')) { wi++; }
            size_t ws = wi;
            while (wi < pre.size() && isIdent(pre[wi])) { wi++; }
            if (ws < wi && hasWord(c.prof->funcWords, pre.substr(ws, wi - ws))) { foundFunc = true; break; }
            if (ws == wi) { wi++; }
        }
        if (foundFunc)
        {
            parseFunctionHead(c, out, H);
            return;
        }
    }
    // class/interface/enum keywords (with modifier prefixes)
    {
        std::string kw;
        int kindClass = 0;
        size_t wi = 0;
        size_t kpos = std::string::npos;
        while (wi < H.size())
        {
            while (wi < H.size() && (isHSpace(H[wi]) || H[wi] == '\n')) { wi++; }
            size_t ws = wi;
            while (wi < H.size() && (isIdent(H[wi]) || H[wi] == '@')) { wi++; }
            if (ws < wi)
            {
                std::string word = H.substr(ws, wi - ws);
                if (hasWord(c.prof->classWords, word)) { kw = word; kindClass = 0; kpos = ws; break; }
                if (hasWord(c.prof->ifaceWords, word)) { kw = word; kindClass = 1; kpos = ws; break; }
                if (hasWord(c.prof->enumWords, word)) { kw = word; kindClass = 2; kpos = ws; break; }
                if (word == "(" || H[ws] == '(') { break; }
            }
            if (ws == wi) { wi++; }
        }
        if (kpos != std::string::npos)
        {
            std::string kind = trimStr(H.substr(0, kpos + kw.size()));
            parseTypeHead(c, out, H.substr(kpos + kw.size()), kind, kindClass);
            return;
        }
    }
    // C-style Name(...) headers (constructors, plain functions, prototypes)
    {
        FunctionDeclNode* fn = tryCFunction(H, false);
        if (fn != NULL)
        {
            // ponytail: a "return type" of get/set means an accessor
            // (get v()/set v(x)), not a C prototype — fall through to the
            // verbatim escape hatch instead of gaining a function keyword
            if (fn->m_returnType != "get" && fn->m_returnType != "set")
            {
                fn->m_body = parseBlock(c);
                out.push_back(fn);
                return;
            }
        }
    }
    // ObjC methods keep the whole header as their signature
    if (strcmp(c.prof->code, "objc") == 0 && !H.empty() && (H[0] == '-' || H[0] == '+'))
    {
        FunctionDeclNode* node = ASTNodeFactory::createFunctionDeclNode();
        node->m_name = H;
        node->m_body = parseBlock(c);
        out.push_back(node);
        return;
    }
    // CSS: any other header with a block is a rule (selector + body)
    if (strcmp(c.prof->code, "css") == 0)
    {
        ClassDeclNode* node = ASTNodeFactory::createClassDeclNode();
        node->m_name = H;
        node->m_body = parseBlock(c);
        out.push_back(node);
        return;
    }
    // bare name(){}: declaration in most languages, verbatim method elsewhere.
    // "constructor" is always a declaration (a bare constructor() call with
    // a braced block would print back identically anyway).
    {
        size_t lp = findTopLevel(H, '(');
        if (lp != std::string::npos)
        {
            std::string pre = trimStr(H.substr(0, lp));
            std::string post = trimStr(H.substr(lp));
            bool singleIdent = !pre.empty();
            for (size_t i = 0; singleIdent && i < pre.size(); i++)
            {
                if (!isIdent(pre[i])) { singleIdent = false; }
            }
            // balanced parens with nothing after?
            int depth = 0;
            char quote = 0;
            size_t rp = std::string::npos;
            for (size_t i = lp; i < H.size(); i++)
            {
                char ch = H[i];
                if (quote)
                {
                    if (ch == '\\') { i++; }
                    else if (ch == quote) { quote = 0; }
                    continue;
                }
                if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
                if (ch == '(') { depth++; }
                else if (ch == ')')
                {
                    depth--;
                    if (depth == 0) { rp = i; break; }
                }
            }
            if (singleIdent && rp != std::string::npos && trimStr(H.substr(rp + 1)).empty())
            {
                if (bareFuncIsDecl(c.prof) || pre == "constructor")
                {
                    FunctionDeclNode* node = ASTNodeFactory::createFunctionDeclNode();
                    node->m_name = pre;
                    std::string params = trimStr(H.substr(lp + 1, rp - lp - 1));
                    if (!params.empty()) { node->m_params.push_back(mkIdent(params)); }
                    node->m_body = parseBlock(c);
                    out.push_back(node);
                    return;
                }
            }
            (void)post;
        }
    }
    // trailing match (Scala "expr match {"): discriminant leads, the
    // keyword closes the header. Only for match-word languages; anything
    // else keeps the verbatim escape hatch below.
    if (c.prof->switchWord != NULL && strcmp(c.prof->switchWord, "match") == 0
        && c.p < c.n && c.d[c.p] == '{')
    {
        size_t mp = std::string::npos;
        {
            int paren = 0, brack = 0, brace = 0;
            char quote = 0;
            for (size_t i = 0; i + 5 <= H.size(); i++)
            {
                char ch = H[i];
                if (quote)
                {
                    if (ch == '\\') { i++; }
                    else if (ch == quote) { quote = 0; }
                    continue;
                }
                if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; continue; }
                if (ch == '(') { paren++; }
                else if (ch == ')') { if (paren > 0) paren--; }
                else if (ch == '[') { brack++; }
                else if (ch == ']') { if (brack > 0) brack--; }
                else if (ch == '{') { brace++; }
                else if (ch == '}') { if (brace > 0) brace--; }
                else if (paren == 0 && brack == 0 && brace == 0
                    && H.compare(i, 5, "match") == 0
                    && (i == 0 || (!isIdent(H[i - 1]) && H[i - 1] != '.'))
                    && (i + 5 >= H.size() || !isIdent(H[i + 5])))
                {
                    mp = i;
                }
            }
        }
        if (mp != std::string::npos)
        {
            std::string disc = trimStr(H.substr(0, mp));
            std::string tail = trimStr(H.substr(mp + 5));
            if (!disc.empty() && tail.empty())
            {
                SwitchStmtNode* node = ASTNodeFactory::createSwitchStmtNode();
                node->m_discriminant = mkValue(disc);
                parseSwitchArms(c, node);
                out.push_back(node);
                return;
            }
        }
    }
    // escape hatch: verbatim header, structured body — never corrupt
    out.push_back(mkRaw(H));
    out.push_back(parseBlock(c));
}

void parseFunctionHead(Cursor& c, std::vector<ASTNode*>& out, const std::string& header)
{
    const LangProfile* prof = c.prof;
    // locate the keyword: first funcWord in the header prefix
    size_t lp = findTopLevel(header, '(');
    std::string pre = (lp == std::string::npos) ? header : header.substr(0, lp);
    size_t kwEnd = std::string::npos;
    size_t wi = 0;
    while (wi < pre.size())
    {
        while (wi < pre.size() && (isHSpace(pre[wi]) || pre[wi] == '\n')) { wi++; }
        size_t ws = wi;
        while (wi < pre.size() && isIdent(pre[wi])) { wi++; }
        if (ws < wi && hasWord(prof->funcWords, pre.substr(ws, wi - ws))) { kwEnd = wi; break; }
        if (ws == wi)
        {
            if (wi < pre.size()) { wi++; } else { break; }
        }
    }
    if (kwEnd == std::string::npos || lp == std::string::npos)
    {
        // ponytail: keyword without params is not a function (e.g. `def` alone)
        out.push_back(mkRaw(header));
        if (c.p < c.n && c.d[c.p] == '{') { out.push_back(parseBlock(c)); }
        return;
    }
    // params group
    Cursor sub = c;
    sub.d = header.c_str();
    sub.n = header.size();
    sub.p = lp;
    std::string params;
    if (!readParenGroup(sub, params))
    {
        out.push_back(mkRaw(header));
        if (c.p < c.n && c.d[c.p] == '{') { out.push_back(parseBlock(c)); }
        return;
    }
    std::string trailer = trimStr(header.substr(sub.p));
    std::string modifiers;
    std::string returnType;
    // modifiers are the words before the keyword, verbatim
    {
        size_t kwStart = kwEnd;
        std::string before = trimStr(pre.substr(0, kwStart));
        // walk back over the keyword itself
        size_t e = kwEnd;
        while (e > 0 && (isHSpace(before[e - 1]) || before[e - 1] == '\n')) { e--; }
        size_t s = e;
        while (s > 0 && isIdent(before[s - 1])) { s--; }
        modifiers = trimStr(before.substr(0, s));
    }
    std::string name;
    {
        // ponytail: the name is everything from the keyword to the params:
        // plain names, generics (Scala f[A], Kotlin <T> f) and JS *gen()
        // all ride verbatim instead of losing pieces to a char allowlist
        size_t np = kwEnd;
        while (np < header.size() && (isHSpace(header[np]) || header[np] == '\n')) { np++; }
        size_t ne = (lp == std::string::npos) ? header.size() : lp;
        name = trimStr(header.substr(np, ne > np ? ne - np : 0));
        if (!name.empty() && name.find_first_of("(){};") == std::string::npos)
        {
            // ponytail: plain slice taken (generics like f[A, B] ride whole)
        }
        else if (!name.empty())
        {
            // ponytail: stray punctuation in the slice means hands off
            name.clear();
        }
        else
        {
            // ponytail: Go receivers ("func (s S) M("): the first group is
            // the receiver, the real name+params follow it. Anything else
            // shaped this way stays raw (anonymous JS functions et al).
            // Both pieces are validated by construction (balanced group +
            // identifier), so no re-check is needed here.
            std::string afterRecv = trimStr(header.substr(sub.p));
            size_t rp2 = 0;
            while (rp2 < afterRecv.size() && isIdent(afterRecv[rp2])) { rp2++; }
            size_t qp = rp2;
            while (qp < afterRecv.size()
                && (isHSpace(afterRecv[qp]) || afterRecv[qp] == '\n')) { qp++; }
            if (rp2 > 0 && qp < afterRecv.size() && afterRecv[qp] == '(')
            {
                Cursor sub2 = c;
                sub2.d = afterRecv.c_str();
                sub2.n = afterRecv.size();
                sub2.p = qp;
                std::string params2;
                if (readParenGroup(sub2, params2))
                {
                    name = "(" + params + ") " + afterRecv.substr(0, rp2);
                    params = params2;
                    trailer = trimStr(afterRecv.substr(sub2.p));
                }
            }
        }
    }
    bool bad = false;
    if (prof->funcSyntax == FS_BARE)
    {
        if (strcmp(prof->code, "solidity") == 0 && !trailer.empty())
        {
            // function f() public returns (uint): split at returns
            size_t rpos = trailer.find("returns");
            bool isRet = (rpos != std::string::npos)
                && (rpos == 0 || !isIdent(trailer[rpos - 1]))
                && (rpos + 7 >= trailer.size() || !isIdent(trailer[rpos + 7]));
            if (isRet)
            {
                std::string mods = trimStr(trailer.substr(0, rpos));
                if (!modifiers.empty() && !mods.empty()) { modifiers += " "; }
                modifiers += mods;
                returnType = trimStr(trailer.substr(rpos));
            }
            else
            {
                if (!modifiers.empty() && !trailer.empty()) { modifiers += " "; }
                modifiers += trailer;
            }
        }
        else if (!trailer.empty())
        {
            bad = true;
        }
    }
    else if (prof->funcSyntax == FS_COLON)
    {
        if (!trailer.empty() && trailer[0] == ':')
        {
            returnType = trimStr(trailer.substr(1));
        }
        else if (!trailer.empty())
        {
            bad = true;
        }
    }
    else if (prof->funcSyntax == FS_ARROW)
    {
        if (!trailer.empty() && trailer.compare(0, 2, "->") == 0)
        {
            returnType = trimStr(trailer.substr(2));
        }
        else if (!trailer.empty())
        {
            bad = true;
        }
    }
    else if (prof->funcSyntax == FS_TRAIL)
    {
        returnType = trailer;
    }
    else if (prof->funcSyntax == FS_RAW)
    {
        name = trimStr(header);
    }
    if (name.empty() && prof->funcSyntax != FS_RAW) { bad = true; }
    if (bad)
    {
        out.push_back(mkRaw(header));
        if (c.p < c.n && c.d[c.p] == '{') { out.push_back(parseBlock(c)); }
        return;
    }
    FunctionDeclNode* node = ASTNodeFactory::createFunctionDeclNode();
    node->m_name = name;
    node->m_modifiers = modifiers;
    node->m_returnType = returnType;
    std::string pt = trimStr(params);
    if (!pt.empty()) { node->m_params.push_back(mkIdent(pt)); }
    if (c.p < c.n && c.d[c.p] == '{')
    {
        node->m_body = parseBlock(c);
    }
    out.push_back(node);
}

void parseTypeHead(Cursor& c, std::vector<ASTNode*>& out, const std::string& header,
    const std::string& kind, int kindClass)
{
    // ponytail: rest-after-keyword parsing; unknown bases stay verbatim
    std::string R = trimStr(header);
    std::string name;
    std::string after;
    size_t lp = findTopLevel(R, '(');
    if (lp != std::string::npos)
    {
        // tuple structs: struct Name(...) — the parens ride in the name
        int depth = 0;
        char quote = 0;
        size_t rp = std::string::npos;
        for (size_t i = lp; i < R.size(); i++)
        {
            char ch = R[i];
            if (quote)
            {
                if (ch == '\\') { i++; }
                else if (ch == quote) { quote = 0; }
                continue;
            }
            if (ch == '"' || ch == '\'') { quote = ch; continue; }
            if (ch == '(') { depth++; }
            else if (ch == ')')
            {
                depth--;
                if (depth == 0) { rp = i; break; }
            }
        }
        if (rp != std::string::npos && trimStr(R.substr(rp + 1)).empty())
        {
            name = trimStr(R);
        }
    }
    if (name.empty())
    {
        // ponytail: paren-aware so primary constructors (name: Type)
        // and annotations don't split the name from its bases
        size_t i = 0;
        int pdepth = 0;
        char quote = 0;
        while (i < R.size())
        {
            char ch = R[i];
            if (quote)
            {
                if (ch == '\\') { i++; }
                else if (ch == quote) { quote = 0; }
                i++;
                continue;
            }
            if (ch == '"' || ch == '\'') { quote = ch; i++; continue; }
            if (ch == '(') { pdepth++; }
            else if (ch == ')') { if (pdepth > 0) pdepth--; }
            else if (pdepth == 0)
            {
                // ponytail: :: is scope syntax, only a lone : splits
                if (ch == ':' && !(i + 1 < R.size() && R[i + 1] == ':')
                    && !(i > 0 && R[i - 1] == ':')) { break; }
                if (R.compare(i, 7, "extends") == 0 && (i == 0 || !isIdent(R[i - 1]))
                    && (i + 7 >= R.size() || !isIdent(R[i + 7]))) { break; }
                if (R.compare(i, 10, "implements") == 0 && (i == 0 || !isIdent(R[i - 1]))
                    && (i + 10 >= R.size() || !isIdent(R[i + 10]))) { break; }
                if (R.compare(i, 2, "is") == 0 && i > 0 && !isIdent(R[i - 1])
                    && (i + 2 >= R.size() || !isIdent(R[i + 2]))) { break; }
                if (R.compare(i, 4, "with") == 0 && i > 0 && !isIdent(R[i - 1])
                    && (i + 4 >= R.size() || !isIdent(R[i + 4]))) { break; }
            }
            i++;
        }
        name = trimStr(R.substr(0, i));
        after = trimStr(R.substr(i));
    }
    if (name.empty())
    {
        out.push_back(mkRaw(kind + (R.empty() ? "" : " " + R)));
        if (c.p < c.n && c.d[c.p] == '{') { out.push_back(parseBlock(c)); }
        return;
    }
    std::vector<std::string> protos;
    if (!kind.empty() && kind[0] == '@')
    {
        size_t a = after.find('<');
        size_t b = after.rfind('>');
        if (a != std::string::npos && b != std::string::npos && b > a)
        {
            splitTopLevel(after.substr(a + 1, b - a - 1), ',', protos);
            after = trimStr(after.substr(0, a));
        }
    }
    ASTNode* superCls = NULL;
    std::vector<ASTNode*> impls;
    bool basesOk = true;
    std::string A = trimStr(after);
    if (!A.empty() && A[0] == ':' && !(A.size() > 1 && A[1] == ':'))
    {
        std::vector<std::string> parts;
        splitTopLevel(trimStr(A.substr(1)), ',', parts);
        rejoinAngles(parts);
        for (size_t i = 0; i < parts.size(); i++)
        {
            std::string b = trimStr(parts[i]);
            for (;;)
            {
                size_t sp = b.find_first_of(" \t\n");
                std::string first = (sp == std::string::npos) ? b : b.substr(0, sp);
                if (first == "public" || first == "protected" || first == "private"
                    || first == "virtual")
                {
                    b = trimStr(sp == std::string::npos ? std::string() : b.substr(sp + 1));
                    continue;
                }
                break;
            }
            if (b.empty()) { basesOk = false; break; }
            if (superCls == NULL) { superCls = mkIdent(b); }
            else { impls.push_back(mkIdent(b)); }
        }
    }
    else if (startsWithWord(A, "extends"))
    {
        std::string tail = trimStr(A.substr(7));
        size_t cut = std::string::npos;
        size_t ii = tail.find("implements");
        size_t wi = tail.find(" with ");
        if (ii != std::string::npos && (wi == std::string::npos || ii < wi)) { cut = ii; }
        else if (wi != std::string::npos) { cut = wi; }
        std::string sup = (cut == std::string::npos) ? tail : trimStr(tail.substr(0, cut));
        std::string rest2 = (cut == std::string::npos) ? std::string() : trimStr(tail.substr(cut));
        if (sup.empty()) { basesOk = false; }
        else { superCls = mkIdent(sup); }
        if (!rest2.empty())
        {
            if (startsWithWord(rest2, "implements")) { rest2 = trimStr(rest2.substr(10)); }
            else if (startsWithWord(rest2, "with")) { rest2 = trimStr(rest2.substr(4)); }
            std::vector<std::string> parts;
            splitTopLevel(rest2, ',', parts);
            rejoinAngles(parts);
            for (size_t i = 0; i < parts.size(); i++)
            {
                if (!parts[i].empty()) { impls.push_back(mkIdent(parts[i])); }
            }
        }
    }
    else if (startsWithWord(A, "implements"))
    {
        std::vector<std::string> parts;
        splitTopLevel(trimStr(A.substr(10)), ',', parts);
        rejoinAngles(parts);
        for (size_t i = 0; i < parts.size(); i++)
        {
            if (!parts[i].empty()) { impls.push_back(mkIdent(parts[i])); }
        }
    }
    else if (startsWithWord(A, "with"))
    {
        std::string tail = trimStr(A.substr(4));
        size_t wi = tail.find(" with ");
        while (wi != std::string::npos)
        {
            impls.push_back(mkIdent(trimStr(tail.substr(0, wi))));
            tail = trimStr(tail.substr(wi + 6));
            wi = tail.find(" with ");
        }
        if (!tail.empty()) { impls.push_back(mkIdent(tail)); }
    }
    else if (startsWithWord(A, "is"))
    {
        std::vector<std::string> parts;
        splitTopLevel(trimStr(A.substr(2)), ',', parts);
        rejoinAngles(parts);
        for (size_t i = 0; i < parts.size(); i++)
        {
            if (parts[i].empty()) { continue; }
            if (superCls == NULL) { superCls = mkIdent(parts[i]); }
            else { impls.push_back(mkIdent(parts[i])); }
        }
    }
    else if (!A.empty())
    {
        basesOk = false;
    }
    for (size_t i = 0; i < protos.size(); i++)
    {
        if (!protos[i].empty()) { impls.push_back(mkIdent(protos[i])); }
    }
    if (!basesOk)
    {
        out.push_back(mkRaw(kind + " " + name + (A.empty() ? "" : " " + A)));
        if (c.p < c.n && c.d[c.p] == '{') { out.push_back(parseBlock(c)); }
        return;
    }
    if (kindClass == 2)
    {
        if (c.p >= c.n || c.d[c.p] != '{')
        {
            out.push_back(mkRaw(kind + " " + name + (A.empty() ? "" : " " + A)));
            return;
        }
        c.p++;
        size_t start = c.p;
        int paren = 0, brace = 1, brack = 0;
        while (c.p < c.n && brace > 0)
        {
            size_t before = c.p;
            stepUnit(c, paren, brace, brack);
            if (c.p == before) { c.p++; }
            if (paren < 0) { paren = 0; }
            if (brack < 0) { brack = 0; }
        }
        std::string inner(c.d + start, (c.p > start + 1 ? c.p - start - 1 : 0));
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = name;
        enm->m_kind = kind;
        std::vector<std::string> members;
        splitTopLevel(inner, ',', members);
        if (members.size() == 1 && members[0].find('\n') != std::string::npos)
        {
            // ponytail: newline-separated members (V, Go const blocks);
            // rejoin segments left unbalanced by the split
            std::vector<std::string> lines;
            size_t start = 0;
            for (size_t li = 0; li <= members[0].size(); li++)
            {
                if (li == members[0].size() || members[0][li] == '\n')
                {
                    std::string line = trimStr(members[0].substr(start, li - start));
                    if (!line.empty()) { lines.push_back(line); }
                    start = li + 1;
                }
            }
            members.clear();
            std::string acc;
            int depth = 0;
            for (size_t li = 0; li < lines.size(); li++)
            {
                for (size_t k = 0; k < lines[li].size(); k++)
                {
                    char ch = lines[li][k];
                    if (ch == '(' || ch == '[' || ch == '{') { depth++; }
                    else if (ch == ')' || ch == ']' || ch == '}') { if (depth > 0) depth--; }
                }
                if (!acc.empty()) { acc += "\n"; }
                acc += lines[li];
                if (depth <= 0)
                {
                    members.push_back(acc);
                    acc.clear();
                }
            }
            if (!trimStr(acc).empty()) { members.push_back(trimStr(acc)); }
        }
        for (size_t i = 0; i < members.size(); i++)
        {
            if (!members[i].empty()) { enm->m_members.push_back(mkIdent(members[i])); }
        }
        out.push_back(enm);
        return;
    }
    if (kindClass == 1)
    {
        InterfaceDeclNode* node = ASTNodeFactory::createInterfaceDeclNode();
        node->m_name = name;
        node->m_kind = kind;
        if (superCls != NULL) { node->m_extends.push_back(superCls); }
        for (size_t i = 0; i < impls.size(); i++) { node->m_extends.push_back(impls[i]); }
        if (c.p < c.n && c.d[c.p] == '{')
        {
            bool raw = (strcmp(c.prof->code, "go") == 0 || strcmp(c.prof->code, "v") == 0);
            node->m_body = parseBlock(c, raw);
        }
        out.push_back(node);
        return;
    }
    ClassDeclNode* node = ASTNodeFactory::createClassDeclNode();
    node->m_name = name;
    node->m_kind = kind;
    node->m_superClass = superCls;
    node->m_implements = impls;
    if (c.p < c.n && c.d[c.p] == '{')
    {
        bool raw = (strcmp(c.prof->code, "go") == 0 || strcmp(c.prof->code, "v") == 0)
            || ((strcmp(c.prof->code, "rs") == 0)
                && (kind == "struct" || kind == "union"));
        node->m_body = parseBlock(c, raw);
    }
    out.push_back(node);
}

void parseIfRest(Cursor& c, std::vector<ASTNode*>& out)
{
    // consume the keyword, parse condition, expect a block
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    std::string cond = parseCondRest(c);
    skipHSpace(c);
    if (c.p < c.n && c.d[c.p] == '\n' && c.prof->semicolons)
    {
        // ponytail: same-line brace required in semi languages is NOT
        // enforced; Allman style keeps the brace on the next line
    }
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    IfStmtNode* node = ASTNodeFactory::createIfStmtNode();
    if (!cond.empty()) { node->m_condition = mkValue(cond); }
    if (c.p < c.n && c.d[c.p] == '{')
    {
        node->m_thenBranch = parseBlock(c);
    }
    else
    {
        // ponytail: single-statement body without braces
        std::vector<ASTNode*> body;
        bool end = false;
        parseOne(c, body, end);
        if (!body.empty())
        {
            BlockNode* block = ASTNodeFactory::createBlockNode();
            block->m_statements = body;
            node->m_thenBranch = block;
        }
    }
    out.push_back(node);
    attachElse(c, node);
}

void parseWhileRest(Cursor& c, std::vector<ASTNode*>& out)
{
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    std::string cond = parseCondRest(c);
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    WhileStmtNode* node = ASTNodeFactory::createWhileStmtNode();
    if (!cond.empty()) { node->m_condition = mkValue(cond); }
    if (c.p < c.n && c.d[c.p] == '{')
    {
        node->m_body = parseBlock(c);
    }
    out.push_back(node);
}

void parseForRest(Cursor& c, std::vector<ASTNode*>& out, const std::string& keyword)
{
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    ForStmtNode* node = ASTNodeFactory::createForStmtNode();
    node->m_keyword = keyword;
    skipHSpace(c);
    std::string inner;
    bool haveParens = (c.p < c.n && c.d[c.p] == '(');
    // ponytail: Zig captures ride after the group (for (xs) |x|); kept
    // separate so the parens survive verbatim below
    std::string forCapture;
    if (haveParens)
    {
        if (!readParenGroup(c, inner)) { inner.clear(); }
        inner = trimStr(inner);
        size_t q = c.p;
        while (q < c.n && (isHSpace(c.d[q]) || c.d[q] == '\n')) { q++; }
        if (q < c.n && c.d[q] == '|')
        {
            size_t e = q + 1;
            while (e < c.n && c.d[e] != '|' && c.d[e] != '\n') { e++; }
            if (e < c.n && c.d[e] == '|')
            {
                forCapture = trimStr(std::string(c.d + q, e - q + 1));
                c.p = e + 1;
            }
        }
    }
    else
    {
        size_t start = c.p;
        int paren = 0, brace = 0, brack = 0;
        while (c.p < c.n && c.d[c.p] != '{' && c.d[c.p] != '\n')
        {
            size_t before = c.p;
            stepUnit(c, paren, brace, brack);
            if (c.p == before) { c.p++; }
        }
        inner = trimStr(std::string(c.d + start, c.p - start));
    }
    if (!inner.empty())
    {
        std::vector<std::string> parts;
        splitTopLevel(inner, ';', parts);
        if (parts.size() == 3)
        {
            node->m_classic = true;
            if (!parts[0].empty()) { node->m_init = mkValue(parts[0]); }
            if (!parts[1].empty()) { node->m_condition = mkValue(parts[1]); }
            if (!parts[2].empty()) { node->m_update = mkValue(parts[2]); }
        }
        else
        {
            // ponytail: for-in/foreach/range headers stay whole so no
            // keyword, variable or separator is ever dropped or invented;
            // a capture keeps its parens (Zig requires them around the
            // payload, and dropping them corrupts the header)
            node->m_classic = false;
            if (haveParens && !forCapture.empty())
            {
                node->m_condition = mkValue("(" + inner + ") " + forCapture);
            }
            else
            {
                node->m_condition = mkValue(inner);
            }
        }
    }
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    if (c.p < c.n && c.d[c.p] == '{')
    {
        node->m_body = parseBlock(c);
    }
    out.push_back(node);
}

void parseDoRest(Cursor& c, std::vector<ASTNode*>& out)
{
    size_t save = c.p;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    if (c.p >= c.n || c.d[c.p] != '{')
    {
        c.p = save;
        out.push_back(mkRaw("do"));
        return;
    }
    BlockNode* body = parseBlock(c);
    size_t afterBody = c.p;
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    size_t wstart = c.p;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    if (std::string(c.d + wstart, c.p - wstart) != "while")
    {
        // ponytail: malformed do without tail: verbatim head, structured body
        c.p = afterBody;
        out.push_back(mkRaw("do"));
        out.push_back(body);
        return;
    }
    std::string cond = parseCondRest(c);
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    if (c.p < c.n && c.d[c.p] == ';') { c.p++; }
    out.push_back(mkRaw("do"));
    out.push_back(body);
    WhileStmtNode* tail = ASTNodeFactory::createWhileStmtNode();
    if (!cond.empty()) { tail->m_condition = mkValue(cond); }
    out.push_back(tail);
}

// Flushes one switch arm as one node per line, so every continuation
// line re-indents instead of printing at column 0.
void pushSwitchArm(const LangProfile* prof, SwitchStmtNode* node, const std::string& cur)
{
    std::string arm = trimStr(cur);
    if (arm.empty()) { return; }
    if (prof->stripCaseComma && arm[arm.size() - 1] == ',')
    {
        arm = trimStr(arm.substr(0, arm.size() - 1));
    }
    size_t start = 0;
    for (size_t i = 0; i <= arm.size(); i++)
    {
        if (i == arm.size() || arm[i] == '\n')
        {
            std::string line = trimStr(arm.substr(start, i - start));
            if (!line.empty())
            {
                node->m_cases.push_back(mkRaw(line));
            }
            start = i + 1;
        }
    }
}

bool isArmStart(const std::string& line, bool first)
{
    std::string t = trimStr(line);
    if (t.empty()) { return false; }
    if (first) { return true; }
    if (startsWithWord(t, "case") || startsWithWord(t, "default")
        || startsWithWord(t, "else")) { return true; }
    // depth-0 => or -> opens an arm
    int paren = 0, brack = 0, brace = 0;
    char quote = 0;
    for (size_t i = 0; i + 1 < t.size(); i++)
    {
        char ch = t[i];
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'') { quote = ch; continue; }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if (paren == 0 && brack == 0 && brace == 0)
        {
            if (ch == '=' && t[i + 1] == '>') { return true; }
            if (ch == '-' && t[i + 1] == '>') { return true; }
        }
    }
    return false;
}

void parseSwitchRest(Cursor& c, std::vector<ASTNode*>& out, const std::string& keyword)
{
    (void)keyword;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    std::string disc = parseCondRest(c);
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    SwitchStmtNode* node = ASTNodeFactory::createSwitchStmtNode();
    node->m_keyword = keyword;
    if (!disc.empty()) { node->m_discriminant = mkValue(disc); }
    if (c.p >= c.n || c.d[c.p] != '{')
    {
        out.push_back(mkRaw(disc.empty() ? keyword : keyword + " " + disc));
        return;
    }
    parseSwitchArms(c, node);
    out.push_back(node);
}

// Consumes a brace-delimited arm list into node (c.p is AT '{').
void parseSwitchArms(Cursor& c, SwitchStmtNode* node)
{
    if (c.p < c.n && c.d[c.p] == '{') { c.p++; }
    bool armPerChunk = (strcmp(c.prof->code, "v") == 0);
    std::string cur;
    bool haveArm = false;
    bool first = true;
    for (;;)
    {
        while (c.p < c.n && (c.d[c.p] == ' ' || c.d[c.p] == '\t' || c.d[c.p] == '\n' || c.d[c.p] == '\r')) { c.p++; }
        if (c.p >= c.n) { break; }
        if (c.d[c.p] == '}')
        {
            c.p++;
            break;
        }
        // peek the next line to decide arm boundaries
        size_t ls = c.p;
        while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
        std::string line(c.d + ls, c.p - ls);
        if (c.p < c.n) { c.p++; }
        // comments inside switches stay as their own arms
        std::string lt = trimStr(line);
        if (lt.compare(0, 2, "//") == 0 && hasSlashComment(c.prof))
        {
            if (haveArm && !cur.empty())
            {
                pushSwitchArm(c.prof, node, cur);
                cur.clear();
                haveArm = false;
                first = false;
            }
            CommentNode* cm = ASTNodeFactory::createCommentNode();
            cm->m_text = lt.substr(2);
            cm->m_isBlock = false;
            node->m_cases.push_back(cm);
            continue;
        }
        if (isArmStart(line, first) || (armPerChunk && haveArm))
        {
            if (haveArm && !trimStr(cur).empty())
            {
                pushSwitchArm(c.prof, node, cur);
            }
            // rewind and consume the arm as a balanced chunk
            c.p = ls;
            cur = readBalancedChunk(c);
            haveArm = !trimStr(cur).empty();
            first = false;
            continue;
        }
        if (!cur.empty()) { cur += "\n"; }
        cur += line;
        haveArm = true;
    }
    if (haveArm && !trimStr(cur).empty())
    {
        pushSwitchArm(c.prof, node, cur);
    }
}

void parseCatchInto(Cursor& c, TryCatchStmtNode* node, bool first, bool isFinally)
{
    // c.p is just past the catch/finally keyword
    if (isFinally)
    {
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        if (c.p < c.n && c.d[c.p] == '{')
        {
            node->m_finallyBlock = parseBlock(c);
        }
        return;
    }
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    ASTNode* clause = NULL;
    if (c.p < c.n && c.d[c.p] == '(')
    {
        std::string inner;
        if (readParenGroup(c, inner) && !trimStr(inner).empty())
        {
            clause = mkIdent(trimStr(inner));
        }
    }
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    ASTNode* body = NULL;
    if (c.p < c.n && c.d[c.p] == '{')
    {
        body = parseBlock(c);
    }
    if (first)
    {
        node->m_catchClause = clause;
        node->m_catchBody = body;
        return;
    }
    TryCatchStmtNode* extra = ASTNodeFactory::createTryCatchStmtNode();
    extra->m_catchClause = clause;
    extra->m_catchBody = body;
    node->m_extraCatches.push_back(extra);
}

void parseTryRest(Cursor& c, std::vector<ASTNode*>& out)
{
    size_t kw = c.p;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    // remainder on the try line must be empty, else verbatim escape
    while (c.p < c.n && (isHSpace(c.d[c.p]))) { c.p++; }
    if (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '{')
    {
        // ponytail: try-with-expression (returns clauses) stays verbatim
        size_t i = c.p;
        int paren = 0, brack = 0;
        char quote = 0;
        bool haveBrace = false;
        while (i < c.n && c.d[i] != '\n')
        {
            char ch = c.d[i];
            if (quote)
            {
                if (ch == '\\') { i++; }
                else if (ch == quote) { quote = 0; }
            }
            else if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; }
            else if (ch == '(') { paren++; }
            else if (ch == ')') { if (paren > 0) paren--; }
            else if (ch == '[') { brack++; }
            else if (ch == ']') { if (brack > 0) brack--; }
            else if (ch == '{' && paren == 0 && brack == 0) { haveBrace = true; break; }
            i++;
        }
        out.push_back(mkRaw(std::string(c.d + kw, i - kw)));
        c.p = i;
        if (haveBrace) { out.push_back(parseBlock(c)); }
        return;
    }
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    TryCatchStmtNode* node = ASTNodeFactory::createTryCatchStmtNode();
    if (c.p < c.n && c.d[c.p] == '{')
    {
        node->m_body = parseBlock(c);
    }
    // catch/finally chain: whitespace (not comments) may intervene.
    // A newline between `}` and the keyword means Allman placement;
    // stamp it so printers reproduce rather than normalize it.
    bool firstCatch = true;
    for (;;)
    {
        size_t save = c.p;
        bool sawNewline = false;
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n'))
        {
            if (c.d[c.p] == '\n') { sawNewline = true; }
            c.p++;
        }
        size_t ws = c.p;
        while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
        std::string w(c.d + ws, c.p - ws);
        if (w != "catch" && w != "finally")
        {
            c.p = save;
            break;
        }
        parseCatchInto(c, node, firstCatch, w == "finally");
        if (w == "catch")
        {
            // ponytail: only the first catch belongs to this node;
            // later ones ride in m_extraCatches and keep current behavior
            if (firstCatch && sawNewline) { node->m_catchOnNewLine = true; }
            firstCatch = false;
        }
        else
        {
            if (sawNewline) { node->m_finallyOnNewLine = true; }
            break;
        }
    }
    out.push_back(node);
}

void parseTypeStmt(Cursor& c, std::vector<ASTNode*>& out)
{
    // go/v: type Name struct|interface {...} | type Name = ... | type Name T
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    skipHSpace(c);
    size_t ns = c.p;
    while (c.p < c.n && (isIdent(c.d[c.p]) || c.d[c.p] == '.')) { c.p++; }
    std::string name(c.d + ns, c.p - ns);
    skipHSpace(c);
    size_t ws = c.p;
    while (c.p < c.n && isIdent(c.d[c.p])) { c.p++; }
    std::string kind(c.d + ws, c.p - ws);
    if (kind == "struct")
    {
        ClassDeclNode* node = ASTNodeFactory::createClassDeclNode();
        node->m_name = name;
        node->m_kind = "struct";
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        if (c.p < c.n && c.d[c.p] == '{') { node->m_body = parseBlock(c, true); }
        out.push_back(node);
        return;
    }
    if (kind == "interface")
    {
        InterfaceDeclNode* node = ASTNodeFactory::createInterfaceDeclNode();
        node->m_name = name;
        while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
        if (c.p < c.n && c.d[c.p] == '{') { node->m_body = parseBlock(c, true); }
        out.push_back(node);
        return;
    }
    // alias or plain type: rest of the statement, braces included so that
    // type X = struct { ... } stays one node (the = is significant in Go)
    std::string rest;
    if (c.prof->semicolons)
    {
        rest = consumeThroughSemi(c);
        if (!rest.empty() && rest[rest.size() - 1] == ';')
        {
            rest = trimStr(rest.substr(0, rest.size() - 1));
        }
    }
    else
    {
        size_t s = c.p;
        while (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '{') { c.p++; }
        rest = trimStr(std::string(c.d + s, c.p - s));
        if (c.p < c.n && c.d[c.p] == '{')
        {
            size_t bs = c.p;
            if (consumeBalancedBraces(c))
            {
                rest += (rest.empty() ? "" : " ") + std::string(c.d + bs, c.p - bs);
            }
        }
        if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
    }
    TypeAliasNode* node = ASTNodeFactory::createTypeAliasNode();
    node->m_name = name;
    std::string ann = rest;
    if (!kind.empty() && kind != "=")
    {
        ann = kind + (ann.empty() ? "" : " " + ann);
    }
    if (!ann.empty()) { node->m_typeAnnotation = mkIdent(ann); }
    out.push_back(node);
}


// Previews one logical line without consuming it.
std::string consumeLogicalLinePreview(Cursor& c)
{
    size_t save = c.p;
    std::string line = consumeLogicalLine(c);
    c.p = save;
    return trimStr(line);
}

std::string lowerStr(const std::string& s)
{
    std::string out = s;
    for (size_t i = 0; i < out.size(); i++)
    {
        if (out[i] >= 'A' && out[i] <= 'Z') { out[i] = static_cast<char>(out[i] + 32); }
    }
    return out;
}

bool isHtmlVoid(const std::string& tag)
{
    std::string t = lowerStr(tag);
    return t == "area" || t == "base" || t == "br" || t == "col" || t == "embed"
        || t == "hr" || t == "img" || t == "input" || t == "link" || t == "meta"
        || t == "param" || t == "source" || t == "track" || t == "wbr";
}

bool isRawTextElement(const std::string& tag)
{
    std::string t = lowerStr(tag);
    return t == "script" || t == "style" || t == "textarea" || t == "title";
}

void htmlAttach(ProgramNode* prog, std::vector<HtmlElementNode*>& stack, ASTNode* node)
{
    if (stack.empty()) { prog->m_statements.push_back(node); }
    else { stack.back()->m_children.push_back(node); }
}

ProgramNode* parseHtmlDoc(const char* src, size_t len)
{
    std::string normalized;
    if (normalizeNewlines(src, len, normalized)) { src = normalized.c_str(); len = normalized.size(); }
    Cursor c;
    c.d = src;
    c.n = len;
    c.p = 0;
    c.prof = profileFor("html");
    ProgramNode* prog = ASTNodeFactory::createProgramNode();
    std::vector<HtmlElementNode*> stack;
    while (c.p < c.n)
    {
        if (c.d[c.p] == '<' && c.p + 3 < c.n && c.d[c.p + 1] == '!'
            && c.d[c.p + 2] == '-' && c.d[c.p + 3] == '-')
        {
            size_t start = c.p + 4;
            c.p += 4;
            while (c.p + 2 < c.n
                && !(c.d[c.p] == '-' && c.d[c.p + 1] == '-' && c.d[c.p + 2] == '>')) { c.p++; }
            CommentNode* node = ASTNodeFactory::createCommentNode();
            node->m_text = std::string(c.d + start, c.p > start ? c.p - start : 0);
            node->m_isBlock = true;
            if (c.p + 2 < c.n) { c.p += 3; }
            else { c.p = c.n; }
            htmlAttach(prog, stack, node);
            continue;
        }
        if (c.d[c.p] == '<' && c.p + 1 < c.n && c.d[c.p + 1] == '/')
        {
            c.p += 2;
            size_t ns = c.p;
            while (c.p < c.n && (isIdent(c.d[c.p]) || c.d[c.p] == ':' || c.d[c.p] == '-'
                || c.d[c.p] == '.')) { c.p++; }
            std::string tag(c.d + ns, c.p - ns);
            while (c.p < c.n && c.d[c.p] != '>') { c.p++; }
            if (c.p < c.n) { c.p++; }
            // ponytail: pop through the match; stray closes are ignored
            for (size_t i = stack.size(); i > 0; i--)
            {
                if (lowerStr(stack[i - 1]->m_tag) == lowerStr(tag))
                {
                    stack.erase(stack.begin() + i - 1, stack.end());
                    break;
                }
                if (i == 1) { break; }
            }
            continue;
        }
        if (c.d[c.p] == '<' && c.p + 1 < c.n && c.d[c.p + 1] == '!')
        {
            size_t start = c.p;
            while (c.p < c.n && c.d[c.p] != '>') { c.p++; }
            if (c.p < c.n) { c.p++; }
            PreprocessorDirectiveNode* node = ASTNodeFactory::createPreprocessorDirectiveNode();
            node->m_directive = trimStr(std::string(c.d + start, c.p - start));
            htmlAttach(prog, stack, node);
            continue;
        }
        if (c.d[c.p] == '<' && c.p + 1 < c.n && c.d[c.p + 1] == '?')
        {
            size_t start = c.p;
            while (c.p + 1 < c.n && !(c.d[c.p] == '?' && c.d[c.p + 1] == '>')) { c.p++; }
            if (c.p + 1 < c.n) { c.p += 2; }
            else { c.p = c.n; }
            PreprocessorDirectiveNode* node = ASTNodeFactory::createPreprocessorDirectiveNode();
            node->m_directive = trimStr(std::string(c.d + start, c.p - start));
            htmlAttach(prog, stack, node);
            continue;
        }
        if (c.d[c.p] == '<' && c.p + 1 < c.n && isAlpha(c.d[c.p + 1]))
        {
            c.p++;
            size_t ns = c.p;
            while (c.p < c.n && (isIdent(c.d[c.p]) || c.d[c.p] == ':' || c.d[c.p] == '-'
                || c.d[c.p] == '.')) { c.p++; }
            std::string tag(c.d + ns, c.p - ns);
            HtmlElementNode* el = ASTNodeFactory::createHtmlElementNode();
            el->m_tag = tag;
            bool closed = false;
            while (c.p < c.n && c.d[c.p] != '>')
            {
                while (c.p < c.n && (c.d[c.p] == ' ' || c.d[c.p] == '\t'
                    || c.d[c.p] == '\n' || c.d[c.p] == '\r')) { c.p++; }
                if (c.p + 1 < c.n && c.d[c.p] == '/' && c.d[c.p + 1] == '>')
                {
                    closed = true;
                    c.p += 2;
                    break;
                }
                if (c.p < c.n && c.d[c.p] == '>') { break; }
                if (c.p >= c.n) { break; }
                size_t as = c.p;
                while (c.p < c.n && !isHSpace(c.d[c.p]) && c.d[c.p] != '\n'
                    && c.d[c.p] != '>' && c.d[c.p] != '='
                    && !(c.d[c.p] == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '>')) { c.p++; }
                if (c.p < c.n && c.d[c.p] == '=')
                {
                    c.p++;
                    while (c.p < c.n && (c.d[c.p] == ' ' || c.d[c.p] == '\t')) { c.p++; }
                    if (c.p < c.n && (c.d[c.p] == '"' || c.d[c.p] == '\''))
                    {
                        char q = c.d[c.p];
                        c.p++;
                        while (c.p < c.n && c.d[c.p] != q) { c.p++; }
                        if (c.p < c.n) { c.p++; }
                    }
                    else
                    {
                        while (c.p < c.n && !isHSpace(c.d[c.p]) && c.d[c.p] != '\n'
                            && c.d[c.p] != '>') { c.p++; }
                    }
                }
                std::string attr(c.d + as, c.p - as);
                if (!trimStr(attr).empty()) { el->m_attrs.push_back(trimStr(attr)); }
            }
            if (c.p < c.n && c.d[c.p] == '>') { c.p++; }
            if (closed || isHtmlVoid(tag))
            {
                el->m_selfClosing = true;
                htmlAttach(prog, stack, el);
                continue;
            }
            htmlAttach(prog, stack, el);
            if (isRawTextElement(tag))
            {
                // ponytail: script/style content is raw until the matching
                // close tag, so < and strings inside can't split the tree
                size_t start = c.p;
                std::string want = "</" + lowerStr(tag);
                while (c.p < c.n)
                {
                    if (c.d[c.p] == '<' && c.p + want.size() < c.n)
                    {
                        bool match = true;
                        for (size_t k = 0; k < want.size() && match; k++)
                        {
                            char a = c.d[c.p + k];
                            if (a >= 'A' && a <= 'Z') { a = static_cast<char>(a + 32); }
                            if (a != want[k]) { match = false; }
                        }
                        if (match) { break; }
                    }
                    c.p++;
                }
                std::string text = trimStr(std::string(c.d + start, c.p - start));
                if (!text.empty())
                {
                    RawTextNode* raw = ASTNodeFactory::createRawTextNode();
                    raw->m_text = text;
                    el->m_children.push_back(raw);
                }
                continue;
            }
            stack.push_back(el);
            continue;
        }
        // text node (a stray '<' that opens nothing is literal text)
        size_t start = c.p;
        if (c.p < c.n && c.d[c.p] == '<') { c.p++; }
        while (c.p < c.n && c.d[c.p] != '<') { c.p++; }
        std::string text = trimStr(std::string(c.d + start, c.p - start));
        if (!text.empty())
        {
            RawTextNode* raw = ASTNodeFactory::createRawTextNode();
            raw->m_text = text;
            htmlAttach(prog, stack, raw);
        }
    }
    return prog;
}

struct JsonCursor
{
    const char* d;
    size_t n;
    size_t p;
};

void skipJsonWs(JsonCursor& c)
{
    while (c.p < c.n && (c.d[c.p] == ' ' || c.d[c.p] == '\t'
        || c.d[c.p] == '\n' || c.d[c.p] == '\r')) { c.p++; }
}

ASTNode* parseJsonValue(JsonCursor& c);

ASTNode* parseJsonString(JsonCursor& c)
{
    // assumes current char is the opening quote
    c.p++;
    size_t start = c.p;
    while (c.p < c.n && c.d[c.p] != '"')
    {
        if (c.d[c.p] == '\\' && c.p + 1 < c.n) { c.p += 2; continue; }
        c.p++;
    }
    std::string inner(c.d + start, c.p > start ? c.p - start : 0);
    if (c.p < c.n) { c.p++; }
    else { return NULL; }
    return mkStr(inner);
}

ASTNode* parseJsonValue(JsonCursor& c)
{
    skipJsonWs(c);
    if (c.p >= c.n) { return NULL; }
    char ch = c.d[c.p];
    if (ch == '"') { return parseJsonString(c); }
    if (ch == '{')
    {
        c.p++;
        ObjectLiteralNode* obj = ASTNodeFactory::createObjectLiteralNode();
        skipJsonWs(c);
        if (c.p < c.n && c.d[c.p] == '}')
        {
            c.p++;
            return obj;
        }
        for (;;)
        {
            skipJsonWs(c);
            if (c.p >= c.n || c.d[c.p] != '"') { return NULL; }
            ASTNode* key = parseJsonString(c);
            if (key == NULL) { return NULL; }
            skipJsonWs(c);
            if (c.p >= c.n || c.d[c.p] != ':') { return NULL; }
            c.p++;
            ASTNode* value = parseJsonValue(c);
            if (value == NULL) { return NULL; }
            PropertyNode* prop = ASTNodeFactory::createPropertyNode();
            prop->m_key = key;
            prop->m_value = value;
            obj->m_properties.push_back(prop);
            skipJsonWs(c);
            if (c.p < c.n && c.d[c.p] == ',') { c.p++; continue; }
            if (c.p < c.n && c.d[c.p] == '}') { c.p++; return obj; }
            return NULL;
        }
    }
    if (ch == '[')
    {
        c.p++;
        ArrayLiteralNode* arr = ASTNodeFactory::createArrayLiteralNode();
        skipJsonWs(c);
        if (c.p < c.n && c.d[c.p] == ']')
        {
            c.p++;
            return arr;
        }
        for (;;)
        {
            ASTNode* value = parseJsonValue(c);
            if (value == NULL) { return NULL; }
            arr->m_elements.push_back(value);
            skipJsonWs(c);
            if (c.p < c.n && c.d[c.p] == ',') { c.p++; continue; }
            if (c.p < c.n && c.d[c.p] == ']') { c.p++; return arr; }
            return NULL;
        }
    }
    if ((ch >= '0' && ch <= '9') || ch == '-')
    {
        size_t start = c.p;
        while (c.p < c.n && ((c.d[c.p] >= '0' && c.d[c.p] <= '9')
            || c.d[c.p] == '.' || c.d[c.p] == '-' || c.d[c.p] == '+'
            || c.d[c.p] == 'e' || c.d[c.p] == 'E')) { c.p++; }
        return mkNumber(std::string(c.d + start, c.p - start));
    }
    if (c.n - c.p >= 4 && strncmp(c.d + c.p, "true", 4) == 0) { c.p += 4; return mkBool("true"); }
    if (c.n - c.p >= 5 && strncmp(c.d + c.p, "false", 5) == 0) { c.p += 5; return mkBool("false"); }
    if (c.n - c.p >= 4 && strncmp(c.d + c.p, "null", 4) == 0) { c.p += 4; return mkNull(); }
    return NULL;
}

ProgramNode* parseJsonDoc(const char* src, size_t len)
{
    std::string normalized;
    if (normalizeNewlines(src, len, normalized)) { src = normalized.c_str(); len = normalized.size(); }
    JsonCursor c;
    c.d = src;
    c.n = len;
    c.p = 0;
    ASTNode* value = parseJsonValue(c);
    if (value == NULL) { return NULL; }
    skipJsonWs(c);
    if (c.p != c.n) { return NULL; }
    ProgramNode* prog = ASTNodeFactory::createProgramNode();
    prog->m_statements.push_back(value);
    return prog;
}


void parseOne(Cursor& c, std::vector<ASTNode*>& out, bool& end)
{
    end = false;
    // ponytail: newlines are never significant at item boundaries
    // (no-semi termination happens inside the scanners, not here)
    while (c.p < c.n && (isHSpace(c.d[c.p]) || c.d[c.p] == '\n')) { c.p++; }
    size_t isp = c.p;
    while (isp > 0 && (c.d[isp - 1] == ' ' || c.d[isp - 1] == '\t')) { isp--; }
    bool lineStart = (isp == 0 || c.d[isp - 1] == '\n');
    if (c.p >= c.n)
    {
        end = true;
        return;
    }
    if (c.d[c.p] == '}')
    {
        end = true;
        return;
    }
    if (c.d[c.p] == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '/'
        && hasSlashComment(c.prof))
    {
        size_t start = c.p + 2;
        while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
        CommentNode* node = ASTNodeFactory::createCommentNode();
        node->m_text = std::string(c.d + start, c.p - start);
        node->m_isBlock = false;
        out.push_back(node);
        return;
    }
    if (c.d[c.p] == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '*')
    {
        size_t start = c.p + 2;
        c.p += 2;
        int depth = 1;
        bool nest = (strcmp(c.prof->code, "rs") == 0);
        while (c.p < c.n && depth > 0)
        {
            if (nest && c.d[c.p] == '/' && c.p + 1 < c.n && c.d[c.p + 1] == '*') { depth++; c.p += 2; }
            else if (c.d[c.p] == '*' && c.p + 1 < c.n && c.d[c.p + 1] == '/') { depth--; c.p += 2; }
            else { c.p++; }
        }
        CommentNode* node = ASTNodeFactory::createCommentNode();
        size_t stop = (c.p >= 2) ? c.p - 2 : c.p;
        node->m_text = std::string(c.d + start, stop > start ? stop - start : 0);
        node->m_isBlock = true;
        out.push_back(node);
        return;
    }
    if (lineStart && c.p + 1 < c.n && c.d[c.p] == '#' && c.d[c.p + 1] == '!')
    {
        out.push_back(mkRaw(consumeLogicalLine(c)));
        return;
    }
    if (c.d[c.p] == '<' && c.p + 1 < c.n && c.d[c.p + 1] == '?')
    {
        PreprocessorDirectiveNode* node = ASTNodeFactory::createPreprocessorDirectiveNode();
        node->m_directive = trimStr(consumeLogicalLine(c));
        out.push_back(node);
        return;
    }
    if (trimStr(consumeLogicalLinePreview(c)) == "?>")
    {
        PreprocessorDirectiveNode* node = ASTNodeFactory::createPreprocessorDirectiveNode();
        node->m_directive = trimStr(consumeLogicalLine(c));
        out.push_back(node);
        return;
    }
    if (c.prof->hashDirectives && lineStart && c.d[c.p] == '#')
    {
        PreprocessorDirectiveNode* node = ASTNodeFactory::createPreprocessorDirectiveNode();
        node->m_directive = trimStr(consumeLogicalLine(c));
        out.push_back(node);
        return;
    }
    std::string w = peekWord(c);
    if (c.prof->attrLines && !w.empty() && w[0] == '@')
    {
        // ponytail: type keywords keep their @ (@interface is a
        // declaration, not a decorator); check both spellings
        std::string bare = w.substr(1);
        if (!hasWord(c.prof->classWords, bare) && !hasWord(c.prof->ifaceWords, bare)
            && !hasWord(c.prof->enumWords, bare)
            && !hasWord(c.prof->classWords, w) && !hasWord(c.prof->ifaceWords, w)
            && !hasWord(c.prof->enumWords, w))
        {
            out.push_back(mkRaw(readBalancedChunk(c)));
            return;
        }
    }
    if (c.prof->attrLines && c.d[c.p] == '[')
    {
        size_t ls = c.p;
        size_t i = c.p;
        int depth = 0;
        char quote = 0;
        bool okLine = false;
        while (i < c.n && c.d[i] != '\n')
        {
            char ch = c.d[i];
            if (quote)
            {
                if (ch == '\\') { i++; }
                else if (ch == quote) { quote = 0; }
            }
            else if (ch == '"' || ch == '\'') { quote = ch; }
            else if (ch == '[') { depth++; }
            else if (ch == ']')
            {
                depth--;
                if (depth == 0)
                {
                    size_t j = i + 1;
                    while (j < c.n && (c.d[j] == ' ' || c.d[j] == '\t')) { j++; }
                    if (j >= c.n || c.d[j] == '\n') { okLine = true; }
                    break;
                }
            }
            i++;
        }
        if (okLine)
        {
            while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
            out.push_back(mkRaw(std::string(c.d + ls, c.p - ls)));
            if (c.p < c.n) { c.p++; }
            return;
        }
    }
    if (w == "export")
    {
        parseExportLine(c, out);
        return;
    }
    if (hasWord(c.prof->importWords, w))
    {
        parseImportLine(c, out, w);
        return;
    }
    if (c.prof->namespaceWord != NULL && w == c.prof->namespaceWord)
    {
        // ponytail: dotted calls rewind and fall through to statements
        if (parseNamespaceLine(c, out, w)) { return; }
    }
    if (w == "if") { c.p += 2; parseIfRest(c, out); return; }
    if (w == "for" || w == "foreach") { c.p += w.size(); parseForRest(c, out, w); return; }
    if (w == "while") { c.p += 5; parseWhileRest(c, out); return; }
    if (w == "do") { c.p += 2; parseDoRest(c, out); return; }
    if (c.prof->switchWord != NULL && !w.empty() && w == c.prof->switchWord)
    {
        c.p += w.size();
        parseSwitchRest(c, out, w);
        return;
    }
    // ponytail: Go select shares the switch machinery (case/default arms);
    // anything else named select falls through to statements below
    if (strcmp(c.prof->code, "go") == 0 && w == "select")
    {
        c.p += w.size();
        parseSwitchRest(c, out, w);
        return;
    }
    if (w == "try") { c.p += 3; parseTryRest(c, out); return; }
    if (w == "catch" || w == "finally")
    {
        bool isFin = (w == "finally");
        c.p += w.size();
        TryCatchStmtNode* node = ASTNodeFactory::createTryCatchStmtNode();
        parseCatchInto(c, node, true, isFin);
        out.push_back(node);
        return;
    }
    if (w == "return" || w == "throw" || w == "else")
    {
        if (w == "else")
        {
            out.push_back(mkRaw("else"));
            c.p += 4;
            return;
        }
        std::string tail;
        if (c.prof->semicolons)
        {
            tail = consumeThroughSemi(c);
            if (!tail.empty() && tail[tail.size() - 1] == ';')
            {
                tail = trimStr(tail.substr(0, tail.size() - 1));
            }
        }
        else
        {
            tail = readBalancedChunk(c);
        }
        pushStatement(c, out, tail);
        return;
    }
    if (c.prof->typeWord != NULL && w == c.prof->typeWord)
    {
        parseTypeStmt(c, out);
        return;
    }
    // generic statement-or-block scan
    size_t stmtStart = c.p;
    for (;;)
    {
        size_t termPos = 0;
        char term = scanTerminator(c, termPos);
        if (term == '{')
        {
            std::string H = trimStr(std::string(c.d + stmtStart, termPos - stmtStart));
            // ponytail: "= {" and "return {" open an expression (object
            // literal, composite value), not a block; a lone const/let/var
            // opens a destructuring pattern. Skip the group opaque and keep
            // scanning for the statement end instead of splitting there.
            // (findAssign is depth-aware, so for/if headers never qualify;
            // same-line only, so ASI return-newline keeps working.)
            bool exprBrace = (findAssign(H) != std::string::npos)
                || startsWithWord(H, "return");
            if (!exprBrace && (H == "const" || H == "let" || H == "var"))
            {
                exprBrace = true;
            }
            if (exprBrace)
            {
                c.p = termPos;
                if (skipBalancedBraces(c)) { continue; }
                c.p = termPos;
            }
            parseHeaderBlock(c, out, std::string(c.d + stmtStart, termPos - stmtStart), stmtStart);
            return;
        }
    if (term == ';')
    {
        std::string text(c.d + stmtStart, termPos - stmtStart + 1);
        c.p = termPos + 1;
        pushStatement(c, out, text);
        return;
    }
    if (term == '\n')
    {
        std::string text(c.d + stmtStart, termPos - stmtStart);
        c.p = termPos + 1;
        if (trimStr(text).empty()) { return; }
        pushStatement(c, out, text);
        return;
    }
    if (term == '}')
    {
        std::string text(c.d + stmtStart, termPos - stmtStart);
        c.p = termPos;
        pushStatement(c, out, text);
        return;
    }
    // EOF: tolerate a missing terminator (returns: nothing follows EOF)
    std::string text(c.d + stmtStart, termPos - stmtStart);
    c.p = termPos;
    if (trimStr(text).empty())
    {
        end = true;
        return;
    }
    pushStatement(c, out, text);
    end = true;
    return;
    }
}

// --- Indentation engine (Python) ---

void parsePythonItems(Cursor& c, std::vector<ASTNode*>& out, int baseIndent,
    std::vector<ASTNode*>& pending);
void parsePythonLineText(Cursor& c, std::vector<ASTNode*>& out, const std::string& text,
    int indent, std::vector<ASTNode*>& pending, IfStmtNode** heldIf,
    TryCatchStmtNode** heldTry, ForStmtNode** heldFor,
    WhileStmtNode** heldWhile);

// --- Ruby end-delimited engine (see below, past the Python engine) ---

struct RubyHolders
{
    IfStmtNode* iff;
    TryCatchStmtNode* tcb;
    SwitchStmtNode* sw;
    FunctionDeclNode* def;
    // which holder owns a trailing else: 0 none, 1 if, 2 case, 3 begin,
    // 4 def (method-level rescue/ensure with no begin)
    int kind;
};

int parseRubyItems(Cursor& c, std::vector<ASTNode*>& out,
    std::vector<ASTNode*>& pending, std::vector<std::string>& segs,
    size_t& pos, bool topLevel);

int parseRubySegment(Cursor& c, std::vector<ASTNode*>& out, const std::string& t,
    std::vector<ASTNode*>& pending, std::vector<std::string>& segs, size_t& pos,
    RubyHolders* held, bool topLevel);

// Splits a suite remainder (after :) into ;-separated statements.
void parseSuiteRest(Cursor& c, const std::string& rest, BlockNode* suite)
{
    std::vector<std::string> parts;
    splitTopLevel(rest, ';', parts);
    for (size_t i = 0; i < parts.size(); i++)
    {
        std::string t = trimStr(parts[i]);
        if (t.empty()) { continue; }
        ASTNode* stmt = parseStatementText(c.prof, t);
        if (stmt != NULL) { suite->m_statements.push_back(stmt); }
    }
}

// Strips one trailing suite colon plus an optional trailing comment.
// Comment text (if any) goes to trailingComment.
std::string stripSuiteColon(const std::string& text, std::string& trailingComment)
{
    trailingComment.clear();
    std::string t = trimStr(text);
    size_t hash = std::string::npos;
    {
        char quote = 0;
        for (size_t i = 0; i < t.size(); i++)
        {
            char ch = t[i];
            if (quote)
            {
                if (ch == '\\') { i++; }
                else if (ch == quote) { quote = 0; }
                continue;
            }
            if (ch == '"' || ch == '\'') { quote = ch; continue; }
            if (ch == '#') { hash = i; break; }
        }
    }
    std::string code = t;
    if (hash != std::string::npos)
    {
        trailingComment = trimStr(t.substr(hash));
        code = trimStr(t.substr(0, hash));
    }
    if (!code.empty() && code[code.size() - 1] == ':')
    {
        code = trimStr(code.substr(0, code.size() - 1));
    }
    return code;
}

// Skips one Python lambda header starting at the 'l'. Returns the position
// of the lambda's own colon, or npos when this is not a lambda expression
// (comparison, call, attribute, plain name...). Only flat params are
// consumed (words, commas, stars, dots, single-= defaults); nested lambdas
// recurse, while brackets/strings stop the scan (the main scanner tracks
// those correctly itself, e.g. defaults holding dicts or slices).
size_t skipPythonLambda(const std::string& t, size_t at)
{
    size_t n = t.size();
    size_t p = at + 6; // past "lambda"
    for (;;)
    {
        while (p < n && (t[p] == ' ' || t[p] == '\t' || t[p] == '\n')) { p++; }
        if (p >= n) { return std::string::npos; }
        char ch = t[p];
        if (ch == ':') { return p; }
        if (ch == ',' || ch == '*') { p++; continue; }
        if (ch == '=' && !(p + 1 < n && t[p + 1] == '='))
        {
            // ponytail: single = is a default; == is a comparison (bail,
            // the main scanner then finds the real suite colon itself)
            p++;
            continue;
        }
        if (ch == '.' || ch == '_' || isAlpha(ch) || isDigit(ch))
        {
            size_t ws = p;
            while (p < n && (isIdent(t[p]) && t[p] != '$')) { p++; }
            std::string word(t.data() + ws, p - ws);
            bool beforeOk = (ws == 0) || (!isIdent(t[ws - 1]) && t[ws - 1] != '.');
            bool afterOk = (p >= n) || !isIdent(t[p]);
            if (word == "lambda" && beforeOk && afterOk)
            {
                size_t past = skipPythonLambda(t, ws);
                if (past == std::string::npos) { return std::string::npos; }
                p = past + 1;
                continue;
            }
            continue;
        }
        return std::string::npos;
    }
}

// Splits "header: suite..." at the FIRST depth-0 colon (string and
// paren/bracket/brace aware). A suite rest holding only a # comment
// counts as the trailing comment; a # comment inside the header is
// lifted too. False when there is no suite colon.
bool splitSuite(const std::string& t, size_t from, std::string& header,
    std::string& body, std::string& tc)
{
    tc.clear();
    header = t;
    body.clear();
    char quote = 0;
    int paren = 0, brack = 0, brace = 0;
    size_t colon = std::string::npos;
    for (size_t i = from; i < t.size(); i++)
    {
        char ch = t[i];
        if (quote)
        {
            if (ch == '\\') { i++; }
            else if (ch == quote) { quote = 0; }
            continue;
        }
        if (ch == '"' || ch == '\'') { quote = ch; continue; }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if ((ch == '_' || isAlpha(ch)) && paren == 0 && brack == 0 && brace == 0)
        {
            // ponytail: a bare lambda's colon is not the suite colon
            // ("if lambda: 1: pass" conditions on the lambda itself)
            size_t ws = i;
            size_t j = i;
            while (j < t.size() && (isIdent(t[j]) && t[j] != '$')) { j++; }
            std::string word(t.data() + ws, j - ws);
            bool beforeOk = (ws == 0) || (!isIdent(t[ws - 1]) && t[ws - 1] != '.');
            bool afterOk = (j >= t.size()) || !isIdent(t[j]);
            i = j - 1;
            if (word == "lambda" && beforeOk && afterOk)
            {
                size_t past = skipPythonLambda(t, ws);
                if (past != std::string::npos) { i = past; }
            }
            continue;
        }
        else if (ch == ':' && paren == 0 && brack == 0 && brace == 0)
        {
            colon = i;
            break;
        }
    }
    if (colon == std::string::npos) { return false; }
    header = trimStr(t.substr(0, colon));
    body = trimStr(t.substr(colon + 1));
    if (!body.empty() && body[0] == '#')
    {
        // ponytail: "if x:  # c" carries no inline suite, just a comment
        tc = body;
        body.clear();
    }
    std::string htc;
    header = stripSuiteColon(header, htc);
    if (tc.empty()) { tc = htc; }
    return true;
}

// Appends a trailing "# ..." comment (with the #) as a suite statement.
void pushSuiteComment(BlockNode* suite, const std::string& tc)
{
    if (suite == NULL || tc.empty()) { return; }
    CommentNode* cm = ASTNodeFactory::createCommentNode();
    cm->m_text = tc.size() > 1 ? trimStr(tc.substr(1)) : std::string();
    cm->m_isBlock = false;
    suite->m_statements.push_back(cm);
}

// Reads one logical Python line: joins backslash and bracket continuations.
// Returns false at EOF. Comment-only lines come back flagged.
bool readPythonLogical(Cursor& c, std::string& text, int& indent, bool& isComment)
{
    text.clear();
    isComment = false;
    // skip blank lines entirely (they carry no structure)
    for (;;)
    {
        if (c.p >= c.n) { return false; }
        size_t ls = c.p;
        while (c.p < c.n && (c.d[c.p] == ' ' || c.d[c.p] == '\t')) { c.p++; }
        if (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '\r') { break; }
        while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
        if (c.p < c.n) { c.p++; }
        (void)ls;
    }
    // ponytail: c.p already skipped the leading whitespace, so walk back
    // to the line start before measuring (else every line reads as 0)
    {
        size_t ls = c.p;
        while (ls > 0 && (c.d[ls - 1] == ' ' || c.d[ls - 1] == '\t')) { ls--; }
        indent = measureIndent(c.d, c.n, ls);
    }
    // accumulate continuation lines
    std::string acc;
    int paren = 0, brack = 0, brace = 0;
    for (;;)
    {
        size_t ls = c.p;
        bool lineCont = false;
        while (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '\r')
        {
            char ch = c.d[c.p];
            if (ch == '"' || ch == '\'')
            {
                // ponytail: triple-quoted strings span lines; skip as a unit
                if (c.p + 2 < c.n && c.d[c.p + 1] == ch && c.d[c.p + 2] == ch)
                {
                    c.p += 3;
                    while (c.p + 2 < c.n
                        && !(c.d[c.p] == ch && c.d[c.p + 1] == ch && c.d[c.p + 2] == ch))
                    {
                        if (c.d[c.p] == '\\') { c.p++; }
                        c.p++;
                    }
                    if (c.p + 2 < c.n) { c.p += 3; }
                    continue;
                }
                Cursor tmp = c;
                skipString(tmp);
                c.p = tmp.p;
                continue;
            }
            // ponytail: trailing comments ride along verbatim inside leaves
            if (ch == '#')
            {
                while (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '\r') { c.p++; }
                break;
            }
            if (ch == '(') { paren++; }
            else if (ch == ')') { if (paren > 0) paren--; }
            else if (ch == '[') { brack++; }
            else if (ch == ']') { if (brack > 0) brack--; }
            else if (ch == '{') { brace++; }
            else if (ch == '}') { if (brace > 0) brace--; }
            else if (ch == '\\' && c.p + 1 < c.n
                && (c.d[c.p + 1] == '\n' || c.d[c.p + 1] == '\r'))
            {
                // ponytail: the backslash stays in the text (dropping it
                // would corrupt the statement); the join newline follows
                lineCont = true;
                c.p++;
                continue;
            }
            c.p++;
        }
        if (!acc.empty()) { acc += "\n"; }
        acc += std::string(c.d + ls, c.p - ls);
        if (c.p < c.n && c.d[c.p] == '\\') { c.p++; }
        if (c.p < c.n && c.d[c.p] == '\r')
        {
            c.p++;
            if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
        }
        else if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
        if (!lineCont && paren <= 0 && brack <= 0 && brace <= 0) { break; }
        if (c.p >= c.n) { break; }
    }
    text = acc;
    std::string trimmed = trimStr(text);
    if (!trimmed.empty() && trimmed[0] == '#')
    {
        isComment = true;
    }
    return true;
}

void parsePythonItems(Cursor& c, std::vector<ASTNode*>& out, int baseIndent,
    std::vector<ASTNode*>& pending)
{
    IfStmtNode* heldIf = NULL;
    TryCatchStmtNode* heldTry = NULL;
    ForStmtNode* heldFor = NULL;
    WhileStmtNode* heldWhile = NULL;
    for (;;)
    {
        std::string text;
        int indent = 0;
        bool isComment = false;
        size_t save = c.p;
        if (!readPythonLogical(c, text, indent, isComment))
        {
            break;
        }
        if (isComment)
        {
            // ponytail: comments buffer until the next real line decides
            // their level; a dangling tail flushes at the outer level
            CommentNode* cm = ASTNodeFactory::createCommentNode();
            std::string t = trimStr(text);
            cm->m_text = t.size() > 1 ? trimStr(t.substr(1)) : std::string();
            cm->m_isBlock = false;
            pending.push_back(cm);
            continue;
        }
        if (indent <= baseIndent)
        {
            // ponytail: dedent ends this suite; the line and the buffered
            // comments belong to the caller
            c.p = save;
            break;
        }
        // ponytail: no flush here; continuations inside parsePythonLineText
        // attach buffered comments to the suites they close, and only fresh
        // lines flush them as siblings (see below)
        parsePythonLineText(c, out, text, indent, pending, &heldIf, &heldTry,
            &heldFor, &heldWhile);
    }
    // ponytail: every held node is already pushed; chains simply end here
}

// Parses one "case PATTERN: [suite]" arm line into a match node.
// A colon-less or pattern-less arm stays verbatim (pattern intact).
void pushPythonMatchArm(Cursor& c, SwitchStmtNode* node, const std::string& line,
    int armIndent, std::vector<ASTNode*>& pending)
{
    std::string t = trimStr(line);
    std::string header;
    std::string bodyrest;
    std::string tc;
    if (firstWord(t) != "case" || !splitSuite(t, 4, header, bodyrest, tc)
        || trimStr(header.substr(4)).empty())
    {
        SwitchCaseNode* arm = ASTNodeFactory::createSwitchCaseNode();
        arm->m_pattern = mkRaw(t);
        arm->m_body = ASTNodeFactory::createBlockNode();
        node->m_cases.push_back(arm);
        return;
    }
    SwitchCaseNode* arm = ASTNodeFactory::createSwitchCaseNode();
    arm->m_pattern = mkRaw(header + ":");
    BlockNode* suite = ASTNodeFactory::createBlockNode();
    if (!bodyrest.empty())
    {
        parseSuiteRest(c, bodyrest, suite);
    }
    else
    {
        parsePythonItems(c, suite->m_statements, armIndent, pending);
    }
    pushSuiteComment(suite, tc);
    // ponytail: comments buffered before the arm open its suite
    for (size_t i = 0; i < pending.size(); i++)
    {
        suite->m_statements.insert(suite->m_statements.begin() + i, pending[i]);
    }
    pending.clear();
    arm->m_body = suite;
    node->m_cases.push_back(arm);
}

// Parses indented "case" arms under a match header (c.p past the header).
void parsePythonMatchArms(Cursor& c, SwitchStmtNode* node, int matchIndent,
    std::vector<ASTNode*>& pending)
{
    for (;;)
    {
        std::string text;
        int indent = 0;
        bool isComment = false;
        size_t save = c.p;
        if (!readPythonLogical(c, text, indent, isComment))
        {
            break;
        }
        if (isComment)
        {
            CommentNode* cm = ASTNodeFactory::createCommentNode();
            std::string t = trimStr(text);
            cm->m_text = t.size() > 1 ? trimStr(t.substr(1)) : std::string();
            cm->m_isBlock = false;
            if (!node->m_cases.empty() && node->m_cases.back() != NULL)
            {
                SwitchCaseNode* last = static_cast<SwitchCaseNode*>(
                    node->m_cases.back());
                if (last->m_body != NULL && last->m_body->isBlock())
                {
                    static_cast<BlockNode*>(last->m_body)->m_statements.push_back(cm);
                    continue;
                }
            }
            pending.push_back(cm);
            continue;
        }
        if (indent <= matchIndent)
        {
            c.p = save;
            break;
        }
        std::string t = trimStr(text);
        if (firstWord(t) != "case")
        {
            // ponytail: a non-arm line ends the match; the caller owns it
            c.p = save;
            break;
        }
        pushPythonMatchArm(c, node, t, indent, pending);
    }
}

// Parses one logical Python line already read (c.p is past it).
void parsePythonLineText(Cursor& c, std::vector<ASTNode*>& out, const std::string& text,
    int indent, std::vector<ASTNode*>& pending, IfStmtNode** heldIf,
    TryCatchStmtNode** heldTry, ForStmtNode** heldFor,
    WhileStmtNode** heldWhile)
{
    (void)indent;
    std::string t = trimStr(text);
    if (t.empty()) { return; }
    std::string w = firstWord(t);
    // 1. continuations of a held if (pending comments close the suite)
    if ((w == "elif" || w == "else") && heldIf != NULL && *heldIf != NULL
        && (*heldIf)->m_elseBranch == NULL)
    {
        IfStmtNode* target = *heldIf;
        *heldIf = NULL;
        BlockNode* thenSuite = NULL;
        if (target->m_thenBranch != NULL && target->m_thenBranch->isBlock())
        {
            thenSuite = static_cast<BlockNode*>(target->m_thenBranch);
        }
        if (thenSuite == NULL)
        {
            thenSuite = ASTNodeFactory::createBlockNode();
            target->m_thenBranch = thenSuite;
        }
        for (size_t i = 0; i < pending.size(); i++) { thenSuite->m_statements.push_back(pending[i]); }
        pending.clear();
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(t, w.size(), header, bodyrest, tc))
        {
            // ponytail: colon-less elif/else is invalid; keep it verbatim
            // after the complete if instead of dropping it
            out.push_back(target);
            out.push_back(mkRaw(t));
            return;
        }
        if (w == "else")
        {
            if (!trimStr(header.substr(4)).empty())
            {
                out.push_back(mkRaw(t));
                return;
            }
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!bodyrest.empty())
            {
                parseSuiteRest(c, bodyrest, suite);
            }
            else
            {
                parsePythonItems(c, suite->m_statements, indent, pending);
            }
            pushSuiteComment(suite, tc);
            target->m_elseBranch = suite;
        }
        else
        {
            std::string cond = trimStr(header.substr(4));
            if (cond.empty())
            {
                out.push_back(mkRaw(t));
                return;
            }
            IfStmtNode* elif = ASTNodeFactory::createIfStmtNode();
            elif->m_keyword = "elif";
            elif->m_condition = mkValue(cond, true);
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!bodyrest.empty())
            {
                parseSuiteRest(c, bodyrest, suite);
            }
            else
            {
                parsePythonItems(c, suite->m_statements, indent, pending);
            }
            pushSuiteComment(suite, tc);
            elif->m_thenBranch = suite;
            target->m_elseBranch = elif;
            *heldIf = elif;
        }
        return;
    }
    if ((w == "except" || w == "finally" || w == "else") && heldTry != NULL
        && *heldTry != NULL)
    {
        TryCatchStmtNode* target = *heldTry;
        *heldTry = NULL;
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(t, w.size(), header, bodyrest, tc))
        {
            // ponytail: colon-less except/finally/else is invalid; the try
            // is already pushed, so keep just the line verbatim
            out.push_back(mkRaw(t));
            return;
        }
        if (w == "finally")
        {
            if (!trimStr(header.substr(7)).empty())
            {
                out.push_back(mkRaw(t));
                return;
            }
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!bodyrest.empty())
            {
                parseSuiteRest(c, bodyrest, suite);
            }
            else
            {
                parsePythonItems(c, suite->m_statements, indent, pending);
            }
            pushSuiteComment(suite, tc);
            target->m_finallyBlock = suite;
            return;
        }
        if (w == "else" && target->m_finallyBlock == NULL
            && target->m_elseBlock == NULL)
        {
            if (!trimStr(header.substr(4)).empty())
            {
                out.push_back(mkRaw(t));
                return;
            }
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!bodyrest.empty())
            {
                parseSuiteRest(c, bodyrest, suite);
            }
            else
            {
                parsePythonItems(c, suite->m_statements, indent, pending);
            }
            pushSuiteComment(suite, tc);
            target->m_elseBlock = suite;
            *heldTry = target;
            return;
        }
        if (w == "except" && target->m_finallyBlock == NULL)
        {
            std::string rest = trimStr(header.substr(6));
            BlockNode* lastSuite = NULL;
            if (!target->m_extraCatches.empty() && target->m_extraCatches.back() != NULL)
            {
                TryCatchStmtNode* last = static_cast<TryCatchStmtNode*>(
                    target->m_extraCatches.back());
                if (last->m_catchBody != NULL && last->m_catchBody->isBlock())
                {
                    lastSuite = static_cast<BlockNode*>(last->m_catchBody);
                }
            }
            if (lastSuite == NULL && target->m_catchBody != NULL
                && target->m_catchBody->isBlock())
            {
                lastSuite = static_cast<BlockNode*>(target->m_catchBody);
            }
            if (lastSuite == NULL && target->m_body != NULL
                && target->m_body->isBlock())
            {
                lastSuite = static_cast<BlockNode*>(target->m_body);
            }
            if (lastSuite == NULL)
            {
                lastSuite = ASTNodeFactory::createBlockNode();
                target->m_body = lastSuite;
            }
            for (size_t i = 0; i < pending.size(); i++) { lastSuite->m_statements.push_back(pending[i]); }
            pending.clear();
            if (target->m_catchClause == NULL && target->m_catchBody == NULL
                && target->m_extraCatches.empty())
            {
                if (!rest.empty()) { target->m_catchClause = mkIdent(rest); }
                BlockNode* suite = ASTNodeFactory::createBlockNode();
                if (!bodyrest.empty())
                {
                    parseSuiteRest(c, bodyrest, suite);
                }
                else
                {
                    parsePythonItems(c, suite->m_statements, indent, pending);
                }
                pushSuiteComment(suite, tc);
                target->m_catchBody = suite;
            }
            else
            {
                TryCatchStmtNode* extra = ASTNodeFactory::createTryCatchStmtNode();
                if (!rest.empty()) { extra->m_catchClause = mkIdent(rest); }
                BlockNode* suite = ASTNodeFactory::createBlockNode();
                if (!bodyrest.empty())
                {
                    parseSuiteRest(c, bodyrest, suite);
                }
                else
                {
                    parsePythonItems(c, suite->m_statements, indent, pending);
                }
                pushSuiteComment(suite, tc);
                extra->m_catchBody = suite;
                target->m_extraCatches.push_back(extra);
            }
            *heldTry = target;
            return;
        }
        // ponytail: misplaced continuation (else/except after finally)
        // cannot attach; the try is already pushed, keep just the line
        out.push_back(mkRaw(t));
        return;
    }
    // 3. loop else: attaches to a held for/while, which is already complete
    if (w == "else" && (heldTry == NULL || *heldTry == NULL))
    {
        ASTNode* loop = NULL;
        ASTNode* loopBody = NULL;
        ASTNode** elseSlot = NULL;
        ForStmtNode** forHolder = NULL;
        WhileStmtNode** whileHolder = NULL;
        if (heldFor != NULL && *heldFor != NULL
            && (*heldFor)->m_elseBlock == NULL)
        {
            loop = *heldFor;
            loopBody = (*heldFor)->m_body;
            elseSlot = &(*heldFor)->m_elseBlock;
            forHolder = heldFor;
        }
        else if (heldWhile != NULL && *heldWhile != NULL
            && (*heldWhile)->m_elseBlock == NULL)
        {
            loop = *heldWhile;
            loopBody = (*heldWhile)->m_body;
            elseSlot = &(*heldWhile)->m_elseBlock;
            whileHolder = heldWhile;
        }
        if (elseSlot != NULL)
        {
            std::string header;
            std::string bodyrest;
            std::string tc;
            if (!splitSuite(t, 4, header, bodyrest, tc)
                || !trimStr(header.substr(4)).empty())
            {
                out.push_back(mkRaw(t));
                return;
            }
            if (loopBody != NULL && loopBody->isBlock())
            {
                BlockNode* lb = static_cast<BlockNode*>(loopBody);
                for (size_t i = 0; i < pending.size(); i++) { lb->m_statements.push_back(pending[i]); }
                pending.clear();
            }
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!bodyrest.empty())
            {
                parseSuiteRest(c, bodyrest, suite);
            }
            else
            {
                parsePythonItems(c, suite->m_statements, indent, pending);
            }
            pushSuiteComment(suite, tc);
            *elseSlot = suite;
            if (forHolder != NULL) { *forHolder = NULL; }
            if (whileHolder != NULL) { *whileHolder = NULL; }
            return;
        }
    }
    // ponytail: holders only track open chains for continuations; every
    // held node is already pushed, so a fresh line just closes the chains.
    // Buffered comments flush here (not earlier), so continuations above
    // could still claim them for the suites they close.
    if (heldIf != NULL) { *heldIf = NULL; }
    if (heldTry != NULL) { *heldTry = NULL; }
    if (heldFor != NULL) { *heldFor = NULL; }
    if (heldWhile != NULL) { *heldWhile = NULL; }
    for (size_t i = 0; i < pending.size(); i++) { out.push_back(pending[i]); }
    pending.clear();
    for (size_t i = 0; i < pending.size(); i++) { out.push_back(pending[i]); }
    pending.clear();
    // 4. fresh line dispatch below; async only prefixes def/for/with,
    // anything else keeps the line whole for the verbatim fallback
    std::string asyncPrefix;
    std::string et = t;
    if (w == "async")
    {
        std::string r2 = trimStr(t.substr(5));
        std::string w2 = firstWord(r2);
        if (w2 == "def" || w2 == "for" || w2 == "with")
        {
            asyncPrefix = "async ";
            et = r2;
            w = w2;
        }
    }
    if (!t.empty() && t[0] == '@')
    {
        // ponytail: decorators ride along verbatim; the def below stays clean
        out.push_back(mkRaw(t));
        return;
    }
    if (w == "def")
    {
        // suite colon is the first depth-0 ':' past "def"; the body rest
        // (if any) is the inline suite
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 3, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        // header is now `def name(params) [-> ret]` minus colon and comment
        std::string rest = trimStr(header.substr(3));
        size_t lp = findTopLevel(rest, '(');
        std::string modifiers;
        std::string name;
        std::string params;
        std::string returnType;
        bool badDef = true;
        if (lp != std::string::npos)
        {
            // ponytail: match the parameter parens structurally, so default
            // values holding colons (lambdas, slices, dicts) cannot split
            // the header, and the "-> ret" trailer is read exactly
            std::string pre = trimStr(rest.substr(0, lp));
            size_t nsp = pre.find_last_of(" \t\n");
            std::string cand = (nsp == std::string::npos) ? pre : trimStr(pre.substr(nsp + 1));
            std::string mods = (nsp == std::string::npos) ? std::string() : trimStr(pre.substr(0, nsp));
            // ponytail: the name is one word; anything else is not a def
            if (!cand.empty() && cand.find_first_of(" \t\n(){};,") == std::string::npos)
            {
                Cursor sub = c;
                sub.d = rest.c_str();
                sub.n = rest.size();
                sub.p = lp;
                if (readParenGroup(sub, params))
                {
                    std::string trailer = trimStr(rest.substr(sub.p));
                    if (trailer.empty())
                    {
                        name = cand;
                        modifiers = mods;
                        badDef = false;
                    }
                    else if (trailer.size() > 2 && trailer[0] == '-'
                        && trailer[1] == '>')
                    {
                        std::string rt = trimStr(trailer.substr(2));
                        if (!rt.empty())
                        {
                            name = cand;
                            modifiers = mods;
                            returnType = rt;
                            badDef = false;
                        }
                    }
                }
            }
        }
        if (badDef)
        {
            out.push_back(mkRaw(t));
            return;
        }
        FunctionDeclNode* node = ASTNodeFactory::createFunctionDeclNode();
        node->m_name = name;
        node->m_modifiers = asyncPrefix + modifiers;
        node->m_returnType = returnType;
        if (!trimStr(params).empty()) { node->m_params.push_back(mkIdent(trimStr(params))); }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!bodyrest.empty())
        {
            parseSuiteRest(c, bodyrest, suite);
        }
        else
        {
            parsePythonItems(c, suite->m_statements, indent, pending);
        }
        pushSuiteComment(suite, tc);
        node->m_body = suite;
        out.push_back(node);
        return;
    }
    if (w == "class")
    {
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 5, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        std::string rest = trimStr(header.substr(5));
        // name runs to '(' or end; bases live inside the parens
        std::string name = rest;
        std::string bases;
        size_t lp = findTopLevel(rest, '(');
        if (lp != std::string::npos)
        {
            name = trimStr(rest.substr(0, lp));
            size_t rp = rest.rfind(')');
            if (rp == std::string::npos || rp < lp)
            {
                out.push_back(mkRaw(t));
                return;
            }
            bases = trimStr(rest.substr(lp + 1, rp - lp - 1));
        }
        if (name.empty() || name.find_first_of(" \t\n") != std::string::npos)
        {
            out.push_back(mkRaw(t));
            return;
        }
        ClassDeclNode* node = ASTNodeFactory::createClassDeclNode();
        node->m_name = name;
        if (!bases.empty())
        {
            std::vector<std::string> parts;
            splitTopLevel(bases, ',', parts);
            for (size_t i = 0; i < parts.size(); i++)
            {
                if (parts[i].empty()) { continue; }
                if (node->m_superClass == NULL) { node->m_superClass = mkIdent(parts[i]); }
                else { node->m_implements.push_back(mkIdent(parts[i])); }
            }
        }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!bodyrest.empty())
        {
            parseSuiteRest(c, bodyrest, suite);
        }
        else
        {
            parsePythonItems(c, suite->m_statements, indent, pending);
        }
        pushSuiteComment(suite, tc);
        node->m_body = suite;
        out.push_back(node);
        return;
    }
    if (w == "if")
    {
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 2, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        std::string cond = trimStr(header.substr(2));
        if (cond.empty())
        {
            out.push_back(mkRaw(t));
            return;
        }
        IfStmtNode* node = ASTNodeFactory::createIfStmtNode();
        node->m_condition = mkValue(cond, true);
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!bodyrest.empty())
        {
            parseSuiteRest(c, bodyrest, suite);
        }
        else
        {
            parsePythonItems(c, suite->m_statements, indent, pending);
        }
        pushSuiteComment(suite, tc);
        node->m_thenBranch = suite;
        // ponytail: pushed now, held for elif/else; continuations mutate
        // the pushed node in place, so nothing is ever pushed twice
        out.push_back(node);
        *heldIf = node;
        return;
    }
    if (w == "for")
    {
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 3, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        std::string cond = trimStr(header.substr(3));
        if (cond.empty())
        {
            out.push_back(mkRaw(t));
            return;
        }
        ForStmtNode* node = ASTNodeFactory::createForStmtNode();
        node->m_keyword = asyncPrefix + "for";
        node->m_classic = false;
        node->m_condition = mkValue(cond, true);
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!bodyrest.empty())
        {
            parseSuiteRest(c, bodyrest, suite);
        }
        else
        {
            parsePythonItems(c, suite->m_statements, indent, pending);
        }
        pushSuiteComment(suite, tc);
        node->m_body = suite;
        // ponytail: pushed now and held for a loop else (same protocol)
        out.push_back(node);
        if (heldFor != NULL) { *heldFor = node; }
        return;
    }
    if (w == "while")
    {
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 5, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        std::string cond = trimStr(header.substr(5));
        if (cond.empty())
        {
            out.push_back(mkRaw(t));
            return;
        }
        WhileStmtNode* node = ASTNodeFactory::createWhileStmtNode();
        node->m_condition = mkValue(cond, true);
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!bodyrest.empty())
        {
            parseSuiteRest(c, bodyrest, suite);
        }
        else
        {
            parsePythonItems(c, suite->m_statements, indent, pending);
        }
        pushSuiteComment(suite, tc);
        node->m_body = suite;
        // ponytail: pushed now and held for a loop else (same protocol)
        out.push_back(node);
        if (heldWhile != NULL) { *heldWhile = node; }
        return;
    }
    if (w == "try")
    {
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 3, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        if (!trimStr(header.substr(3)).empty())
        {
            out.push_back(mkRaw(t));
            return;
        }
        TryCatchStmtNode* node = ASTNodeFactory::createTryCatchStmtNode();
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!bodyrest.empty())
        {
            parseSuiteRest(c, bodyrest, suite);
        }
        else
        {
            parsePythonItems(c, suite->m_statements, indent, pending);
        }
        pushSuiteComment(suite, tc);
        node->m_body = suite;
        // ponytail: pushed now, held for except/else/finally below
        out.push_back(node);
        *heldTry = node;
        return;
    }
    if (w == "with")
    {
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 4, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        std::string ctx = trimStr(header.substr(4));
        if (ctx.empty())
        {
            out.push_back(mkRaw(t));
            return;
        }
        WithNode* node = ASTNodeFactory::createWithNode();
        node->m_keyword = asyncPrefix + "with";
        node->m_context = mkValue(ctx, true);
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!bodyrest.empty())
        {
            parseSuiteRest(c, bodyrest, suite);
        }
        else
        {
            parsePythonItems(c, suite->m_statements, indent, pending);
        }
        pushSuiteComment(suite, tc);
        node->m_body = suite;
        out.push_back(node);
        return;
    }
    if (w == "match")
    {
        std::string header;
        std::string bodyrest;
        std::string tc;
        if (!splitSuite(et, 5, header, bodyrest, tc))
        {
            out.push_back(mkRaw(t));
            return;
        }
        std::string disc = trimStr(header.substr(5));
        if (disc.empty())
        {
            out.push_back(mkRaw(t));
            return;
        }
        SwitchStmtNode* node = ASTNodeFactory::createSwitchStmtNode();
        node->m_discriminant = mkValue(disc, true);
        if (!bodyrest.empty())
        {
            // ponytail: "match x: case 1: y" nests its one arm inline
            pushPythonMatchArm(c, node, bodyrest, indent, pending);
        }
        else if (!tc.empty())
        {
            CommentNode* cm = ASTNodeFactory::createCommentNode();
            cm->m_text = tc.size() > 1 ? trimStr(tc.substr(1)) : std::string();
            cm->m_isBlock = false;
            pending.push_back(cm);
        }
        parsePythonMatchArms(c, node, indent, pending);
        out.push_back(node);
        return;
    }
    if (w == "import" || w == "from")
    {
        // ponytail: comments ride inside the raw rest; the formatter splits
        // source/specifiers back out, so nothing is lost either way
        std::string rest = trimStr(et.substr(w.size()));
        if (rest.empty())
        {
            out.push_back(mkRaw(t));
            return;
        }
        ImportDeclNode* node = ASTNodeFactory::createImportDeclNode();
        if (w == "from")
        {
            size_t im = rest.find(" import ");
            if (im == std::string::npos)
            {
                out.push_back(mkRaw(t));
                return;
            }
            node->m_source = trimStr(rest.substr(0, im));
            std::string specs = trimStr(rest.substr(im + 8));
            if (!specs.empty() && specs[0] == '('
                && specs[specs.size() - 1] == ')')
            {
                specs = trimStr(specs.substr(1, specs.size() - 2));
            }
            std::vector<std::string> parts;
            splitTopLevel(specs, ',', parts);
            for (size_t i = 0; i < parts.size(); i++)
            {
                if (parts[i].empty()) { continue; }
                node->m_specifiers.push_back(mkValue(parts[i], true));
            }
            if (node->m_source.empty() || node->m_specifiers.empty())
            {
                out.push_back(mkRaw(t));
                return;
            }
        }
        else
        {
            // ponytail: "import a as b" splits cleanly; "import a, b" stays
            // one verbatim source, which the formatter reprints as-is
            std::string src = rest;
            std::string alias;
            size_t ap = rest.find(" as ");
            if (ap != std::string::npos)
            {
                src = trimStr(rest.substr(0, ap));
                alias = trimStr(rest.substr(ap + 4));
            }
            if (src.empty() || src.find(',') == std::string::npos)
            {
                node->m_source = src;
                if (!alias.empty()) { node->m_defaultImport = mkIdent(alias); }
            }
            else if (alias.empty())
            {
                node->m_source = src;
            }
            else
            {
                out.push_back(mkRaw(t));
                return;
            }
            if (node->m_source.empty())
            {
                out.push_back(mkRaw(t));
                return;
            }
        }
        out.push_back(node);
        return;
    }
    // ponytail: return/raise/assignments/calls/expressions all round-trip
    // through the shared statement parser (trailing comments included)
    {
        ASTNode* stmt = parseStatementText(c.prof, t);
        if (stmt != NULL)
        {
            out.push_back(stmt);
            return;
        }
        out.push_back(mkRaw(t));
        return;
    }
}

} // namespace

Parser::Parser()
    : m_parser(NULL)
    , m_language(NULL)
    , m_isValid(true)
{
}

Parser::~Parser()
{
}

ProgramNode* Parser::parse(const char* source, size_t len)
{
    return parse(source, len, "js");
}

ProgramNode* Parser::parse(const char* source, size_t len, const std::string& language)
{
    if (source == NULL || len == 0)
    {
        return NULL;
    }

    if (g_pool != NULL)
    {
        g_pool->reset();
    }

    return parseNested(source, len, language);
}

ProgramNode* Parser::parse(const SourceBuffer& source)
{
    return parse(source.getData(), source.getSize(), "js");
}

ProgramNode* Parser::parse(const SourceBuffer& source, const std::string& language)
{
    return parse(source.getData(), source.getSize(), language);
}

namespace {

// --- Ruby end-delimited engine ---
//
// Segments (not lines) are the unit: each logical line splits on top-level
// ';', so one-liners (def f; body; end) flow through the same path as
// multi-line code. Openers consume their bodies via recursion; the recursion
// stops (rewound) on continuations and consumes the closing end.

// Skips one Ruby %-literal starting at '%' (opaque: brackets, semicolons,
// quotes and = signs inside never count). True when a literal opened;
// plain % (modulo, format, %=) is left alone. Paired delimeters nest,
// anything else closes on its twin, backslashes escape. Safe for other
// languages too: outside Ruby a glued %letter+delim span is never valid
// code, while %, %= and letter forms without delimeters step over plainly.
bool skipRubyPercent(const char* d, size_t n, size_t& p)
{
    // caller guarantees d[p] == '%'
    size_t q = p + 1;
    if (q < n && d[q] == '%') { p += 2; return true; }
    if (q < n && isAlpha(d[q]))
    {
        char m = d[q];
        if (m != 'q' && m != 'Q' && m != 'w' && m != 'W' && m != 'i'
            && m != 'I' && m != 'r' && m != 's' && m != 'x' && m != 'X')
        {
            return false;
        }
        q++;
    }
    if (q >= n) { return false; }
    char open = d[q];
    if (open == ' ' || open == '\t' || open == '\n' || open == '\r'
        || open == '=' || isAlpha(open) || isDigit(open) || open == '_')
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
        char ch = d[q];
        if (ch == '\\' && q + 1 < n) { q += 2; continue; }
        if (open == close)
        {
            if (ch == close) { p = q + 1; return true; }
        }
        else if (ch == open) { depth++; }
        else if (ch == close)
        {
            if (depth == 0) { p = q + 1; return true; }
            depth--;
        }
        q++;
    }
    // ponytail: unterminated runs to the end of the scanned text; content
    // stays verbatim either way, it just neither splits nor joins on it
    p = q;
    return true;
}

// Collects <<[-~]?["']?WORD heredoc markers from one code line.
// String-aware (strings stay opaque); stops at #. << needs its word glued:
// spaces mean a shift operator, <<= is an assignment, quoted needs no space.
void findRubyHeredocs(const std::string& line, const LangProfile* prof,
    std::vector<std::string>& words)
{
    Cursor s;
    s.d = line.c_str();
    s.n = line.size();
    s.p = 0;
    s.prof = prof;
    int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    while (s.p < s.n)
    {
        char ch = s.d[s.p];
        if (ch == '#') { break; }
        if (ch == '"' || ch == '\'' || ch == '`') { skipString(s); continue; }
        if (ch == '/') { stepUnit(s, dummy1, dummy2, dummy3); continue; }
        if (ch == '%')
        {
            // ponytail: %-literals are opaque (<< and ; inside are content)
            size_t j = s.p;
            if (skipRubyPercent(s.d, s.n, j)) { s.p = j; continue; }
        }
        if (ch == '<' && s.p + 1 < s.n && s.d[s.p + 1] == '<')
        {
            size_t j = s.p + 2;
            if (j < s.n && (s.d[j] == '-' || s.d[j] == '~')) { j++; }
            std::string word;
            if (j < s.n && (s.d[j] == '"' || s.d[j] == '\''))
            {
                char q = s.d[j];
                j++;
                size_t ws = j;
                // ponytail: quoted heredoc words take anything but the
                // quote (<<-"END;X" is a legal terminator name)
                while (j < s.n && s.d[j] != q) { j++; }
                if (j > ws && j < s.n && s.d[j] == q)
                {
                    word = line.substr(ws, j - ws);
                    j++;
                }
            }
            else if (j < s.n && (isAlpha(s.d[j]) || s.d[j] == '_'))
            {
                size_t ws = j;
                while (j < s.n && (isAlpha(s.d[j]) || s.d[j] == '_' || isDigit(s.d[j]))) { j++; }
                word = line.substr(ws, j - ws);
            }
            if (!word.empty()) { words.push_back(word); s.p = j; continue; }
        }
        s.p++;
    }
}

// Walks one physical line, carrying bracket depths; true when the logical
// line continues (unclosed brackets or a trailing backslash or operator).
// Strings, regexes and comments travel as opaque units, so nothing inside
// them can open, close or split anything.
bool rubyJoins(const std::string& line, const LangProfile* prof,
    int& paren, int& brack, int& brace)
{
    Cursor s;
    s.d = line.c_str();
    s.n = line.size();
    s.p = 0;
    s.prof = prof;
    int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    while (s.p < s.n)
    {
        char ch = s.d[s.p];
        if (ch == '#') { break; }
        if (ch == '"' || ch == '\'' || ch == '`') { skipString(s); continue; }
        if (ch == '/') { stepUnit(s, dummy1, dummy2, dummy3); continue; }
        if (ch == '%')
        {
            // ponytail: %-literals are opaque (brackets inside do not join)
            size_t j = s.p;
            if (skipRubyPercent(s.d, s.n, j)) { s.p = j; continue; }
        }
        if (ch == '(') { paren++; }
        else if (ch == ')') { paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { brace--; }
        s.p++;
    }
    if (paren > 0 || brack > 0 || brace > 0) { return true; }
    // ponytail: trailing operators (arg lists without parens, dotted
    // chains, ternaries) continue; anything else ends the logical line
    size_t e = line.size();
    while (e > 0 && (line[e - 1] == ' ' || line[e - 1] == '\t')) { e--; }
    if (e == 0) { return false; }
    char last = line[e - 1];
    // ponytail: a lone trailing & or | closes do-block params (do |x|),
    // only the doubled && and || continue an expression
    if (last == '&' || last == '|')
    {
        return e >= 2 && line[e - 2] == last;
    }
    // ponytail: a trailing ? continues a ternary ("cond ?") but ends a
    // predicate call ("tired?"); the space before it tells them apart
    if (last == '?')
    {
        return e < 2 || (!isIdent(line[e - 2]) && line[e - 2] != '?'
            && line[e - 2] != '!' && line[e - 2] != '$');
    }
    return last == '\\' || last == ',' || last == '.' || last == '+'
        || last == '-' || last == '*' || last == '/' || last == '%'
        || last == '^' || last == '<'
        || last == '>' || last == '=' || last == ':';
}

// Splits one heredoc-free logical line on top-level ';' (string, regex,
// comment and bracket aware). Comment-only lines stay whole.
void splitRubyLine(const std::string& text, const LangProfile* prof,
    std::vector<std::string>& out)
{
    std::string t = trimStr(text);
    if (t.empty()) { return; }
    if (t[0] == '#') { out.push_back(t); return; }
    Cursor s;
    s.d = text.c_str();
    s.n = text.size();
    s.p = 0;
    s.prof = prof;
    int paren = 0, brack = 0, brace = 0;
    int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    size_t start = 0;
    while (s.p < s.n)
    {
        char ch = s.d[s.p];
        if (ch == '#') { break; }
        if (ch == '"' || ch == '\'' || ch == '`') { skipString(s); continue; }
        if (ch == '/') { stepUnit(s, dummy1, dummy2, dummy3); continue; }
        if (ch == '%')
        {
            // ponytail: %-literals are opaque (; inside never splits)
            size_t j = s.p;
            if (skipRubyPercent(s.d, s.n, j)) { s.p = j; continue; }
        }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if (ch == ';' && paren == 0 && brack == 0 && brace == 0)
        {
            std::string part = trimStr(text.substr(start, s.p - start));
            if (!part.empty()) { out.push_back(part); }
            start = s.p + 1;
        }
        s.p++;
    }
    std::string tail = trimStr(text.substr(start));
    if (!tail.empty()) { out.push_back(tail); }
}

// True when a physical line ends inside a Ruby string (join the next
// line, or re-indentation would rewrite the string value). Single-state
// naive scan: only open/closed parity matters, so #{...} needs no
// structure (its quotes still toggle parity correctly). Regexes,
// %-literals and comments travel opaquely or end the scan.
bool rubyStringOpen(const std::string& line, const LangProfile* prof)
{
    Cursor s;
    s.d = line.c_str();
    s.n = line.size();
    s.p = 0;
    s.prof = prof;
    int dummy1 = 0, dummy2 = 0, dummy3 = 0;
    char quote = 0;
    while (s.p < s.n)
    {
        char ch = s.d[s.p];
        if (quote)
        {
            if (ch == '\\' && s.p + 1 < s.n) { s.p += 2; continue; }
            if (ch == quote) { quote = 0; }
            s.p++;
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; s.p++; continue; }
        if (ch == '#') { return false; }
        if (ch == '/') { stepUnit(s, dummy1, dummy2, dummy3); continue; }
        if (ch == '%')
        {
            size_t j = s.p;
            if (skipRubyPercent(s.d, s.n, j)) { s.p = j; continue; }
        }
        s.p++;
    }
    return quote != 0;
}

// Reads one logical Ruby line into segments: joins backslash, bracket and
// operator continuations, slurps heredoc bodies verbatim, keeps =begin
// blocks and __END__ tails whole. False at EOF (nothing appended).
bool readRubyLine(Cursor& c, std::vector<std::string>& segs)
{
    // skip blank lines entirely (they carry no structure)
    for (;;)
    {
        if (c.p >= c.n) { return false; }
        size_t ls = c.p;
        while (c.p < c.n && (c.d[c.p] == ' ' || c.d[c.p] == '\t')) { c.p++; }
        if (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '\r') { break; }
        while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
        if (c.p < c.n) { c.p++; }
        (void)ls;
    }
    // =begin ... =end block comment: one verbatim unit
    if (c.p + 6 <= c.n && strncmp(c.d + c.p, "=begin", 6) == 0
        && (c.p + 6 >= c.n || c.d[c.p + 6] == '\n' || c.d[c.p + 6] == '\r'
            || c.d[c.p + 6] == ' ' || c.d[c.p + 6] == '\t'))
    {
        size_t start = c.p;
        while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
        if (c.p < c.n) { c.p++; }
        for (;;)
        {
            if (c.p >= c.n) { break; }
            size_t ls = c.p;
            while (c.p < c.n && c.d[c.p] != '\n') { c.p++; }
            std::string line(c.d + ls, c.p - ls);
            if (c.p < c.n) { c.p++; }
            if (trimStr(line) == "=end") { break; }
        }
        segs.push_back(trimStr(std::string(c.d + start, c.p - start)));
        return true;
    }
    // __END__: the marker plus the whole data tail stay verbatim
    {
        size_t ls = c.p;
        while (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '\r') { c.p++; }
        std::string line(c.d + ls, c.p - ls);
        if (trimStr(line) == "__END__")
        {
            if (c.p < c.n && c.d[c.p] == '\r')
            {
                c.p++;
                if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
            }
            else if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
            segs.push_back("__END__");
            if (c.p < c.n)
            {
                segs.push_back(trimStr(std::string(c.d + c.p, c.n - c.p)));
                c.p = c.n;
            }
            return true;
        }
        c.p = ls;
    }
    std::string acc;
    int paren = 0, brack = 0, brace = 0;
    bool heredoc = false;
    for (;;)
    {
        size_t ls = c.p;
        while (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '\r') { c.p++; }
        std::string line(c.d + ls, c.p - ls);
        if (c.p < c.n && c.d[c.p] == '\r')
        {
            c.p++;
            if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
        }
        else if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
        if (!acc.empty()) { acc += "\n"; }
        acc += line;
        // heredoc bodies ride along verbatim (brackets inside do not count)
        std::vector<std::string> marks;
        findRubyHeredocs(line, c.prof, marks);
        for (size_t mi = 0; mi < marks.size(); mi++)
        {
            heredoc = true;
            for (;;)
            {
                if (c.p >= c.n) { break; }
                size_t bs = c.p;
                while (c.p < c.n && c.d[c.p] != '\n' && c.d[c.p] != '\r') { c.p++; }
                std::string body(c.d + bs, c.p - bs);
                if (c.p < c.n && c.d[c.p] == '\r')
                {
                    c.p++;
                    if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
                }
                else if (c.p < c.n && c.d[c.p] == '\n') { c.p++; }
                acc += "\n";
                acc += body;
                if (trimStr(body) == marks[mi]) { break; }
            }
        }
        if (c.p < c.n && (rubyJoins(line, c.prof, paren, brack, brace)
            || rubyStringOpen(line, c.prof))) { continue; }
        break;
    }
    if (heredoc) { segs.push_back(trimStr(acc)); }
    else { splitRubyLine(acc, c.prof, segs); }
    return true;
}

// Splits "head then rest" / "head do rest" at the first top-level then/do
// word. Dotted and suffixed lookalikes (x.then, then?, do!) do not count.
void splitRubyThen(const std::string& header, std::string& head,
    std::string& rest)
{
    head = header;
    rest.clear();
    const char* d = header.c_str();
    size_t n = header.size();
    size_t p = 0;
    int paren = 0, brack = 0, brace = 0;
    char quote = 0;
    while (p < n)
    {
        char ch = d[p];
        if (quote)
        {
            if (ch == '\\') { p++; }
            else if (ch == quote) { quote = 0; }
            p++;
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; p++; continue; }
        if (ch == '#') { break; }
        if (ch == '%')
        {
            // ponytail: %-literals are opaque (then/do inside are content)
            size_t j = p;
            if (skipRubyPercent(d, n, j)) { p = j; continue; }
        }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if ((ch == '_' || isAlpha(ch)) && paren == 0 && brack == 0 && brace == 0)
        {
            size_t ws = p;
            while (p < n && (isIdent(d[p]) && d[p] != '$')) { p++; }
            std::string word(d + ws, p - ws);
            bool beforeOk = (ws == 0) || (!isIdent(d[ws - 1]) && d[ws - 1] != '.'
                && d[ws - 1] != ':' && d[ws - 1] != '@' && d[ws - 1] != '$');
            bool afterOk = (p >= n) || (!isIdent(d[p]) && d[p] != '?' && d[p] != '!');
            if (beforeOk && afterOk && (word == "then" || word == "do"))
            {
                head = trimStr(header.substr(0, ws));
                rest = trimStr(header.substr(p));
                return;
            }
            continue;
        }
        p++;
    }
}

// Splits a trailing top-level "end" word ("y end" -> head "y").
// Dotted (x.end) and suffixed words never count; strings and brackets
// stay opaque. False when the text does not end with a bare end.
bool splitTrailingEnd(const std::string& text, std::string& head)
{
    const char* d = text.c_str();
    size_t n = text.size();
    size_t p = 0;
    int paren = 0, brack = 0, brace = 0;
    char quote = 0;
    size_t found = std::string::npos;
    while (p < n)
    {
        char ch = d[p];
        if (quote)
        {
            if (ch == '\\') { p++; }
            else if (ch == quote) { quote = 0; }
            p++;
            continue;
        }
        if (ch == '"' || ch == '\'' || ch == '`') { quote = ch; p++; continue; }
        if (ch == '#') { break; }
        if (ch == '%')
        {
            // ponytail: %-literals are opaque (end inside is content)
            size_t j = p;
            if (skipRubyPercent(d, n, j)) { p = j; continue; }
        }
        if (ch == '(') { paren++; }
        else if (ch == ')') { if (paren > 0) paren--; }
        else if (ch == '[') { brack++; }
        else if (ch == ']') { if (brack > 0) brack--; }
        else if (ch == '{') { brace++; }
        else if (ch == '}') { if (brace > 0) brace--; }
        else if ((ch == '_' || isAlpha(ch)) && paren == 0 && brack == 0 && brace == 0)
        {
            size_t ws = p;
            while (p < n && (isIdent(d[p]) && d[p] != '$')) { p++; }
            std::string word(d + ws, p - ws);
            bool beforeOk = (ws == 0) || (!isIdent(d[ws - 1]) && d[ws - 1] != '.'
                && d[ws - 1] != ':' && d[ws - 1] != '@' && d[ws - 1] != '$');
            bool afterOk = (p >= n) || (!isIdent(d[p]) && d[p] != '?' && d[p] != '!');
            if (beforeOk && afterOk && word == "end")
            {
                // ponytail: only a trailing end can be a closer; anything
                // after it (but a comment) means hands off
                size_t q = p;
                while (q < n && (d[q] == ' ' || d[q] == '\t')) { q++; }
                if (q >= n || d[q] == '#') { found = ws; }
            }
            continue;
        }
        p++;
    }
    if (found == std::string::npos) { return false; }
    head = trimStr(text.substr(0, found));
    return true;
}

// Queues an inline suite rest: splits ;-separated statements and peels a
// trailing end closer ("if x then y end" -> y, end). Order preserved.
void insertInlineRest(std::vector<std::string>& segs, size_t pos,
    const std::string& rest, const LangProfile* prof)
{
    std::vector<std::string> parts;
    splitRubyLine(rest, prof, parts);
    for (size_t i = parts.size(); i > 0; i--)
    {
        std::string head;
        if (splitTrailingEnd(parts[i - 1], head))
        {
            segs.insert(segs.begin() + pos, "end");
            if (!head.empty()) { segs.insert(segs.begin() + pos, head); }
        }
        else if (!parts[i - 1].empty())
        {
            segs.insert(segs.begin() + pos, parts[i - 1]);
        }
    }
}

// True for trailing do-blocks ("call args do [|params|]"), which keep the
// opener verbatim and nest the body. Keyword openers never reach here:
// while/until/for strip their own do, the rest have dedicated branches.
bool isRubyDoOpener(const std::string& t, const std::string& w)
{
    if (w == "def" || w == "class" || w == "module" || w == "if"
        || w == "unless" || w == "case" || w == "when" || w == "in"
        || w == "begin" || w == "rescue" || w == "ensure" || w == "else"
        || w == "elsif" || w == "while" || w == "until" || w == "for"
        || w == "end" || w == "do")
    {
        return false;
    }
    // ponytail: "do |a, b|" tails; the bars hold no nesting here
    size_t e = t.size();
    while (e > 0 && (t[e - 1] == ' ' || t[e - 1] == '\t')) { e--; }
    if (e > 0 && t[e - 1] == '|')
    {
        size_t b = e - 1;
        while (b > 0 && t[b - 1] != '|' && t[b - 1] != '\n') { b--; }
        if (b == 0 || t[b - 1] == '\n') { return false; }
        e = b - 1;
        while (e > 0 && (t[e - 1] == ' ' || t[e - 1] == '\t')) { e--; }
    }
    if (e < 2 || e > t.size()) { return false; }
    if (t.compare(e - 2, 2, "do") != 0) { return false; }
    if (e - 2 > 0 && (isIdent(t[e - 3]) && t[e - 3] != '$')) { return false; }
    return true;
}

} // namespace

namespace {

// Statuses for the Ruby recursion: EOF unwinds everything, END consumed the
// level's closer, CONT keeps the level loop going (a rewound continuation
// waits in the shared segments for the holder to attach it).
enum { RB_EOF = 0, RB_END = 1, RB_CONT = 2 };

int parseRubyItems(Cursor& c, std::vector<ASTNode*>& out,
    std::vector<ASTNode*>& pending, std::vector<std::string>& segs,
    size_t& pos, bool topLevel)
{
    RubyHolders held;
    held.iff = NULL;
    held.tcb = NULL;
    held.sw = NULL;
    held.def = NULL;
    held.kind = 0;
    for (;;)
    {
        if (pos >= segs.size())
        {
            size_t before = segs.size();
            if (!readRubyLine(c, segs)) { return RB_EOF; }
            if (segs.size() == before) { continue; }
        }
        std::string seg = segs[pos++];
        std::string t = trimStr(seg);
        if (t.empty()) { continue; }
        if (t[0] == '#')
        {
            CommentNode* cm = ASTNodeFactory::createCommentNode();
            cm->m_text = t.size() > 1 ? trimStr(t.substr(1)) : std::string();
            cm->m_isBlock = false;
            pending.push_back(cm);
            continue;
        }
        // ponytail: no flush here; continuations inside parseRubySegment
        // attach buffered comments to the suites they close, and only fresh
        // lines (or a closing end) flush them as siblings (see below)
        // ponytail: continuations belong to the level holding the opener; a
        // body rewinds a holder-less one so it bubbles to the holder (maybe
        // levels up); only a holder-less top level keeps one verbatim
        {
            std::string w = firstWord(t);
            bool cont = (w == "elsif" || w == "else" || w == "when" || w == "in"
                || w == "rescue" || w == "ensure");
            if (cont)
            {
                bool attach = false;
                if (w == "elsif") { attach = held.iff != NULL && held.iff->m_elseBranch == NULL; }
                else if (w == "else") { attach = held.kind != 0; }
                else if (w == "when" || w == "in") { attach = held.sw != NULL; }
                else if (w == "rescue") { attach = (held.tcb != NULL && held.tcb->m_finallyBlock == NULL) || (held.kind == 4 && held.def != NULL && held.tcb == NULL); }
                else if (w == "ensure") { attach = held.tcb != NULL || (held.kind == 4 && held.def != NULL && held.tcb == NULL); }
                if (!attach)
                {
                    if (!topLevel) { pos--; return RB_CONT; }
                    out.push_back(mkRaw(t));
                    continue;
                }
            }
        }
        int st = parseRubySegment(c, out, t, pending, segs, pos, &held, topLevel);
        if (st == RB_END) { return RB_END; }
        if (st == RB_EOF) { return RB_EOF; }
    }
}

// Splits a trailing # comment off an opener header into pending
// (string- and %-aware). Callers parse the stripped code, keep raw
// fallbacks on the original line, and queue only on success (the suite
// loop-top then opens with the comment).
void queueHeaderComment(const std::string& hc, std::vector<ASTNode*>& pending)
{
    if (hc.empty()) { return; }
    CommentNode* cm = ASTNodeFactory::createCommentNode();
    cm->m_text = hc.size() > 1 ? trimStr(hc.substr(1)) : std::string();
    cm->m_isBlock = false;
    pending.push_back(cm);
}

// Parses one Ruby segment. Openers push their node, then consume the body
// through recursion; continuations mutate the pushed holder in place, so
// nothing is ever pushed twice. RB_END closes the level, RB_EOF unwinds all.
int parseRubySegment(Cursor& c, std::vector<ASTNode*>& out, const std::string& t,
    std::vector<ASTNode*>& pending, std::vector<std::string>& segs, size_t& pos,
    RubyHolders* held, bool topLevel)
{
    std::string w = firstWord(t);
    // end closes the level (a stray top-level end stays verbatim).
    // Buffered comments flush first: they sit inside the closing suite.
    if (w == "end")
    {
        for (size_t i = 0; i < pending.size(); i++) { out.push_back(pending[i]); }
        pending.clear();
        std::string rest = trimStr(t.substr(3));
        if (rest.empty())
        {
            if (topLevel) { out.push_back(mkRaw(t)); return RB_CONT; }
            return RB_END;
        }
        if (rest[0] == '#')
        {
            CommentNode* cm = ASTNodeFactory::createCommentNode();
            cm->m_text = rest.size() > 1 ? trimStr(rest.substr(1)) : std::string();
            cm->m_isBlock = false;
            pending.push_back(cm);
            if (topLevel) { out.push_back(mkRaw("end")); return RB_CONT; }
            return RB_END;
        }
        // ponytail: "end ..." is never valid; keep it verbatim
        out.push_back(mkRaw(t));
        return RB_CONT;
    }
    // elsif chains onto a held if/unless
    if (w == "elsif" && held != NULL && held->iff != NULL
        && held->iff->m_elseBranch == NULL)
    {
        IfStmtNode* target = held->iff;
        std::string head;
        std::string rest;
        splitRubyThen(t, head, rest);
        std::string cond = trimStr(head.substr(5));
        if (cond.empty())
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        IfStmtNode* elif = ASTNodeFactory::createIfStmtNode();
        elif->m_keyword = "elsif";
        elif->m_condition = mkValue(cond, true);
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        elif->m_thenBranch = suite;
        target->m_elseBranch = elif;
        held->iff = elif;
        held->kind = 1;
        if (st == RB_EOF) { return RB_EOF; }
        if (st == RB_END) { held->iff = NULL; held->kind = 0; }
        return RB_CONT;
    }
    // else binds to the innermost open holder (if, case, or begin)
    if (w == "else" && held != NULL && held->kind != 0)
    {
        std::string code;
        std::string hc = stripTrailingComment(t, code);
        std::string head;
        std::string rest;
        splitRubyThen(code, head, rest);
        // ponytail: "else stmt" carries its suite inline with no then;
        // a trailing comment alone keeps the whole line verbatim
        if (trimStr(head) != "else")
        {
            std::string after = (code.size() > 4) ? trimStr(code.substr(4)) : std::string();
            if (after.empty() || after[0] == '#')
            {
                out.push_back(mkRaw(t));
                return RB_CONT;
            }
            head = "else";
            // ponytail: the stripped comment rides back onto the suite
            // rest, which keeps it verbatim as one statement
            rest = hc.empty() ? after : after + " " + hc;
            hc.clear();
        }
        if (held->kind == 1 && held->iff != NULL
            && held->iff->m_elseBranch == NULL)
        {
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
            queueHeaderComment(hc, pending);
            int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
            held->iff->m_elseBranch = suite;
            held->iff = NULL;
            held->kind = 0;
            if (st == RB_EOF) { return RB_EOF; }
            return RB_CONT;
        }
        if (held->kind == 2 && held->sw != NULL)
        {
            SwitchCaseNode* arm = ASTNodeFactory::createSwitchCaseNode();
            arm->m_pattern = mkRaw("else");
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
            queueHeaderComment(hc, pending);
            int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
            arm->m_body = suite;
            held->sw->m_cases.push_back(arm);
            if (st == RB_EOF) { return RB_EOF; }
            if (st == RB_END) { held->sw = NULL; held->kind = 0; }
            return RB_CONT;
        }
        if (held->kind == 3 && held->tcb != NULL
            && held->tcb->m_elseBlock == NULL
            && held->tcb->m_finallyBlock == NULL)
        {
            BlockNode* suite = ASTNodeFactory::createBlockNode();
            if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
            queueHeaderComment(hc, pending);
            int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
            held->tcb->m_elseBlock = suite;
            if (st == RB_EOF) { return RB_EOF; }
            if (st == RB_END) { held->tcb = NULL; held->kind = 0; }
            return RB_CONT;
        }
        // ponytail: doubled or misplaced else cannot attach; the holder is
        // already pushed, so keep just the line verbatim
        out.push_back(mkRaw(t));
        return RB_CONT;
    }
    // when/in arms belong to a held case
    if ((w == "when" || w == "in") && held != NULL && held->sw != NULL)
    {
        std::string head;
        std::string rest;
        splitRubyThen(t, head, rest);
        std::string pat = trimStr(head.substr(w.size()));
        if (pat.empty())
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        SwitchCaseNode* arm = ASTNodeFactory::createSwitchCaseNode();
        arm->m_pattern = mkRaw(head);
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        arm->m_body = suite;
        held->sw->m_cases.push_back(arm);
        if (st == RB_EOF) { return RB_EOF; }
        if (st == RB_END) { held->sw = NULL; held->kind = 0; }
        return RB_CONT;
    }
    // method-level rescue: no begin, so the chain nests as the def's last
    // suite statement (a body-less TryCatch prints no begin header or end)
    if (w == "rescue" && held != NULL && held->kind == 4 && held->def != NULL
        && held->tcb == NULL)
    {
        FunctionDeclNode* fn = held->def;
        BlockNode* db = NULL;
        if (fn->m_body != NULL && fn->m_body->isBlock())
        {
            db = static_cast<BlockNode*>(fn->m_body);
        }
        std::string head;
        std::string rest;
        splitRubyThen(t, head, rest);
        std::string clause = trimStr(head.substr(6));
        if (db == NULL)
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        for (size_t i = 0; i < pending.size(); i++) { db->m_statements.push_back(pending[i]); }
        pending.clear();
        TryCatchStmtNode* chain = ASTNodeFactory::createTryCatchStmtNode();
        if (!clause.empty()) { chain->m_catchClause = mkIdent(clause); }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        chain->m_catchBody = suite;
        db->m_statements.push_back(chain);
        held->def = NULL;
        held->tcb = chain;
        held->kind = 3;
        if (st == RB_EOF) { return RB_EOF; }
        if (st == RB_END) { held->tcb = NULL; held->kind = 0; }
        return RB_CONT;
    }
    // rescue extends a held begin
    if (w == "rescue" && held != NULL && held->tcb != NULL
        && held->tcb->m_finallyBlock == NULL)
    {
        TryCatchStmtNode* target = held->tcb;
        std::string head;
        std::string rest;
        splitRubyThen(t, head, rest);
        std::string clause = trimStr(head.substr(6));
        // ponytail: buffered comments close the previous suite, like Python
        BlockNode* lastSuite = NULL;
        if (!target->m_extraCatches.empty() && target->m_extraCatches.back() != NULL)
        {
            TryCatchStmtNode* last = static_cast<TryCatchStmtNode*>(
                target->m_extraCatches.back());
            if (last->m_catchBody != NULL && last->m_catchBody->isBlock())
            {
                lastSuite = static_cast<BlockNode*>(last->m_catchBody);
            }
        }
        if (lastSuite == NULL && target->m_catchBody != NULL
            && target->m_catchBody->isBlock())
        {
            lastSuite = static_cast<BlockNode*>(target->m_catchBody);
        }
        if (lastSuite == NULL && target->m_body != NULL
            && target->m_body->isBlock())
        {
            lastSuite = static_cast<BlockNode*>(target->m_body);
        }
        for (size_t i = 0; i < pending.size() && lastSuite != NULL; i++)
        {
            lastSuite->m_statements.push_back(pending[i]);
        }
        pending.clear();
            if (target->m_catchClause == NULL && target->m_catchBody == NULL
                && target->m_extraCatches.empty())
            {
                if (!clause.empty()) { target->m_catchClause = mkIdent(clause); }
                BlockNode* suite = ASTNodeFactory::createBlockNode();
                if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
                int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
                target->m_catchBody = suite;
                if (st == RB_EOF) { return RB_EOF; }
                if (st == RB_END && held != NULL) { held->tcb = NULL; held->kind = 0; }
                return RB_CONT;
            }
        TryCatchStmtNode* extra = ASTNodeFactory::createTryCatchStmtNode();
        if (!clause.empty()) { extra->m_catchClause = mkIdent(clause); }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        extra->m_catchBody = suite;
        target->m_extraCatches.push_back(extra);
        if (st == RB_EOF) { return RB_EOF; }
        // ponytail: an end closed the whole begin; drop the stale holder
        if (st == RB_END && held != NULL) { held->tcb = NULL; held->kind = 0; }
        return RB_CONT;
    }
    // method-level ensure (def/ensure with no rescue): same nest, only a
    // finally block; a non-bare ensure line stays verbatim
    if (w == "ensure" && held != NULL && held->kind == 4 && held->def != NULL
        && held->tcb == NULL)
    {
        FunctionDeclNode* fn = held->def;
        BlockNode* db = NULL;
        if (fn->m_body != NULL && fn->m_body->isBlock())
        {
            db = static_cast<BlockNode*>(fn->m_body);
        }
        std::string code;
        std::string hc = stripTrailingComment(t, code);
        std::string head;
        std::string rest;
        splitRubyThen(code, head, rest);
        if (db == NULL || trimStr(head) != "ensure")
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        for (size_t i = 0; i < pending.size(); i++) { db->m_statements.push_back(pending[i]); }
        pending.clear();
        TryCatchStmtNode* chain = ASTNodeFactory::createTryCatchStmtNode();
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        queueHeaderComment(hc, pending);
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        chain->m_finallyBlock = suite;
        db->m_statements.push_back(chain);
        held->def = NULL;
        held->tcb = chain;
        held->kind = 3;
        if (st == RB_EOF) { return RB_EOF; }
        if (st == RB_END) { held->tcb = NULL; held->kind = 0; }
        return RB_CONT;
    }
    // ensure closes a held begin (nothing attaches past it)
    if (w == "ensure" && held != NULL && held->tcb != NULL
        && held->tcb->m_finallyBlock == NULL)
    {
        std::string code;
        std::string hc = stripTrailingComment(t, code);
        std::string head;
        std::string rest;
        splitRubyThen(code, head, rest);
        if (trimStr(head) != "ensure")
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        queueHeaderComment(hc, pending);
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        held->tcb->m_finallyBlock = suite;
        held->tcb = NULL;
        held->kind = 0;
        if (st == RB_EOF) { return RB_EOF; }
        return RB_CONT;
    }
    // a fresh line closes open chains (every held node is already pushed).
    // Buffered comments flush here (not earlier), so continuations above
    // could still claim them for the suites they close.
    if (held != NULL)
    {
        held->iff = NULL;
        held->tcb = NULL;
        held->sw = NULL;
        held->def = NULL;
        held->kind = 0;
    }
    for (size_t i = 0; i < pending.size(); i++) { out.push_back(pending[i]); }
    pending.clear();
    if (w == "def")
    {
        std::string code;
        std::string hc = stripTrailingComment(t, code);
        std::string rest = trimStr(code.substr(3));
        size_t lp = findTopLevel(rest, '(');
        std::string name;
        std::string params;
        std::string trailer;
        bool badDef = true;
        if (lp != std::string::npos)
        {
            std::string pre = trimStr(rest.substr(0, lp));
            if (!pre.empty() && pre.find_first_of(" \t\n(){};,") == std::string::npos)
            {
                Cursor sub = c;
                sub.d = rest.c_str();
                sub.n = rest.size();
                sub.p = lp;
                if (readParenGroup(sub, params))
                {
                    name = pre;
                    trailer = trimStr(rest.substr(sub.p));
                    badDef = false;
                }
            }
        }
        else
        {
            // ponytail: paren-less defs split on a top-level = (endless
            // defs); ==, =>, =~ never count, findAssign already skips them
            size_t eq = findAssign(rest);
            if (eq != std::string::npos && rest[eq] == '='
                && !(eq + 1 < rest.size() && rest[eq + 1] == '~'))
            {
                name = trimStr(rest.substr(0, eq));
                trailer = trimStr(rest.substr(eq));
                badDef = name.empty()
                    || name.find_first_of(" \t\n(){};,") != std::string::npos;
            }
            else if (eq == std::string::npos)
            {
                name = rest;
                badDef = name.empty()
                    || name.find_first_of(" \t\n(){};,") != std::string::npos;
            }
        }
        if (badDef)
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        FunctionDeclNode* node = ASTNodeFactory::createFunctionDeclNode();
        node->m_name = name;
        if (!trimStr(params).empty()) { node->m_params.push_back(mkIdent(trimStr(params))); }
        else if (lp != std::string::npos) { node->m_hasParens = true; }
        // ponytail: "def f = body" is endless: one statement, no end coming
        bool endless = !trailer.empty() && trailer[0] == '='
            && (trailer.size() < 2 || (trailer[1] != '=' && trailer[1] != '>'));
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (endless)
        {
            std::string bodyText = trimStr(trailer.substr(1));
            // ponytail: endless bodies are one statement ("def f = x; y"
            // ends the def at ;); anything else stays verbatim, and the
            // lone statement dispatches alone so siblings cannot leak in
            std::vector<std::string> parts;
            splitRubyLine(bodyText, c.prof, parts);
            if (bodyText.empty() || parts.size() != 1)
            {
                out.push_back(mkRaw(t));
                return RB_CONT;
            }
            node->m_body = suite;
            // ponytail: dispatch the lone statement directly (no level
            // loop, so no read-ahead); anything reaching past its own
            // line rewinds and the opener stays verbatim instead.
            // op starts past parts[0] (already taken, dispatched below).
            size_t pMark = c.p;
            size_t op = 1;
            RubyHolders bh;
            bh.iff = NULL;
            bh.tcb = NULL;
            bh.sw = NULL;
            bh.def = NULL;
            bh.kind = 0;
            size_t pendMark = pending.size();
            queueHeaderComment(hc, pending);
            parseRubySegment(c, suite->m_statements, parts[0], pending,
                parts, op, &bh, false);
            if (c.p != pMark || suite->m_statements.size() != 1
                || op != parts.size())
            {
                // ponytail: the lone statement looked past its own line,
                // left pieces behind, or made several; rewind and keep the
                // opener verbatim instead, siblings re-read untouched
                // (the queued header comment with them: raw keeps it glued)
                c.p = pMark;
                pending.resize(pendMark);
                out.push_back(mkRaw(t));
                return RB_CONT;
            }
            node->m_endless = true;
            out.push_back(node);
            return RB_CONT;
        }
        if (!trailer.empty())
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        node->m_body = suite;
        out.push_back(node);
        queueHeaderComment(hc, pending);
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        if (st == RB_EOF) { return RB_EOF; }
        if (st == RB_END)
        {
            // ponytail: a clean end needs no holder; anything else rewound
            // (rescue/ensure attach below, strays flush it away)
            if (held != NULL) { held->def = NULL; held->kind = 0; }
            return RB_CONT;
        }
        // ponytail: hold the def for a method-level rescue/ensure (no
        // begin); any other next line releases it through the flush
        if (held != NULL) { held->def = node; held->kind = 4; }
        return RB_CONT;
    }
    if (w == "class")
    {
        std::string code;
        std::string hc = stripTrailingComment(t, code);
        std::string rest = trimStr(code.substr(5));
        if (rest.empty())
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        ClassDeclNode* node = ASTNodeFactory::createClassDeclNode();
        if (rest.size() >= 2 && rest[0] == '<' && rest[1] == '<')
        {
            // ponytail: singleton classes (class << self) keep it all
            node->m_name = rest;
        }
        else
        {
            size_t lt = findTopLevel(rest, '<');
            if (lt != std::string::npos && (lt + 1 >= rest.size() || rest[lt + 1] != '<')
                && (lt == 0 || rest[lt - 1] != '<'))
            {
                node->m_name = trimStr(rest.substr(0, lt));
                std::string sup = trimStr(rest.substr(lt + 1));
                if (!sup.empty()) { node->m_superClass = mkIdent(sup); }
            }
            else
            {
                node->m_name = rest;
            }
            if (node->m_name.empty()
                || node->m_name.find_first_of(" \t\n") != std::string::npos)
            {
                out.push_back(mkRaw(t));
                return RB_CONT;
            }
        }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        node->m_body = suite;
        out.push_back(node);
        queueHeaderComment(hc, pending);
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        if (st == RB_EOF) { return RB_EOF; }
        return RB_CONT;
    }
    if (w == "module")
    {
        std::string code;
        std::string hc = stripTrailingComment(t, code);
        std::string rest = trimStr(code.substr(6));
        if (rest.empty() || rest.find_first_of(" \t\n") != std::string::npos)
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        NamespaceNode* node = ASTNodeFactory::createNamespaceNode();
        node->m_name = rest;
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        node->m_body = suite;
        out.push_back(node);
        queueHeaderComment(hc, pending);
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        if (st == RB_EOF) { return RB_EOF; }
        return RB_CONT;
    }
    if (w == "if" || w == "unless")
    {
        std::string head;
        std::string rest;
        splitRubyThen(t, head, rest);
        std::string cond = trimStr(head.substr(w.size()));
        if (cond.empty())
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        IfStmtNode* node = ASTNodeFactory::createIfStmtNode();
        node->m_keyword = w;
        node->m_condition = mkValue(cond, true);
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        node->m_thenBranch = suite;
        out.push_back(node);
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        if (st == RB_EOF) { return RB_EOF; }
        if (st == RB_END)
        {
            if (held != NULL) { held->iff = NULL; held->kind = 0; }
            return RB_CONT;
        }
        if (held != NULL) { held->iff = node; held->kind = 1; }
        return RB_CONT;
    }
    if (w == "case")
    {
        std::string head;
        std::string rest;
        splitRubyThen(t, head, rest);
        std::string disc = trimStr(head.substr(4));
        SwitchStmtNode* node = ASTNodeFactory::createSwitchStmtNode();
        if (!disc.empty()) { node->m_discriminant = mkValue(disc, true); }
        out.push_back(node);
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        // ponytail: arms arrive as level segments; stray prelude lines land
        // after the node (they run unconditionally, order kept losslessly)
        std::vector<ASTNode*> prelude;
        int st = parseRubyItems(c, prelude, pending, segs, pos, false);
        for (size_t i = 0; i < prelude.size(); i++) { out.push_back(prelude[i]); }
        if (st == RB_EOF) { return RB_EOF; }
        // ponytail: RB_END consumed our end; RB_CONT rewound an arm for us
        if (st == RB_CONT && held != NULL) { held->sw = node; held->kind = 2; }
        return RB_CONT;
    }
    if (w == "begin")
    {
        std::string code;
        std::string hc = stripTrailingComment(t, code);
        std::string head;
        std::string rest;
        splitRubyThen(code, head, rest);
        if (trimStr(head) != "begin")
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        TryCatchStmtNode* node = ASTNodeFactory::createTryCatchStmtNode();
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        node->m_body = suite;
        out.push_back(node);
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        queueHeaderComment(hc, pending);
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        if (st == RB_EOF) { return RB_EOF; }
        if (st == RB_END)
        {
            if (held != NULL) { held->tcb = NULL; held->kind = 0; }
            return RB_CONT;
        }
        if (held != NULL) { held->tcb = node; held->kind = 3; }
        return RB_CONT;
    }
    if (w == "while" || w == "until" || w == "for")
    {
        std::string head;
        std::string rest;
        splitRubyThen(t, head, rest);
        std::string cond = trimStr(head.substr(w.size()));
        if (cond.empty())
        {
            out.push_back(mkRaw(t));
            return RB_CONT;
        }
        BlockNode* suite = ASTNodeFactory::createBlockNode();
        if (!rest.empty()) { insertInlineRest(segs, pos, rest, c.prof); }
        int st = parseRubyItems(c, suite->m_statements, pending, segs, pos, false);
        if (w == "for")
        {
            ForStmtNode* node = ASTNodeFactory::createForStmtNode();
            node->m_keyword = "for";
            node->m_classic = false;
            node->m_condition = mkValue(cond, true);
            node->m_body = suite;
            out.push_back(node);
        }
        else
        {
            WhileStmtNode* node = ASTNodeFactory::createWhileStmtNode();
            node->m_keyword = w;
            node->m_condition = mkValue(cond, true);
            node->m_body = suite;
            out.push_back(node);
        }
        if (st == RB_EOF) { return RB_EOF; }
        return RB_CONT;
    }
    // ponytail: the do check runs on comment-stripped code so a trailing
    // comment cannot hide the opener; the comment opens the body instead
    std::string doCode;
    std::string doHc = stripTrailingComment(t, doCode);
    if (isRubyDoOpener(doCode, firstWord(doCode)))
    {
        // ponytail: one statement for the whole construct: the verbatim
        // opener owns the slot line, the body nests with its own end
        BlockNode* block = ASTNodeFactory::createBlockNode();
        block->m_kind = "do";
        block->m_statements.push_back(mkRaw(doCode));
        queueHeaderComment(doHc, pending);
        int st = parseRubyItems(c, block->m_statements, pending, segs, pos, false);
        out.push_back(block);
        if (st == RB_EOF) { return RB_EOF; }
        return RB_CONT;
    }
    if (w == "require")
    {
        std::string rest = trimStr(t.substr(7));
        if (!rest.empty() && rest[0] == '(' && rest[rest.size() - 1] == ')')
        {
            rest = trimStr(rest.substr(1, rest.size() - 2));
        }
        bool quoted = rest.size() >= 2
            && ((rest[0] == '"' && rest[rest.size() - 1] == '"')
                || (rest[0] == '\'' && rest[rest.size() - 1] == '\''));
        if (quoted && findTopLevel(rest, ',') == std::string::npos)
        {
            ImportDeclNode* node = ASTNodeFactory::createImportDeclNode();
            node->m_source = rest.substr(1, rest.size() - 2);
            out.push_back(node);
            return RB_CONT;
        }
        // ponytail: dynamic/multi requires keep their exact text
        out.push_back(mkRaw(t));
        return RB_CONT;
    }
    // ponytail: return/raise/assignments/calls/modifiers round-trip through
    // the shared statement parser (trailing comments ride along verbatim).
    // Ruby throw is not raise (different control flow), so it stays raw.
    if (w == "throw")
    {
        out.push_back(mkRaw(t));
        return RB_CONT;
    }
    {
        ASTNode* stmt = parseStatementText(c.prof, t);
        if (stmt != NULL)
        {
            out.push_back(stmt);
            return RB_CONT;
        }
        out.push_back(mkRaw(t));
        return RB_CONT;
    }
}

} // namespace

ProgramNode* Parser::parseNested(const char* source, size_t len, const std::string& language)
{
    if (source == NULL || len == 0)
    {
        return NULL;
    }
    // ponytail: no pool reset here; the caller's AST is still alive in it
    if (language == "html" || language == "vue")
    {
        return parseHtmlDoc(source, len);
    }
    if (language == "json")
    {
        return parseJsonDoc(source, len);
    }
    std::string normalized;
    if (normalizeNewlines(source, len, normalized)) { source = normalized.c_str(); len = normalized.size(); }
    Cursor c;
    c.d = source;
    c.n = len;
    c.p = 0;
    c.prof = profileFor(language);
    ProgramNode* prog = ASTNodeFactory::createProgramNode();
    if (language == "python")
    {
        std::vector<ASTNode*> pending;
        parsePythonItems(c, prog->m_statements, -1, pending);
        for (size_t i = 0; i < pending.size(); i++) { prog->m_statements.push_back(pending[i]); }
        return prog;
    }
    if (language == "ruby")
    {
        std::vector<ASTNode*> pending;
        std::vector<std::string> segs;
        size_t pos = 0;
        parseRubyItems(c, prog->m_statements, pending, segs, pos, true);
        for (size_t i = 0; i < pending.size(); i++) { prog->m_statements.push_back(pending[i]); }
        return prog;
    }
    parseItems(c, prog->m_statements);
    return prog;
}

bool Parser::isValid() const
{
    return m_isValid;
}

