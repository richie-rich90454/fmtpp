#include "fmtpp/VueFormatter.hpp"
#include "fmtpp/Parser.hpp"
#include "fmtpp/FormatterFactory.hpp"
#include "fmtpp/ast/HtmlElementNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include <cstdio>

// Easter egg: Vue formatter - v-for every tasteful indentation

VueFormatter::VueFormatter(OutputBuffer& out, const Config& config)
    : HtmlFormatter(out, config)
{
}

VueFormatter::~VueFormatter()
{
}

std::string VueFormatter::langAttr(const HtmlElementNode& node) const
{
    for (size_t i = 0; i < node.m_attrs.size(); i++)
    {
        const std::string& attr = node.m_attrs[i];
        if (attr.size() > 5 && attr.compare(0, 5, "lang=") == 0)
        {
            std::string value = attr.substr(5);
            if (value.size() >= 2 && (value[0] == '"' || value[0] == '\''))
            {
                return value.substr(1, value.size() - 2);
            }
            return value;
        }
    }
    return std::string();
}

void VueFormatter::formatInner(const std::string& code, const std::string& language)
{
    OutputBuffer inner(code.size() * 2 + 64);
    Parser parser;
    // ponytail: nested parse shares the pool; a reset would kill the outer AST
    ProgramNode* ast = parser.parseNested(code.c_str(), code.size(), language);
    if (ast == NULL)
    {
        printIndent();
        printToken(code.c_str());
        printNewline();
        return;
    }
    FormatterVisitor* formatter = FormatterFactory::create(language, inner, m_config);
    if (formatter == NULL)
    {
        printIndent();
        printToken(code.c_str());
        printNewline();
        return;
    }
    formatter->format(ast);
    delete formatter;

    // ponytail: splice the sub-output one indent deeper, line by line
    std::string text = inner.toString();
    bool atLineStart = true;
    for (size_t i = 0; i < text.size(); i++)
    {
        if (atLineStart && text[i] != '\n')
        {
            printIndent();
            atLineStart = false;
        }
        char ch[2];
        ch[0] = text[i];
        ch[1] = '\0';
        printToken(ch);
        if (text[i] == '\n')
        {
            atLineStart = true;
        }
    }
}

void VueFormatter::visitHtmlElement(HtmlElementNode& node)
{
    bool isScript = (node.m_tag == "script");
    bool isStyle = (node.m_tag == "style");
    if (!isScript && !isStyle)
    {
        CstyleFormatterBase::visitHtmlElement(node);
        return;
    }

    printToken("<");
    printToken(node.m_tag.c_str());
    if (m_config.singleAttributePerLine && node.m_attrs.size() > 1)
    {
        indent();
        for (size_t i = 0; i < node.m_attrs.size(); i++)
        {
            printNewline();
            printIndent();
            printToken(node.m_attrs[i].c_str());
        }
        dedent();
        printNewline();
        printIndent();
    }
    else
    {
        for (size_t i = 0; i < node.m_attrs.size(); i++)
        {
            printSpace();
            printToken(node.m_attrs[i].c_str());
        }
    }
    printToken(">");
    printNewline();
    if (m_config.vueIndentScriptAndStyle) { indent(); }
    indent();
    std::string language = "js";
    if (isScript)
    {
        std::string lang = langAttr(node);
        if (lang == "ts" || lang == "tsx") { language = "ts"; }
    }
    else
    {
        language = "css";
    }
    for (size_t i = 0; i < node.m_children.size(); i++)
    {
        if (node.m_children[i] != NULL && node.m_children[i]->isRawText())
        {
            RawTextNode* text = static_cast<RawTextNode*>(node.m_children[i]);
            if (m_config.embeddedLanguageFormatting == "off")
            {
                printIndent();
                printToken(text->m_text.c_str());
                printNewline();
            }
            else
            {
                formatInner(text->m_text, language);
            }
        }
        else if (node.m_children[i] != NULL)
        {
            printIndent();
            node.m_children[i]->accept(*this);
            printNewline();
        }
    }
    if (m_config.vueIndentScriptAndStyle) { dedent(); }
    dedent();
    printIndent();
    printToken("</");
    printToken(node.m_tag.c_str());
    printToken(">");
    printNewline();
}

// Easter egg: VueFormatter complete - mounted with perfect formatting
