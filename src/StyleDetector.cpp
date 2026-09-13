#include "fmtpp/StyleDetector.hpp"
#include <toml++/toml.hpp>
#include <fstream>
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
#include <vector>
#include <algorithm>

// Easter egg: the style detective is on the case - analyzing your code, one pattern at a time

namespace {
bool detIsIdent(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
        || (ch >= '0' && ch <= '9') || ch == '_' || ch == '$';
}

bool detIsControlKeyword(const std::string& w)
{
    return w == "if" || w == "for" || w == "while" || w == "switch"
        || w == "catch" || w == "return" || w == "typeof" || w == "new"
        || w == "delete" || w == "void" || w == "in" || w == "of"
        || w == "await" || w == "yield" || w == "instanceof";
}

bool detIsHSpace(char ch)
{
    return ch == ' ' || ch == '\t';
}

bool startsWithWord(const char* d, size_t pos, size_t end, const char* w)
{
    size_t k = 0;
    while (w[k] != '\0')
    {
        if (pos + k >= end || d[pos + k] != w[k]) { return false; }
        k++;
    }
    return pos + k >= end || !detIsIdent(d[pos + k]);
}

// True when the word occurs anywhere in range with ident boundaries.
bool containsWord(const char* d, size_t start, size_t end, const char* w)
{
    size_t k = 0;
    while (w[k] != '\0') { k++; }
    if (k == 0) { return false; }
    for (size_t i = start; i + k <= end; i++)
    {
        size_t j = 0;
        while (j < k && d[i + j] == w[j]) { j++; }
        if (j == k)
        {
            bool leftOk = (i == start) || !detIsIdent(d[i - 1]);
            bool rightOk = (i + k >= end) || !detIsIdent(d[i + k]);
            if (leftOk && rightOk) { return true; }
        }
    }
    return false;
}

// True for `else` + spaces + `if` (both word-bounded).
bool containsElseIf(const char* d, size_t start, size_t end)
{
    for (size_t i = start; i + 4 <= end; i++)
    {
        if (d[i] == 'e' && startsWithWord(d, i, end, "else"))
        {
            size_t r = i + 4;
            while (r < end && detIsHSpace(d[r])) { r++; }
            if (startsWithWord(d, r, end, "if")) { return true; }
        }
    }
    return false;
}
}

void StyleDetector::scanCodeLine(const char* d, size_t start, size_t end)
{
    for (size_t i = start; i < end; i++)
    {
        char ch = d[i];
        if (ch == '"' || ch == '\'')
        {
            i++;
            while (i < end)
            {
                if (d[i] == '\\' && i + 1 < end) { i += 2; continue; }
                if (d[i] == ch) { break; }
                i++;
            }
            continue;
        }
        bool prevSpace = (i > start && detIsHSpace(d[i - 1]));
        bool nextSpace = (i + 1 < end && detIsHSpace(d[i + 1]));
        char next = (i + 1 < end) ? d[i + 1] : '\0';
        char prev = (i > start) ? d[i - 1] : '\0';
        if (ch == '{')
        {
            if (next == '}') { continue; }
            else if (nextSpace) { m_curlySpacedCount++; }
            else if (next != '\0') { m_curlyTightCount++; }
            else
            {
                // ponytail: `{` at end of line is brace style, not brace
                // spacing; vote only on `){` vs `) {` tightness instead
                size_t k = i;
                while (k > start && detIsHSpace(d[k - 1])) { k--; }
                if (k > start && d[k - 1] == ')')
                {
                    if (k == i) { m_braceTightCount++; }
                    else { m_braceSpacedCount++; }
                }
            }
        }
        else if (ch == '}')
        {
            if (prev == '{' || i == start) { continue; }
            else if (prevSpace) { m_curlySpacedCount++; }
            else { m_curlyTightCount++; }
        }
        else if (ch == '[')
        {
            if (next == ']') { continue; }
            else if (nextSpace) { m_squareSpacedCount++; }
            else if (next != '\0') { m_squareTightCount++; }
        }
        else if (ch == ']')
        {
            if (prev == '[' || i == start) { continue; }
            else if (prevSpace) { m_squareSpacedCount++; }
            else { m_squareTightCount++; }
        }
        else if (ch == '(')
        {
            if (next == ')') { continue; }
            else if (nextSpace) { m_parenSpacedCount++; }
            else if (next != '\0') { m_parenTightCount++; }
            {
                // ponytail: scan back past spaces so `f (` attributes to
                // `f` (spaced) instead of vanishing into no word at all
                size_t wend = i;
                while (wend > start && detIsHSpace(d[wend - 1])) { wend--; }
                bool spaced = (wend < i);
                std::string word;
                if (wend > start && detIsIdent(d[wend - 1]))
                {
                    size_t ws = wend - 1;
                    while (ws > start && detIsIdent(d[ws - 1])) { ws--; }
                    word.assign(d + ws, (wend - 1) - ws + 1);
                }
                if (!word.empty() && detIsControlKeyword(word))
                {
                    if (spaced) { m_keywordSpacedCount++; }
                    else { m_keywordTightCount++; }
                    if (word == "if" || word == "for" || word == "while"
                        || word == "switch" || word == "catch")
                    {
                        if (next != ')' && next != '\0')
                        {
                            if (nextSpace) { m_condParenSpacedCount++; }
                            else { m_condParenTightCount++; }
                        }
                    }
                }
                else
                {
                    if (spaced) { m_funcSpaceCount++; }
                    else { m_funcTightCount++; }
                }
            }
        }
        else if (ch == ')')
        {
            if (prev == '(' || i == start) { continue; }
            else if (prevSpace) { m_parenSpacedCount++; }
            else { m_parenTightCount++; }
            size_t j = i + 1;
            while (j < end && detIsHSpace(d[j])) { j++; }
            if (j < end && d[j] == '{')
            {
                if (j > i + 1) { m_braceSpacedCount++; }
                else { m_braceTightCount++; }
            }
        }
        else if (ch == ',')
        {
            if (next == '\0') { continue; }
            else if (nextSpace) { m_commaSpacedCount++; }
            else { m_commaTightCount++; }
        }
        else if (ch == '=' && next == '>')
        {
            bool leftSpace = prevSpace;
            bool rightSpace = (i + 2 < end && detIsHSpace(d[i + 2]));
            if (leftSpace && rightSpace) { m_arrowSpacedCount++; }
            else if (!leftSpace && !rightSpace && i > start) { m_arrowTightCount++; }
            size_t k = i;
            while (k > start && detIsHSpace(d[k - 1])) { k--; }
            if (k > start && d[k - 1] == ')') { m_arrowParenCount++; }
            else if (k > start && detIsIdent(d[k - 1])) { m_bareArrowParamCount++; }
            i++;
        }
        else if (ch == '$' && next == '{')
        {
            if (i + 2 < end && detIsHSpace(d[i + 2])) { m_templateSpacedCount++; }
            else if (i + 2 < end) { m_templateTightCount++; }
            i++;
        }
        else if (ch == '=')
        {
            if (next == '=' || next == '>' || prev == '=' || prev == '<'
                || prev == '!' || prev == ':')
            {
                continue;
            }
            bool leftSpace = prevSpace;
            bool rightSpace = nextSpace;
            if (leftSpace && rightSpace) { m_assignSpacedCount++; }
            else if (!leftSpace && !rightSpace && i > start && next != '\0') { m_assignTightCount++; }
        }
        else if (ch == ':')
        {
            if (next == ':' || prev == ':') { continue; }
            if (next == '\0') { continue; }
            else if (nextSpace) { m_colonSpacedCount++; }
            else { m_colonTightCount++; }
        }
    }
}

StyleDetector::StyleDetector()
    : m_semicolonCount(0)
    , m_noSemicolonCount(0)
    , m_singleQuoteCount(0)
    , m_doubleQuoteCount(0)
    , m_sameLineBraceCount(0)
    , m_nextLineBraceCount(0)
    , m_tabCount(0)
    , m_spaceIndentCount(0)
    , m_curlyTightCount(0)
    , m_curlySpacedCount(0)
    , m_squareTightCount(0)
    , m_squareSpacedCount(0)
    , m_commaTightCount(0)
    , m_commaSpacedCount(0)
    , m_parenTightCount(0)
    , m_parenSpacedCount(0)
    , m_condParenTightCount(0)
    , m_condParenSpacedCount(0)
    , m_arrowParenCount(0)
    , m_bareArrowParamCount(0)
    , m_arrowTightCount(0)
    , m_arrowSpacedCount(0)
    , m_templateTightCount(0)
    , m_templateSpacedCount(0)
    , m_trailingCommaLineCount(0)
    , m_noTrailingCommaLineCount(0)
    , m_multilineTagCount(0)
    , m_singleLineTagCount(0)
    , m_maxBlankRun(0)
    , m_blankAfterImportCount(0)
    , m_noBlankAfterImportCount(0)
    , m_assignSpacedCount(0)
    , m_assignTightCount(0)
    , m_colonSpacedCount(0)
    , m_colonTightCount(0)
    , m_funcSpaceCount(0)
    , m_funcTightCount(0)
    , m_braceTightCount(0)
    , m_braceSpacedCount(0)
    , m_keywordTightCount(0)
    , m_keywordSpacedCount(0)
    , m_elseJoinedCount(0)
    , m_elseSplitCount(0)
    , m_catchJoinedCount(0)
    , m_catchSplitCount(0)
    , m_finallyJoinedCount(0)
    , m_finallySplitCount(0)
    , m_elseifCount(0)
    , m_elseIfCount(0)
    , m_blankBeforeReturnCount(0)
    , m_noBlankBeforeReturnCount(0)
    , m_leadingBlankCount(0)
    , m_finalNewlineCount(0)
    , m_noFinalNewlineCount(0)
    , m_bomCount(0)
    , m_inlineEmptyCount(0)
    , m_splitEmptyCount(0)
    , m_detectedTabWidth(2)
    , m_maxLineWidth(0)
    , m_nodeCount(0)
{
}

StyleDetector::~StyleDetector()
{
    // Easter egg: case closed - the detective has found all your style secrets
}

Config StyleDetector::detect(ASTNode* root)
{
    if (root != NULL)
    {
        root->accept(*this);
    }

    // Determine semicolon usage
    if (m_semicolonCount + m_noSemicolonCount > 0)
    {
        m_config.semi = (m_semicolonCount > m_noSemicolonCount);
    }

    // Determine quote style
    if (m_singleQuoteCount > m_doubleQuoteCount)
    {
        m_config.singleQuote = true;
    }

    // Determine brace style
    if (m_sameLineBraceCount > m_nextLineBraceCount)
    {
        m_config.braceStyle = "same-line";
    }
    else if (m_nextLineBraceCount > 0)
    {
        m_config.braceStyle = "next-line";
    }

    // Determine indentation
    if (m_tabCount > m_spaceIndentCount)
    {
        m_config.useTabs = true;
    }

    // Determine print width
    if (m_maxLineWidth > 80)
    {
        m_config.printWidth = m_maxLineWidth;
    }

    // Easter egg: the detective has cracked the case - your style secrets revealed
    return m_config;
}

bool StyleDetector::writeConfig(const std::string& filepath) const
{
    toml::table tbl;
    tbl.insert_or_assign("printWidth", m_config.printWidth);
    tbl.insert_or_assign("useTabs", m_config.useTabs);
    tbl.insert_or_assign("tabWidth", m_config.tabWidth);
    tbl.insert_or_assign("semi", m_config.semi);
    tbl.insert_or_assign("singleQuote", m_config.singleQuote);
    tbl.insert_or_assign("trailingComma", m_config.trailingComma);
    tbl.insert_or_assign("bracketSpacing", m_config.bracketSpacing);
    tbl.insert_or_assign("arrowParens", m_config.arrowParens);
    tbl.insert_or_assign("endOfLine", m_config.endOfLine);
    tbl.insert_or_assign("braceStyle", m_config.braceStyle);
    tbl.insert_or_assign("spaceBeforeBrace", m_config.spaceBeforeBrace);
    tbl.insert_or_assign("spaceAfterKeyword", m_config.spaceAfterKeyword);
    tbl.insert_or_assign("spaceAroundOperators", m_config.spaceAroundOperators);
    tbl.insert_or_assign("proseWrap", m_config.proseWrap);
    tbl.insert_or_assign("htmlWhitespaceSensitivity", m_config.htmlWhitespaceSensitivity);
    tbl.insert_or_assign("vueIndentScriptAndStyle", m_config.vueIndentScriptAndStyle);
    tbl.insert_or_assign("embeddedLanguageFormatting", m_config.embeddedLanguageFormatting);
    tbl.insert_or_assign("singleAttributePerLine", m_config.singleAttributePerLine);
    tbl.insert_or_assign("preserveBlankLines", m_config.preserveBlankLines);
    tbl.insert_or_assign("maxBlankLines", m_config.maxBlankLines);
    tbl.insert_or_assign("blankLineAtFileStart", m_config.blankLineAtFileStart);
    tbl.insert_or_assign("blankLineAfterImports", m_config.blankLineAfterImports);
    tbl.insert_or_assign("blankLineBeforeReturn", m_config.blankLineBeforeReturn);
    tbl.insert_or_assign("blankLinesAroundTopLevelDefs", m_config.blankLinesAroundTopLevelDefs);
    tbl.insert_or_assign("finalNewline", m_config.finalNewline);
    tbl.insert_or_assign("indentEmptyLines", m_config.indentEmptyLines);
    tbl.insert_or_assign("emitBom", m_config.emitBom);
    tbl.insert_or_assign("braceStyleFunctions", m_config.braceStyleFunctions);
    tbl.insert_or_assign("braceStyleTypes", m_config.braceStyleTypes);
    tbl.insert_or_assign("spaceAfterComma", m_config.spaceAfterComma);
    tbl.insert_or_assign("spaceAfterColon", m_config.spaceAfterColon);
    tbl.insert_or_assign("spaceBeforeFunctionParen", m_config.spaceBeforeFunctionParen);
    tbl.insert_or_assign("spaceInParens", m_config.spaceInParens);
    tbl.insert_or_assign("spaceInCondParens", m_config.spaceInCondParens);
    tbl.insert_or_assign("spaceInBrackets", m_config.spaceInBrackets);
    tbl.insert_or_assign("arrowSpacing", m_config.arrowSpacing);
    tbl.insert_or_assign("templateExprSpacing", m_config.templateExprSpacing);
    tbl.insert_or_assign("spaceAroundAssignment", m_config.spaceAroundAssignment);
    tbl.insert_or_assign("quoteAvoidEscape", m_config.quoteAvoidEscape);
    tbl.insert_or_assign("emptyBlockSameLine", m_config.emptyBlockSameLine);
    tbl.insert_or_assign("indentSwitchCase", m_config.indentSwitchCase);
    tbl.insert_or_assign("elseOnNewLine", m_config.elseOnNewLine);
    tbl.insert_or_assign("catchOnNewLine", m_config.catchOnNewLine);
    tbl.insert_or_assign("finallyOnNewLine", m_config.finallyOnNewLine);
    tbl.insert_or_assign("joinElseIf", m_config.joinElseIf);
    tbl.insert_or_assign("detectShebangLanguage", m_config.detectShebangLanguage);
    tbl.insert_or_assign("defaultLanguage", m_config.defaultLanguage);

    std::ofstream file(filepath.c_str());
    if (!file.is_open())
    {
        return false;
    }

    file << "# fmtpp configuration - generated by 'fmtpp learn'\n";
    file << "# auto-detected by the style detective\n\n";
    file << tbl << "\n";
    return file.good();
}

Config StyleDetector::detectFromSource(const char* data, size_t len)
{
    // ponytail: plain byte scan; no parser needed, works on any language
    m_tabCount = 0;
    m_spaceIndentCount = 0;
    m_detectedTabWidth = 0;
    m_maxLineWidth = 0;
    m_semicolonCount = 0;
    m_noSemicolonCount = 0;
    m_singleQuoteCount = 0;
    m_doubleQuoteCount = 0;
    m_sameLineBraceCount = 0;
    m_nextLineBraceCount = 0;
    m_curlyTightCount = 0;
    m_curlySpacedCount = 0;
    m_squareTightCount = 0;
    m_squareSpacedCount = 0;
    m_commaTightCount = 0;
    m_commaSpacedCount = 0;
    m_parenTightCount = 0;
    m_parenSpacedCount = 0;
    m_condParenTightCount = 0;
    m_condParenSpacedCount = 0;
    m_arrowParenCount = 0;
    m_bareArrowParamCount = 0;
    m_arrowTightCount = 0;
    m_arrowSpacedCount = 0;
    m_templateTightCount = 0;
    m_templateSpacedCount = 0;
    m_trailingCommaLineCount = 0;
    m_noTrailingCommaLineCount = 0;
    m_multilineTagCount = 0;
    m_singleLineTagCount = 0;
    m_maxBlankRun = 0;
    m_blankAfterImportCount = 0;
    m_noBlankAfterImportCount = 0;
    m_assignSpacedCount = 0;
    m_assignTightCount = 0;
    m_colonSpacedCount = 0;
    m_colonTightCount = 0;
    m_funcSpaceCount = 0;
    m_funcTightCount = 0;
    m_braceTightCount = 0;
    m_braceSpacedCount = 0;
    m_keywordTightCount = 0;
    m_keywordSpacedCount = 0;
    m_elseJoinedCount = 0;
    m_elseSplitCount = 0;
    m_catchJoinedCount = 0;
    m_catchSplitCount = 0;
    m_finallyJoinedCount = 0;
    m_finallySplitCount = 0;
    m_elseifCount = 0;
    m_elseIfCount = 0;
    m_blankBeforeReturnCount = 0;
    m_noBlankBeforeReturnCount = 0;
    m_leadingBlankCount = 0;
    m_finalNewlineCount = 0;
    m_noFinalNewlineCount = 0;
    m_bomCount = 0;
    m_inlineEmptyCount = 0;
    m_splitEmptyCount = 0;

    if (data == NULL || len == 0)
    {
        return m_config;
    }

    if (len >= 3 && (unsigned char)data[0] == 0xEF
        && (unsigned char)data[1] == 0xBB && (unsigned char)data[2] == 0xBF)
    {
        m_bomCount = 1;
    }
    if (data[len - 1] == '\n') { m_finalNewlineCount = 1; }
    else { m_noFinalNewlineCount = 1; }

    std::vector<int> widths;
    std::vector<int> indents;
    bool sawCRLF = false;
    int lineIndex = 0;
    int blankRun = 0;
    bool prevWasEmpty = false;
    bool prevNonEmptyWasImport = false;
    bool importGapCounted = false;
    std::string prevTrimmed;

    size_t lineStart = 0;
    for (size_t i = 0; i <= len; i++)
    {
        bool eol = (i == len);
        char c = eol ? '\0' : data[i];
        if (!eol && c != '\n' && c != '\r')
        {
            continue;
        }

        size_t lineEnd = i;
        if (lineEnd > lineStart && data[lineEnd - 1] == '\r')
        {
            lineEnd--;
            sawCRLF = true;
        }

        // Skip the '\n' of a CRLF pair on the next pass via lineStart below.
        // A lone '\r' also ends the line.
        if (!eol && c == '\r' && i + 1 < len && data[i + 1] == '\n')
        {
            continue;
        }

        if (lineEnd == lineStart)
        {
            if (lineIndex == 0) { m_leadingBlankCount = 1; }
            blankRun++;
            if (blankRun > m_maxBlankRun) { m_maxBlankRun = blankRun; }
            if (prevNonEmptyWasImport && !importGapCounted)
            {
                m_blankAfterImportCount++;
                importGapCounted = true;
            }
            prevWasEmpty = true;
            lineIndex++;
            lineStart = i + 1;
            continue;
        }
        blankRun = 0;
        importGapCounted = false;

        if (lineEnd > lineStart)
        {
            widths.push_back(static_cast<int>(lineEnd - lineStart));

            size_t p = lineStart;
            int tabs = 0;
            int spaces = 0;
            while (p < lineEnd && (data[p] == ' ' || data[p] == '\t'))
            {
                if (data[p] == '\t') { tabs++; }
                else { spaces++; }
                p++;
            }

            if (p < lineEnd)
            {
                if (tabs > 0 && spaces == 0) { m_tabCount++; }
                else if (spaces > 0 && tabs == 0)
                {
                    m_spaceIndentCount++;
                    indents.push_back(spaces);
                }

                bool isComment = (p + 1 < lineEnd && data[p] == '/' && data[p + 1] == '/')
                    || data[p] == '#' || data[p] == '*'
                    || (p + 1 < lineEnd && data[p] == '/' && data[p + 1] == '*');
                if (!isComment)
                {
                    size_t q = lineEnd;
                    while (q > p && (data[q - 1] == ' ' || data[q - 1] == '\t')) { q--; }
                    char last = (q > p) ? data[q - 1] : '\0';

                    if (last == ';') { m_semicolonCount++; }
                    else if (last != '{' && last != '}' && last != ',' && last != ':'
                        && last != '\0' && data[p] != '#')
                    {
                        m_noSemicolonCount++;
                    }

                    if (last == '{') { m_sameLineBraceCount++; }
                    if (q - p == 1 && data[p] == '{') { m_nextLineBraceCount++; }

                    for (size_t k = p; k < lineEnd; k++)
                    {
                        if (data[k] == '\'') { m_singleQuoteCount++; }
                        else if (data[k] == '"') { m_doubleQuoteCount++; }
                    }

                    if (last == ',') { m_trailingCommaLineCount++; }
                    else { m_noTrailingCommaLineCount++; }
                    scanCodeLine(data, p, q);
                }

                size_t qt = lineEnd;
                while (qt > p && (data[qt - 1] == ' ' || data[qt - 1] == '\t')) { qt--; }
                // ponytail: keyword shapes ignore comment lines (`// else`
                // is prose, not structure); tags and brace pairs stay global
                bool codeLine = !isComment;
                if (codeLine && containsWord(data, p, qt, "elseif")) { m_elseifCount++; }
                if (codeLine && containsElseIf(data, p, qt)) { m_elseIfCount++; }
                if (codeLine)
                {
                    // ponytail: a collated `} else` anywhere on the line is
                    // joined evidence; a leading `else` is split evidence
                    bool hasJoinedElse = false;
                    for (size_t k = p; k + 1 < qt; k++)
                    {
                        if (data[k] == '}')
                        {
                            size_t r = k + 1;
                            while (r < qt && (data[r] == ' ' || data[r] == '\t')) { r++; }
                            if (startsWithWord(data, r, qt, "else")
                                && !startsWithWord(data, r, qt, "elseif"))
                            {
                                hasJoinedElse = true;
                            }
                            break;
                        }
                    }
                    bool hasLeadingElse = startsWithWord(data, p, qt, "else")
                        && !startsWithWord(data, p, qt, "elseif");
                    if (hasJoinedElse) { m_elseJoinedCount++; }
                    else if (hasLeadingElse) { m_elseSplitCount++; }
                    bool hasJoinedCatch = false;
                    for (size_t k = p; k + 1 < qt; k++)
                    {
                        if (data[k] == '}')
                        {
                            size_t r = k + 1;
                            while (r < qt && (data[r] == ' ' || data[r] == '\t')) { r++; }
                            if (startsWithWord(data, r, qt, "catch")) { hasJoinedCatch = true; }
                            break;
                        }
                    }
                    bool hasLeadingCatch = startsWithWord(data, p, qt, "catch");
                    if (hasJoinedCatch) { m_catchJoinedCount++; }
                    else if (hasLeadingCatch) { m_catchSplitCount++; }
                    bool hasJoinedFinally = false;
                    for (size_t k = p; k + 1 < qt; k++)
                    {
                        if (data[k] == '}')
                        {
                            size_t r = k + 1;
                            while (r < qt && (data[r] == ' ' || data[r] == '\t')) { r++; }
                            if (startsWithWord(data, r, qt, "finally")) { hasJoinedFinally = true; }
                            break;
                        }
                    }
                    bool hasLeadingFinally = startsWithWord(data, p, qt, "finally");
                    if (hasJoinedFinally) { m_finallyJoinedCount++; }
                    else if (hasLeadingFinally) { m_finallySplitCount++; }
                }
                if (codeLine && startsWithWord(data, p, qt, "return"))
                {
                    if (prevWasEmpty) { m_blankBeforeReturnCount++; }
                    else { m_noBlankBeforeReturnCount++; }
                }
                bool isImportLine = false;
                if (codeLine)
                {
                    isImportLine = startsWithWord(data, p, qt, "import")
                        || startsWithWord(data, p, qt, "from")
                        || startsWithWord(data, p, qt, "using")
                        || startsWithWord(data, p, qt, "require")
                        || startsWithWord(data, p, qt, "include")
                        || ((p < qt && data[p] == '#')
                            && (startsWithWord(data, p + 1, qt, "include")
                                || startsWithWord(data, p + 1, qt, "import")));
                }
                if (prevNonEmptyWasImport && !prevWasEmpty) { m_noBlankAfterImportCount++; }
                {
                    bool inlineEmpty = false;
                    for (size_t k = p; k + 1 < qt; k++)
                    {
                        if (data[k] == '{' && data[k + 1] == '}')
                        {
                            inlineEmpty = true;
                            break;
                        }
                    }
                    if (inlineEmpty) { m_inlineEmptyCount++; }
                }
                if (qt - p == 1 && data[p] == '}' && prevTrimmed == "{")
                {
                    m_splitEmptyCount++;
                }
                if (p < qt && data[p] == '<' && qt - p > 1 && detIsIdent(data[p + 1]))
                {
                    bool closed = false;
                    for (size_t k = p; k < qt; k++)
                    {
                        if (data[k] == '>') { closed = true; break; }
                    }
                    if (closed) { m_singleLineTagCount++; }
                    else { m_multilineTagCount++; }
                }
                prevNonEmptyWasImport = isImportLine;
                prevWasEmpty = false;
                prevTrimmed.assign(data + p, qt - p);
            }
            lineIndex++;
        }

        lineStart = i + 1;
    }

    if (m_tabCount > m_spaceIndentCount)
    {
        m_config.useTabs = true;
    }

    if (!indents.empty())
    {
        std::sort(indents.begin(), indents.end());
        int mode = indents[0];
        int modeCount = 1;
        int best = mode;
        int bestCount = 1;
        for (size_t i = 1; i < indents.size(); i++)
        {
            if (indents[i] == mode) { modeCount++; }
            else { mode = indents[i]; modeCount = 1; }
            if (modeCount > bestCount) { bestCount = modeCount; best = mode; }
        }
        m_detectedTabWidth = best;
        m_config.tabWidth = best;
    }

    if (!widths.empty())
    {
        std::sort(widths.begin(), widths.end());
        int p90 = widths[(widths.size() * 9) / 10];
        m_maxLineWidth = widths.back();
        if (p90 > 80)
        {
            m_config.printWidth = p90;
        }
    }

    if (m_semicolonCount + m_noSemicolonCount > 0)
    {
        m_config.semi = (m_semicolonCount > m_noSemicolonCount);
    }

    if (m_singleQuoteCount + m_doubleQuoteCount > 0)
    {
        m_config.singleQuote = (m_singleQuoteCount > m_doubleQuoteCount);
    }

    if (m_sameLineBraceCount > m_nextLineBraceCount)
    {
        m_config.braceStyle = "same-line";
    }
    else if (m_nextLineBraceCount > 0)
    {
        m_config.braceStyle = "next-line";
    }

    if (sawCRLF)
    {
        m_config.endOfLine = "crlf";
    }

    if (m_curlySpacedCount + m_curlyTightCount > 0)
    {
        m_config.bracketSpacing = (m_curlySpacedCount >= m_curlyTightCount);
    }
    if (m_commaSpacedCount + m_commaTightCount > 0)
    {
        m_config.spaceAfterComma = (m_commaSpacedCount >= m_commaTightCount);
    }
    if (m_parenSpacedCount + m_parenTightCount > 0)
    {
        m_config.spaceInParens = (m_parenSpacedCount > m_parenTightCount);
    }
    if (m_condParenSpacedCount + m_condParenTightCount > 0)
    {
        m_config.spaceInCondParens = (m_condParenSpacedCount > m_condParenTightCount);
    }
    if (m_squareSpacedCount + m_squareTightCount > 0)
    {
        m_config.spaceInBrackets = (m_squareSpacedCount > m_squareTightCount);
    }
    if (m_arrowParenCount > 0 && m_arrowParenCount >= m_bareArrowParamCount)
    {
        m_config.arrowParens = "always";
    }
    else if (m_bareArrowParamCount > m_arrowParenCount)
    {
        m_config.arrowParens = "as-needed";
    }
    if (m_arrowSpacedCount + m_arrowTightCount > 0)
    {
        m_config.arrowSpacing = (m_arrowSpacedCount >= m_arrowTightCount);
    }
    if (m_templateSpacedCount + m_templateTightCount > 0)
    {
        m_config.templateExprSpacing = (m_templateSpacedCount > m_templateTightCount);
    }
    if (m_trailingCommaLineCount > 0)
    {
        m_config.trailingComma = "es5";
    }
    else if (m_noTrailingCommaLineCount > 0)
    {
        m_config.trailingComma = "none";
    }
    if (m_multilineTagCount > 0 && m_multilineTagCount >= m_singleLineTagCount)
    {
        m_config.singleAttributePerLine = true;
    }
    if (m_maxBlankRun > 0)
    {
        m_config.maxBlankLines = (m_maxBlankRun > 9) ? 9 : m_maxBlankRun;
    }
    if (m_blankAfterImportCount + m_noBlankAfterImportCount > 0)
    {
        m_config.blankLineAfterImports =
            (m_blankAfterImportCount > m_noBlankAfterImportCount);
    }
    if (m_assignSpacedCount + m_assignTightCount > 0)
    {
        m_config.spaceAroundAssignment = (m_assignSpacedCount >= m_assignTightCount);
    }
    if (m_colonSpacedCount + m_colonTightCount > 0)
    {
        m_config.spaceAfterColon = (m_colonSpacedCount >= m_colonTightCount);
    }
    if (m_funcSpaceCount + m_funcTightCount > 0)
    {
        m_config.spaceBeforeFunctionParen = (m_funcSpaceCount > m_funcTightCount);
    }
    if (m_braceSpacedCount + m_braceTightCount > 0)
    {
        m_config.spaceBeforeBrace = (m_braceSpacedCount >= m_braceTightCount);
    }
    if (m_keywordSpacedCount + m_keywordTightCount > 0)
    {
        m_config.spaceAfterKeyword = (m_keywordSpacedCount >= m_keywordTightCount);
    }
    if (m_elseJoinedCount + m_elseSplitCount > 0)
    {
        if (m_elseJoinedCount > m_elseSplitCount) { m_config.elseOnNewLine = "never"; }
        else if (m_elseSplitCount > m_elseJoinedCount) { m_config.elseOnNewLine = "always"; }
    }
    if (m_catchJoinedCount + m_catchSplitCount > 0)
    {
        if (m_catchJoinedCount > m_catchSplitCount) { m_config.catchOnNewLine = "never"; }
        else if (m_catchSplitCount > m_catchJoinedCount) { m_config.catchOnNewLine = "always"; }
    }
    if (m_finallyJoinedCount + m_finallySplitCount > 0)
    {
        if (m_finallyJoinedCount > m_finallySplitCount) { m_config.finallyOnNewLine = "never"; }
        else if (m_finallySplitCount > m_finallyJoinedCount) { m_config.finallyOnNewLine = "always"; }
    }
    if (m_elseifCount > 0)
    {
        m_config.joinElseIf = false;
    }
    if (m_leadingBlankCount > 0)
    {
        m_config.blankLineAtFileStart = true;
    }
    if (m_blankBeforeReturnCount + m_noBlankBeforeReturnCount > 0)
    {
        m_config.blankLineBeforeReturn =
            (m_blankBeforeReturnCount > m_noBlankBeforeReturnCount);
    }
    if (m_noFinalNewlineCount > 0)
    {
        m_config.finalNewline = false;
    }
    if (m_bomCount > 0)
    {
        m_config.emitBom = true;
    }
    if (m_inlineEmptyCount + m_splitEmptyCount > 0)
    {
        m_config.emptyBlockSameLine = (m_inlineEmptyCount > m_splitEmptyCount);
    }
    m_config.braceStyleFunctions = m_config.braceStyle;
    m_config.braceStyleTypes = m_config.braceStyle;

    return m_config;
}

// --- Visitor implementations ---

void StyleDetector::visitProgram(ProgramNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_statements.size(); i++)
    {
        if (node.m_statements[i] != NULL)
        {
            node.m_statements[i]->accept(*this);
        }
    }
}

void StyleDetector::visitFunctionDecl(FunctionDeclNode& node)
{
    // ponytail: AST nodes carry no style evidence (no positions, no quote
    // chars), so AST traversal only takes a node census. Real style comes
    // from detectFromSource(). Nothing is assumed here.
    m_nodeCount++;
    if (node.m_body != NULL)
    {
        node.m_body->accept(*this);
    }
}

void StyleDetector::visitVariableDecl(VariableDeclNode& node)
{
    m_nodeCount++;
    if (node.m_initializer != NULL)
    {
        node.m_initializer->accept(*this);
    }
}

void StyleDetector::visitIfStmt(IfStmtNode& node)
{
    m_nodeCount++;
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_thenBranch != NULL) { node.m_thenBranch->accept(*this); }
    if (node.m_elseBranch != NULL) { node.m_elseBranch->accept(*this); }
}

void StyleDetector::visitForStmt(ForStmtNode& node)
{
    m_nodeCount++;
    if (node.m_init != NULL) { node.m_init->accept(*this); }
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_update != NULL) { node.m_update->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void StyleDetector::visitWhileStmt(WhileStmtNode& node)
{
    m_nodeCount++;
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void StyleDetector::visitSwitchStmt(SwitchStmtNode& node)
{
    m_nodeCount++;
    if (node.m_discriminant != NULL) { node.m_discriminant->accept(*this); }
    size_t i;
    for (i = 0; i < node.m_cases.size(); i++)
    {
        if (node.m_cases[i] != NULL) { node.m_cases[i]->accept(*this); }
    }
}

void StyleDetector::visitBlock(BlockNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_statements.size(); i++)
    {
        if (node.m_statements[i] != NULL) { node.m_statements[i]->accept(*this); }
    }
}

void StyleDetector::visitReturnStmt(ReturnStmtNode& node)
{
    m_nodeCount++;
    if (node.m_argument != NULL) { node.m_argument->accept(*this); }
}

void StyleDetector::visitThrowStmt(ThrowStmtNode& node)
{
    m_nodeCount++;
    if (node.m_expression != NULL) { node.m_expression->accept(*this); }
}

void StyleDetector::visitTryCatchStmt(TryCatchStmtNode& node)
{
    m_nodeCount++;
    if (node.m_body != NULL) { node.m_body->accept(*this); }
    if (node.m_catchClause != NULL) { node.m_catchClause->accept(*this); }
    if (node.m_catchBody != NULL) { node.m_catchBody->accept(*this); }
    if (node.m_finallyBlock != NULL) { node.m_finallyBlock->accept(*this); }
}

void StyleDetector::visitComment(CommentNode&)
{
    m_nodeCount++;
}

void StyleDetector::visitLiteral(LiteralNode& node)
{
    // ponytail: literal values carry no quote characters, so quote style
    // is unobservable here. See detectFromSource().
    m_nodeCount++;
    (void)node;
}

void StyleDetector::visitIdentifier(IdentifierNode&)
{
    m_nodeCount++;
}

void StyleDetector::visitArrayLiteral(ArrayLiteralNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_elements.size(); i++)
    {
        if (node.m_elements[i] != NULL) { node.m_elements[i]->accept(*this); }
    }
}

void StyleDetector::visitObjectLiteral(ObjectLiteralNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_properties.size(); i++)
    {
        if (node.m_properties[i] != NULL) { node.m_properties[i]->accept(*this); }
    }
}

void StyleDetector::visitTemplateLiteral(TemplateLiteralNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_quasis.size(); i++)
    {
        if (node.m_quasis[i] != NULL) { node.m_quasis[i]->accept(*this); }
    }
    for (i = 0; i < node.m_expressions.size(); i++)
    {
        if (node.m_expressions[i] != NULL) { node.m_expressions[i]->accept(*this); }
    }
}

void StyleDetector::visitArrowFunction(ArrowFunctionNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_params.size(); i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
    }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void StyleDetector::visitClassDecl(ClassDeclNode& node)
{
    m_nodeCount++;
    if (node.m_superClass != NULL) { node.m_superClass->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void StyleDetector::visitInterfaceDecl(InterfaceDeclNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_extends.size(); i++)
    {
        if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
    }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void StyleDetector::visitEnumDecl(EnumDeclNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_members.size(); i++)
    {
        if (node.m_members[i] != NULL) { node.m_members[i]->accept(*this); }
    }
}

void StyleDetector::visitTypeAlias(TypeAliasNode& node)
{
    m_nodeCount++;
    if (node.m_typeAnnotation != NULL) { node.m_typeAnnotation->accept(*this); }
}

void StyleDetector::visitImportDecl(ImportDeclNode& node)
{
    m_nodeCount++;
    if (node.m_defaultImport != NULL) { node.m_defaultImport->accept(*this); }
    size_t i;
    for (i = 0; i < node.m_specifiers.size(); i++)
    {
        if (node.m_specifiers[i] != NULL) { node.m_specifiers[i]->accept(*this); }
    }
}

void StyleDetector::visitExportDecl(ExportDeclNode& node)
{
    m_nodeCount++;
    if (node.m_declaration != NULL) { node.m_declaration->accept(*this); }
}

void StyleDetector::visitPreprocessorDirective(PreprocessorDirectiveNode&)
{
    m_nodeCount++;
}

void StyleDetector::visitNamespace(NamespaceNode& node)
{
    m_nodeCount++;
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void StyleDetector::visitProperty(PropertyNode& node)
{
    m_nodeCount++;
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    if (node.m_value != NULL) { node.m_value->accept(*this); }
}

void StyleDetector::visitRawText(RawTextNode&)
{
    m_nodeCount++;
}

void StyleDetector::visitHtmlElement(HtmlElementNode& node)
{
    m_nodeCount++;
    size_t i;
    for (i = 0; i < node.m_children.size(); i++)
    {
        if (node.m_children[i] != NULL) { node.m_children[i]->accept(*this); }
    }
}

void StyleDetector::visitWith(WithNode& node)
{
    m_nodeCount++;
    if (node.m_context != NULL) { node.m_context->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void StyleDetector::visitSwitchCase(SwitchCaseNode& node)
{
    m_nodeCount++;
    if (node.m_pattern != NULL) { node.m_pattern->accept(*this); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

// Easter egg: the style detective has solved the case - your formatting preferences are exposed