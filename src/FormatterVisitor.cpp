#include "fmtpp/FormatterVisitor.hpp"

FormatterVisitor::FormatterVisitor(OutputBuffer& out, const Config& config)
    : m_out(out)
    , m_config(config)
    , m_indentLevel(0)
    , m_currentColumn(0)
{
    // Easter egg: formatter visitor born, ready to make code beautiful
}

FormatterVisitor::~FormatterVisitor()
{
    // Easter egg: formatter visitor signing off - mission accomplished
}

void FormatterVisitor::printIndent()
{
    int indentSize = m_config.tabWidth;
    int totalSpaces = m_indentLevel * indentSize;
    
    if (m_config.useTabs)
    {
        for (int i = 0; i < m_indentLevel; i++)
        {
            m_out.appendChar('\t');
            m_currentColumn += m_config.tabWidth;
        }
    }
    else
    {
        for (int i = 0; i < totalSpaces; i++)
        {
            m_out.appendChar(' ');
        }
        m_currentColumn += totalSpaces;
    }
}

void FormatterVisitor::printNewline()
{
    // ponytail: never stack newlines (statement visitors end with one AND
    // containers add separators); a newline right after a newline is always a bug
    if (m_out.getSize() > 0)
    {
        const char* data = m_out.getData();
        if (data[m_out.getSize() - 1] == '\n')
        {
            return;
        }
    }
    m_out.appendChar('\n');
    m_currentColumn = 0;
}

void FormatterVisitor::printBlankLine(int blanks)
{
    // ponytail: blank separators for preserved input gaps and blank
    // options; never more than maxBlankLines total. Leading blanks only
    // happen when a caller explicitly allows them (blankLineAtFileStart).
    if (blanks <= 0) { return; }
    if (blanks > m_config.maxBlankLines) { blanks = m_config.maxBlankLines; }
    if (blanks <= 0) { return; }
    const char* data = m_out.getData();
    size_t size = m_out.getSize();
    size_t e = 0;
    while (e < size && data[size - 1 - e] == '\n') { e++; }
    int haveBlanks = (e > 0) ? static_cast<int>(e) - 1 : 0;
    if (e == 0) { m_out.appendChar('\n'); }
    for (int b = haveBlanks; b < blanks; b++)
    {
        if (m_config.indentEmptyLines) { printIndent(); }
        m_out.appendChar('\n');
    }
    m_currentColumn = 0;
}

std::string FormatterVisitor::applyEndOfLine(const std::string& text) const
{
    std::string out = text;
    if (m_config.finalNewline)
    {
        if (!out.empty() && out[out.size() - 1] != '\n') { out.push_back('\n'); }
    }
    else
    {
        while (!out.empty() && out[out.size() - 1] == '\n') { out.resize(out.size() - 1); }
    }
    if (m_config.emitBom)
    {
        if (out.size() < 3 || out[0] != '\xEF' || out[1] != '\xBB' || out[2] != '\xBF')
        {
            out.insert(0, "\xEF\xBB\xBF");
        }
    }
    if (m_config.endOfLine == "crlf")
    {
        std::string converted;
        converted.reserve(out.size() + out.size() / 32);
        for (size_t i = 0; i < out.size(); i++)
        {
            if (out[i] == '\n') { converted.push_back('\r'); }
            converted.push_back(out[i]);
        }
        return converted;
    }
    if (m_config.endOfLine == "cr")
    {
        for (size_t i = 0; i < out.size(); i++)
        {
            if (out[i] == '\n') { out[i] = '\r'; }
        }
        return out;
    }
    return out;
}

void FormatterVisitor::printSpace()
{
    // ponytail: never stack spaces (callers print one before bodies AND
    // before braces); a space after space/newline/indent is always a bug
    if (m_out.getSize() > 0)
    {
        const char* data = m_out.getData();
        char last = data[m_out.getSize() - 1];
        if (last == ' ' || last == '\n' || last == '\t')
        {
            return;
        }
    }
    m_out.appendChar(' ');
    m_currentColumn++;
}

void FormatterVisitor::printToken(const char* token)
{
    if (token != NULL)
    {
        m_out.append(token);
        // Count the token length
        const char* p = token;
        while (*p != '\0')
        {
            m_currentColumn++;
            p++;
        }
    }
}

void FormatterVisitor::printStatementBlanks(const std::vector<ASTNode*>& stmts, size_t i)
{
    ASTNode* stmt = (i < stmts.size()) ? stmts[i] : NULL;
    ASTNode* prev = (i > 0 && i - 1 < stmts.size()) ? stmts[i - 1] : NULL;
    if (stmt != NULL && stmt->m_blankLinesBefore > 0 && m_config.preserveBlankLines
        && (i > 0 || m_config.blankLineAtFileStart))
    {
        printBlankLine(stmt->m_blankLinesBefore);
    }
    if (m_config.blankLineAfterImports && prev != NULL && prev->isImportDirective()
        && stmt != NULL && !stmt->isImportDirective())
    {
        printBlankLine(1);
    }
    if (m_config.blankLineBeforeReturn && i > 0 && stmt != NULL && stmt->isReturnStmt())
    {
        printBlankLine(1);
    }
}

void FormatterVisitor::printAssign()
{
    if (m_config.spaceAroundAssignment) { printToken(" = "); }
    else { printToken("="); }
}

void FormatterVisitor::printShortAssign()
{
    if (m_config.spaceAroundAssignment) { printToken(" := "); }
    else { printToken(":="); }
}

void FormatterVisitor::printComma()
{
    m_out.appendChar(',');
    m_currentColumn++;
    if (m_config.spaceAfterComma) { printSpace(); }
}

void FormatterVisitor::printSemicolon()
{
    if (m_config.semi)
    {
        m_out.appendChar(';');
        m_currentColumn++;
    }
}

void FormatterVisitor::indent()
{
    m_indentLevel++;
}

void FormatterVisitor::dedent()
{
    if (m_indentLevel > 0)
    {
        m_indentLevel--;
    }
}