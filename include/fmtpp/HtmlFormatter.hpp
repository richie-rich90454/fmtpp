#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * HTML (and XML/XHTML/SVG) formatter.
 * Elements, text, comments and doctypes are all handled by
 * CstyleFormatterBase; this subclass only names the language.
 *
 * Easter egg: HTML - the only language where divs nest deeper than callbacks
 */
class HtmlFormatter : public CstyleFormatterBase
{
public:
    HtmlFormatter(OutputBuffer& out, const Config& config);
    virtual ~HtmlFormatter();

    virtual void visitComment(CommentNode& node);
};
