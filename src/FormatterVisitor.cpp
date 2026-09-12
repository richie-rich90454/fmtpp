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
    m_out.appendChar('\n');
    m_currentColumn = 0;
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

void FormatterVisitor::printComma()
{
    m_out.appendChar(',');
    m_currentColumn++;
    printSpace();
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