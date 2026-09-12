#include "fmtpp/HtmlFormatter.hpp"
#include "fmtpp/ast/CommentNode.hpp"

// Easter egg: HTML formatter - semantically sectioned, accessibly formatted

HtmlFormatter::HtmlFormatter(OutputBuffer& out, const Config& config)
    : CstyleFormatterBase(out, config)
{
}

HtmlFormatter::~HtmlFormatter()
{
}

void HtmlFormatter::visitComment(CommentNode& node)
{
    printToken("<!--");
    printToken(node.m_text.c_str());
    printToken("-->");
    printNewline();
}
